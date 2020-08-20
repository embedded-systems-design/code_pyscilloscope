/***************************************************************************//**
* \file myUSB_drv.c
* \version 3.20
*
* \brief
*  This file contains the Endpoint 0 Driver for the USBFS Component.  
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myUSB_pvt.h"
#include "cyapicallbacks.h"


/***************************************
* Global data allocation
***************************************/

volatile T_myUSB_EP_CTL_BLOCK myUSB_EP[myUSB_MAX_EP];

/** Contains the current configuration number, which is set by the host using a 
 * SET_CONFIGURATION request. This variable is initialized to zero in 
 * USBFS_InitComponent() API and can be read by the USBFS_GetConfiguration() 
 * API.*/
volatile uint8 myUSB_configuration;

/** Contains the current interface number.*/
volatile uint8 myUSB_interfaceNumber;

/** This variable is set to one after SET_CONFIGURATION and SET_INTERFACE 
 *requests. It can be read by the USBFS_IsConfigurationChanged() API */
volatile uint8 myUSB_configurationChanged;

/** Contains the current device address.*/
volatile uint8 myUSB_deviceAddress;

/** This is a two-bit variable that contains power status in the bit 0 
 * (DEVICE_STATUS_BUS_POWERED or DEVICE_STATUS_SELF_POWERED) and remote wakeup 
 * status (DEVICE_STATUS_REMOTE_WAKEUP) in the bit 1. This variable is 
 * initialized to zero in USBFS_InitComponent() API, configured by the 
 * USBFS_SetPowerStatus() API. The remote wakeup status cannot be set using the 
 * API SetPowerStatus(). */
volatile uint8 myUSB_deviceStatus;

volatile uint8 myUSB_interfaceSetting[myUSB_MAX_INTERFACES_NUMBER];
volatile uint8 myUSB_interfaceSetting_last[myUSB_MAX_INTERFACES_NUMBER];
volatile uint8 myUSB_interfaceStatus[myUSB_MAX_INTERFACES_NUMBER];

/** Contains the started device number. This variable is set by the 
 * USBFS_Start() or USBFS_InitComponent() APIs.*/
volatile uint8 myUSB_device;

/** Initialized class array for each interface. It is used for handling Class 
 * specific requests depend on interface class. Different classes in multiple 
 * alternate settings are not supported.*/
const uint8 CYCODE *myUSB_interfaceClass;


/***************************************
* Local data allocation
***************************************/

volatile uint8  myUSB_ep0Toggle;
volatile uint8  myUSB_lastPacketSize;

/** This variable is used by the communication functions to handle the current 
* transfer state.
* Initialized to TRANS_STATE_IDLE in the USBFS_InitComponent() API and after a 
* complete transfer in the status stage.
* Changed to the TRANS_STATE_CONTROL_READ or TRANS_STATE_CONTROL_WRITE in setup 
* transaction depending on the request type.
*/
volatile uint8  myUSB_transferState;
volatile T_myUSB_TD myUSB_currentTD;
volatile uint8  myUSB_ep0Mode;
volatile uint8  myUSB_ep0Count;
volatile uint16 myUSB_transferByteCount;


/*******************************************************************************
* Function Name: myUSB_ep_0_Interrupt
****************************************************************************//**
*
*  This Interrupt Service Routine handles Endpoint 0 (Control Pipe) traffic.
*  It dispatches setup requests and handles the data and status stages.
*
*
*******************************************************************************/
CY_ISR(myUSB_EP_0_ISR)
{
    uint8 tempReg;
    uint8 modifyReg;

#ifdef myUSB_EP_0_ISR_ENTRY_CALLBACK
    myUSB_EP_0_ISR_EntryCallback();
#endif /* (myUSB_EP_0_ISR_ENTRY_CALLBACK) */
    
    tempReg = myUSB_EP0_CR_REG;
    if ((tempReg & myUSB_MODE_ACKD) != 0u)
    {
        modifyReg = 1u;
        if ((tempReg & myUSB_MODE_SETUP_RCVD) != 0u)
        {
            if ((tempReg & myUSB_MODE_MASK) != myUSB_MODE_NAK_IN_OUT)
            {
                /* Mode not equal to NAK_IN_OUT: invalid setup */
                modifyReg = 0u;
            }
            else
            {
                myUSB_HandleSetup();
                
                if ((myUSB_ep0Mode & myUSB_MODE_SETUP_RCVD) != 0u)
                {
                    /* SETUP bit set: exit without mode modificaiton */
                    modifyReg = 0u;
                }
            }
        }
        else if ((tempReg & myUSB_MODE_IN_RCVD) != 0u)
        {
            myUSB_HandleIN();
        }
        else if ((tempReg & myUSB_MODE_OUT_RCVD) != 0u)
        {
            myUSB_HandleOUT();
        }
        else
        {
            modifyReg = 0u;
        }
        
        /* Modify the EP0_CR register */
        if (modifyReg != 0u)
        {
            
            tempReg = myUSB_EP0_CR_REG;
            
            /* Make sure that SETUP bit is cleared before modification */
            if ((tempReg & myUSB_MODE_SETUP_RCVD) == 0u)
            {
                /* Update count register */
                tempReg = (uint8) myUSB_ep0Toggle | myUSB_ep0Count;
                myUSB_EP0_CNT_REG = tempReg;
               
                /* Make sure that previous write operaiton was successful */
                if (tempReg == myUSB_EP0_CNT_REG)
                {
                    /* Repeat until next successful write operation */
                    do
                    {
                        /* Init temporary variable */
                        modifyReg = myUSB_ep0Mode;
                        
                        /* Unlock register */
                        tempReg = (uint8) (myUSB_EP0_CR_REG & myUSB_MODE_SETUP_RCVD);
                        
                        /* Check if SETUP bit is not set */
                        if (0u == tempReg)
                        {
                            /* Set the Mode Register  */
                            myUSB_EP0_CR_REG = myUSB_ep0Mode;
                            
                            /* Writing check */
                            modifyReg = myUSB_EP0_CR_REG & myUSB_MODE_MASK;
                        }
                    }
                    while (modifyReg != myUSB_ep0Mode);
                }
            }
        }
    }

    myUSB_ClearSieInterruptSource(myUSB_INTR_SIE_EP0_INTR);
	
#ifdef myUSB_EP_0_ISR_EXIT_CALLBACK
    myUSB_EP_0_ISR_ExitCallback();
#endif /* (myUSB_EP_0_ISR_EXIT_CALLBACK) */
}


/*******************************************************************************
* Function Name: myUSB_HandleSetup
****************************************************************************//**
*
*  This Routine dispatches requests for the four USB request types
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_HandleSetup(void) 
{
    uint8 requestHandled;
    
    /* Clear register lock by SIE (read register) and clear setup bit 
    * (write any value in register).
    */
    requestHandled = (uint8) myUSB_EP0_CR_REG;
    myUSB_EP0_CR_REG = (uint8) requestHandled;
    requestHandled = (uint8) myUSB_EP0_CR_REG;

    if ((requestHandled & myUSB_MODE_SETUP_RCVD) != 0u)
    {
        /* SETUP bit is set: exit without mode modification. */
        myUSB_ep0Mode = requestHandled;
    }
    else
    {
        /* In case the previous transfer did not complete, close it out */
        myUSB_UpdateStatusBlock(myUSB_XFER_PREMATURE);

        /* Check request type. */
        switch (myUSB_bmRequestTypeReg & myUSB_RQST_TYPE_MASK)
        {
            case myUSB_RQST_TYPE_STD:
                requestHandled = myUSB_HandleStandardRqst();
                break;
                
            case myUSB_RQST_TYPE_CLS:
                requestHandled = myUSB_DispatchClassRqst();
                break;
                
            case myUSB_RQST_TYPE_VND:
                requestHandled = myUSB_HandleVendorRqst();
                break;
                
            default:
                requestHandled = myUSB_FALSE;
                break;
        }
        
        /* If request is not recognized. Stall endpoint 0 IN and OUT. */
        if (requestHandled == myUSB_FALSE)
        {
            myUSB_ep0Mode = myUSB_MODE_STALL_IN_OUT;
        }
    }
}


/*******************************************************************************
* Function Name: myUSB_HandleIN
****************************************************************************//**
*
*  This routine handles EP0 IN transfers.
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_HandleIN(void) 
{
    switch (myUSB_transferState)
    {
        case myUSB_TRANS_STATE_IDLE:
            break;
        
        case myUSB_TRANS_STATE_CONTROL_READ:
            myUSB_ControlReadDataStage();
            break;
            
        case myUSB_TRANS_STATE_CONTROL_WRITE:
            myUSB_ControlWriteStatusStage();
            break;
            
        case myUSB_TRANS_STATE_NO_DATA_CONTROL:
            myUSB_NoDataControlStatusStage();
            break;
            
        default:    /* there are no more states */
            break;
    }
}


/*******************************************************************************
* Function Name: myUSB_HandleOUT
****************************************************************************//**
*
*  This routine handles EP0 OUT transfers.
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_HandleOUT(void) 
{
    switch (myUSB_transferState)
    {
        case myUSB_TRANS_STATE_IDLE:
            break;
        
        case myUSB_TRANS_STATE_CONTROL_READ:
            myUSB_ControlReadStatusStage();
            break;
            
        case myUSB_TRANS_STATE_CONTROL_WRITE:
            myUSB_ControlWriteDataStage();
            break;
            
        case myUSB_TRANS_STATE_NO_DATA_CONTROL:
            /* Update the completion block */
            myUSB_UpdateStatusBlock(myUSB_XFER_ERROR);
            
            /* We expect no more data, so stall INs and OUTs */
            myUSB_ep0Mode = myUSB_MODE_STALL_IN_OUT;
            break;
            
        default:    
            /* There are no more states */
            break;
    }
}


/*******************************************************************************
* Function Name: myUSB_LoadEP0
****************************************************************************//**
*
*  This routine loads the EP0 data registers for OUT transfers. It uses the
*  currentTD (previously initialized by the _InitControlWrite function and
*  updated for each OUT transfer, and the bLastPacketSize) to determine how
*  many uint8s to transfer on the current OUT.
*
*  If the number of uint8s remaining is zero and the last transfer was full,
*  we need to send a zero length packet.  Otherwise we send the minimum
*  of the control endpoint size (8) or remaining number of uint8s for the
*  transaction.
*
*
* \globalvars
*  myUSB_transferByteCount - Update the transfer byte count from the
*     last transaction.
*  myUSB_ep0Count - counts the data loaded to the SIE memory in
*     current packet.
*  myUSB_lastPacketSize - remembers the USBFS_ep0Count value for the
*     next packet.
*  myUSB_transferByteCount - sum of the previous bytes transferred
*     on previous packets(sum of USBFS_lastPacketSize)
*  myUSB_ep0Toggle - inverted
*  myUSB_ep0Mode  - prepare for mode register content.
*  myUSB_transferState - set to TRANS_STATE_CONTROL_READ
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_LoadEP0(void) 
{
    uint8 ep0Count = 0u;

    /* Update the transfer byte count from the last transaction */
    myUSB_transferByteCount += myUSB_lastPacketSize;

    /* Now load the next transaction */
    while ((myUSB_currentTD.count > 0u) && (ep0Count < 8u))
    {
        myUSB_EP0_DR_BASE.epData[ep0Count] = (uint8) *myUSB_currentTD.pData;
        myUSB_currentTD.pData = &myUSB_currentTD.pData[1u];
        ep0Count++;
        myUSB_currentTD.count--;
    }

    /* Support zero-length packet */
    if ((myUSB_lastPacketSize == 8u) || (ep0Count > 0u))
    {
        /* Update the data toggle */
        myUSB_ep0Toggle ^= myUSB_EP0_CNT_DATA_TOGGLE;
        /* Set the Mode Register  */
        myUSB_ep0Mode = myUSB_MODE_ACK_IN_STATUS_OUT;
        /* Update the state (or stay the same) */
        myUSB_transferState = myUSB_TRANS_STATE_CONTROL_READ;
    }
    else
    {
        /* Expect Status Stage Out */
        myUSB_ep0Mode = myUSB_MODE_STATUS_OUT_ONLY;
        /* Update the state (or stay the same) */
        myUSB_transferState = myUSB_TRANS_STATE_CONTROL_READ;
    }

    /* Save the packet size for next time */
    myUSB_ep0Count =       (uint8) ep0Count;
    myUSB_lastPacketSize = (uint8) ep0Count;
}


/*******************************************************************************
* Function Name: myUSB_InitControlRead
****************************************************************************//**
*
*  Initialize a control read transaction. It is used to send data to the host.
*  The following global variables should be initialized before this function
*  called. To send zero length packet use InitZeroLengthControlTransfer
*  function.
*
*
* \return
*  requestHandled state.
*
* \globalvars
*  myUSB_currentTD.count - counts of data to be sent.
*  myUSB_currentTD.pData - data pointer.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_InitControlRead(void) 
{
    uint16 xferCount;

    if (myUSB_currentTD.count == 0u)
    {
        (void) myUSB_InitZeroLengthControlTransfer();
    }
    else
    {
        /* Set up the state machine */
        myUSB_transferState = myUSB_TRANS_STATE_CONTROL_READ;
        
        /* Set the toggle, it gets updated in LoadEP */
        myUSB_ep0Toggle = 0u;
        
        /* Initialize the Status Block */
        myUSB_InitializeStatusBlock();
        
        xferCount = ((uint16)((uint16) myUSB_lengthHiReg << 8u) | ((uint16) myUSB_lengthLoReg));

        if (myUSB_currentTD.count > xferCount)
        {
            myUSB_currentTD.count = xferCount;
        }
        
        myUSB_LoadEP0();
    }

    return (myUSB_TRUE);
}


/*******************************************************************************
* Function Name: myUSB_InitZeroLengthControlTransfer
****************************************************************************//**
*
*  Initialize a zero length data IN transfer.
*
* \return
*  requestHandled state.
*
* \globalvars
*  myUSB_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*  myUSB_ep0Mode  - prepare for mode register content.
*  myUSB_transferState - set to TRANS_STATE_CONTROL_READ
*  myUSB_ep0Count - cleared, means the zero-length packet.
*  myUSB_lastPacketSize - cleared.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_InitZeroLengthControlTransfer(void)
                                                
{
    /* Update the state */
    myUSB_transferState = myUSB_TRANS_STATE_CONTROL_READ;
    
    /* Set the data toggle */
    myUSB_ep0Toggle = myUSB_EP0_CNT_DATA_TOGGLE;
    
    /* Set the Mode Register  */
    myUSB_ep0Mode = myUSB_MODE_ACK_IN_STATUS_OUT;
    
    /* Save the packet size for next time */
    myUSB_lastPacketSize = 0u;
    
    myUSB_ep0Count = 0u;

    return (myUSB_TRUE);
}


/*******************************************************************************
* Function Name: myUSB_ControlReadDataStage
****************************************************************************//**
*
*  Handle the Data Stage of a control read transfer.
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_ControlReadDataStage(void) 

{
    myUSB_LoadEP0();
}


/*******************************************************************************
* Function Name: myUSB_ControlReadStatusStage
****************************************************************************//**
*
*  Handle the Status Stage of a control read transfer.
*
*
* \globalvars
*  myUSB_USBFS_transferByteCount - updated with last packet size.
*  myUSB_transferState - set to TRANS_STATE_IDLE.
*  myUSB_ep0Mode  - set to MODE_STALL_IN_OUT.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_ControlReadStatusStage(void) 
{
    /* Update the transfer byte count */
    myUSB_transferByteCount += myUSB_lastPacketSize;
    
    /* Go Idle */
    myUSB_transferState = myUSB_TRANS_STATE_IDLE;
    
    /* Update the completion block */
    myUSB_UpdateStatusBlock(myUSB_XFER_STATUS_ACK);
    
    /* We expect no more data, so stall INs and OUTs */
    myUSB_ep0Mode = myUSB_MODE_STALL_IN_OUT;
}


/*******************************************************************************
* Function Name: myUSB_InitControlWrite
****************************************************************************//**
*
*  Initialize a control write transaction
*
* \return
*  requestHandled state.
*
* \globalvars
*  myUSB_USBFS_transferState - set to TRANS_STATE_CONTROL_WRITE
*  myUSB_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*  myUSB_ep0Mode  - set to MODE_ACK_OUT_STATUS_IN
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_InitControlWrite(void) 
{
    uint16 xferCount;

    /* Set up the state machine */
    myUSB_transferState = myUSB_TRANS_STATE_CONTROL_WRITE;
    
    /* This might not be necessary */
    myUSB_ep0Toggle = myUSB_EP0_CNT_DATA_TOGGLE;
    
    /* Initialize the Status Block */
    myUSB_InitializeStatusBlock();

    xferCount = ((uint16)((uint16) myUSB_lengthHiReg << 8u) | ((uint16) myUSB_lengthLoReg));

    if (myUSB_currentTD.count > xferCount)
    {
        myUSB_currentTD.count = xferCount;
    }

    /* Expect Data or Status Stage */
    myUSB_ep0Mode = myUSB_MODE_ACK_OUT_STATUS_IN;

    return(myUSB_TRUE);
}


/*******************************************************************************
* Function Name: myUSB_ControlWriteDataStage
****************************************************************************//**
*
*  Handle the Data Stage of a control write transfer
*       1. Get the data (We assume the destination was validated previously)
*       2. Update the count and data toggle
*       3. Update the mode register for the next transaction
*
*
* \globalvars
*  myUSB_transferByteCount - Update the transfer byte count from the
*    last transaction.
*  myUSB_ep0Count - counts the data loaded from the SIE memory
*    in current packet.
*  myUSB_transferByteCount - sum of the previous bytes transferred
*    on previous packets(sum of USBFS_lastPacketSize)
*  myUSB_ep0Toggle - inverted
*  myUSB_ep0Mode  - set to MODE_ACK_OUT_STATUS_IN.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_ControlWriteDataStage(void) 
{
    uint8 ep0Count;
    uint8 regIndex = 0u;

    ep0Count = (myUSB_EP0_CNT_REG & myUSB_EPX_CNT0_MASK) - myUSB_EPX_CNTX_CRC_COUNT;

    myUSB_transferByteCount += (uint8)ep0Count;

    while ((myUSB_currentTD.count > 0u) && (ep0Count > 0u))
    {
        *myUSB_currentTD.pData = (uint8) myUSB_EP0_DR_BASE.epData[regIndex];
        myUSB_currentTD.pData = &myUSB_currentTD.pData[1u];
        regIndex++;
        ep0Count--;
        myUSB_currentTD.count--;
    }
    
    myUSB_ep0Count = (uint8)ep0Count;
    
    /* Update the data toggle */
    myUSB_ep0Toggle ^= myUSB_EP0_CNT_DATA_TOGGLE;
    
    /* Expect Data or Status Stage */
    myUSB_ep0Mode = myUSB_MODE_ACK_OUT_STATUS_IN;
}


/*******************************************************************************
* Function Name: myUSB_ControlWriteStatusStage
****************************************************************************//**
*
*  Handle the Status Stage of a control write transfer
*
* \globalvars
*  myUSB_transferState - set to TRANS_STATE_IDLE.
*  myUSB_USBFS_ep0Mode  - set to MODE_STALL_IN_OUT.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_ControlWriteStatusStage(void) 
{
    /* Go Idle */
    myUSB_transferState = myUSB_TRANS_STATE_IDLE;
    
    /* Update the completion block */    
    myUSB_UpdateStatusBlock(myUSB_XFER_STATUS_ACK);
    
    /* We expect no more data, so stall INs and OUTs */
    myUSB_ep0Mode = myUSB_MODE_STALL_IN_OUT;
}


/*******************************************************************************
* Function Name: myUSB_InitNoDataControlTransfer
****************************************************************************//**
*
*  Initialize a no data control transfer
*
* \return
*  requestHandled state.
*
* \globalvars
*  myUSB_transferState - set to TRANS_STATE_NO_DATA_CONTROL.
*  myUSB_ep0Mode  - set to MODE_STATUS_IN_ONLY.
*  myUSB_ep0Count - cleared.
*  myUSB_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_InitNoDataControlTransfer(void) 
{
    myUSB_transferState = myUSB_TRANS_STATE_NO_DATA_CONTROL;
    myUSB_ep0Mode       = myUSB_MODE_STATUS_IN_ONLY;
    myUSB_ep0Toggle     = myUSB_EP0_CNT_DATA_TOGGLE;
    myUSB_ep0Count      = 0u;

    return (myUSB_TRUE);
}


/*******************************************************************************
* Function Name: myUSB_NoDataControlStatusStage
****************************************************************************//**
*  Handle the Status Stage of a no data control transfer.
*
*  SET_ADDRESS is special, since we need to receive the status stage with
*  the old address.
*
* \globalvars
*  myUSB_transferState - set to TRANS_STATE_IDLE.
*  myUSB_ep0Mode  - set to MODE_STALL_IN_OUT.
*  myUSB_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*  myUSB_deviceAddress - used to set new address and cleared
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_NoDataControlStatusStage(void) 
{
    if (0u != myUSB_deviceAddress)
    {
        /* Update device address if we got new address. */
        myUSB_CR0_REG = (uint8) myUSB_deviceAddress | myUSB_CR0_ENABLE;
        myUSB_deviceAddress = 0u;
    }

    myUSB_transferState = myUSB_TRANS_STATE_IDLE;
    
    /* Update the completion block. */
    myUSB_UpdateStatusBlock(myUSB_XFER_STATUS_ACK);
    
    /* Stall IN and OUT, no more data is expected. */
    myUSB_ep0Mode = myUSB_MODE_STALL_IN_OUT;
}


/*******************************************************************************
* Function Name: myUSB_UpdateStatusBlock
****************************************************************************//**
*
*  Update the Completion Status Block for a Request.  The block is updated
*  with the completion code the myUSB_transferByteCount.  The
*  StatusBlock Pointer is set to NULL.
*
*  completionCode - status.
*
*
* \globalvars
*  myUSB_currentTD.pStatusBlock->status - updated by the
*    completionCode parameter.
*  myUSB_currentTD.pStatusBlock->length - updated.
*  myUSB_currentTD.pStatusBlock - cleared.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_UpdateStatusBlock(uint8 completionCode) 
{
    if (myUSB_currentTD.pStatusBlock != NULL)
    {
        myUSB_currentTD.pStatusBlock->status = completionCode;
        myUSB_currentTD.pStatusBlock->length = myUSB_transferByteCount;
        myUSB_currentTD.pStatusBlock = NULL;
    }
}


/*******************************************************************************
* Function Name: myUSB_InitializeStatusBlock
****************************************************************************//**
*
*  Initialize the Completion Status Block for a Request.  The completion
*  code is set to USB_XFER_IDLE.
*
*  Also, initializes myUSB_transferByteCount.  Save some space,
*  this is the only consumer.
*
* \globalvars
*  myUSB_currentTD.pStatusBlock->status - set to XFER_IDLE.
*  myUSB_currentTD.pStatusBlock->length - cleared.
*  myUSB_transferByteCount - cleared.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_InitializeStatusBlock(void) 
{
    myUSB_transferByteCount = 0u;
    
    if (myUSB_currentTD.pStatusBlock != NULL)
    {
        myUSB_currentTD.pStatusBlock->status = myUSB_XFER_IDLE;
        myUSB_currentTD.pStatusBlock->length = 0u;
    }
}


/* [] END OF FILE */
