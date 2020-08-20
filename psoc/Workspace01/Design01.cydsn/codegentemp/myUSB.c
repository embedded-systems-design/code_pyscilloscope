/***************************************************************************//**
* \file myUSB.c
* \version 3.20
*
* \brief
*  This file contains the global USBFS API functions.
*
* Note:
*  Many of the functions use an endpoint number. SRAM arrays are sized with 9
*  elements, so they are indexed directly by epNumber.  The SIE and ARB
*  registers are indexed by variations of epNumber - 1.
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myUSB_pvt.h"
#include "myUSB_cydmac.h"
#include "myUSB_hid.h"
#include "myUSB_Dp.h"


/***************************************
* Global data allocation
***************************************/

/** Indicates whether the USBFS has been initialized. The variable is
* initialized to 0 after device reset and set to 1 the first time USBFS_Start()
* is called. This allows the Component to restart without reinitialization after
* the first call to the USBFS_Start() routine.
* If re-initialization of the Component is required, the variable should be set
* to 0 before the USBFS_Start() routine is called. Alternatively, the USBFS can
* be reinitialized by calling both USBFS_Init() and USBFS_InitComponent()
* functions.
*/
uint8 myUSB_initVar = 0u;

#if (myUSB_EP_MANAGEMENT_DMA)
    #if (CY_PSOC4)
        static void myUSB_InitEpDma(void);

        /* DMA chanels assigend for endpoints. */
        const uint8 myUSB_DmaChan[myUSB_MAX_EP] =
        {
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
        };        
    #else
        /* DMA chanels assigend for endpoints. */
        uint8 myUSB_DmaChan[myUSB_MAX_EP];
        
        /* DMA TDs require for PSoC 3/5LP operation. */
        uint8 myUSB_DmaTd[myUSB_MAX_EP];
    #endif /* (CY_PSOC4) */
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

#if (myUSB_EP_MANAGEMENT_DMA_AUTO)
#if (CY_PSOC4)
    /* Number of DMA bursts. */
    uint8  myUSB_DmaEpBurstCnt   [myUSB_MAX_EP];
    
    /* Number of bytes to transfer in last DMA burst. */
    uint8  myUSB_DmaEpLastBurstEl[myUSB_MAX_EP];

    /* Storage for arrays above. */
    uint8  myUSB_DmaEpBurstCntBackup  [myUSB_MAX_EP];
    uint32 myUSB_DmaEpBufferAddrBackup[myUSB_MAX_EP];
     
    /* DMA trigger mux output for usb.dma_req[0-7]. */
    const uint8 myUSB_DmaReqOut[myUSB_MAX_EP] =
    {
        0u,
        myUSB_ep1_dma__TR_OUTPUT,
        myUSB_ep2_dma__TR_OUTPUT,
        myUSB_ep3_dma__TR_OUTPUT,
        0u,
        0u,
        0u,
        0u,
        0u,
    };

    /* DMA trigger mux output for usb.dma_burstend[0-7]. */
    const uint8 myUSB_DmaBurstEndOut[myUSB_MAX_EP] =
    {
        0u,
        myUSB_BURSTEND_0_TR_OUTPUT,
        myUSB_BURSTEND_1_TR_OUTPUT,
        myUSB_BURSTEND_2_TR_OUTPUT,
        myUSB_BURSTEND_3_TR_OUTPUT,
        myUSB_BURSTEND_4_TR_OUTPUT,
        myUSB_BURSTEND_5_TR_OUTPUT,
        myUSB_BURSTEND_6_TR_OUTPUT,
        myUSB_BURSTEND_7_TR_OUTPUT
    };
    
#else
    #if (myUSB_EP_DMA_AUTO_OPT == 0u)
        static uint8 clearInDataRdyStatus = myUSB_ARB_EPX_CFG_DEFAULT;
        uint8 myUSB_DmaNextTd[myUSB_MAX_EP];
        const uint8 myUSB_epX_TD_TERMOUT_EN[myUSB_MAX_EP] =
        {
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

        volatile uint16 myUSB_inLength[myUSB_MAX_EP];
        const uint8 *myUSB_inDataPointer[myUSB_MAX_EP];
        volatile uint8 myUSB_inBufFull[myUSB_MAX_EP];
    #endif /* (myUSB_EP_DMA_AUTO_OPT == 0u) */
#endif /* (CY_PSOC4) */
#endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */


/*******************************************************************************
* Function Name: myUSB_Start
****************************************************************************//**
*
*   This function performs all required initialization for the USBFS component.
*   After this function call, the USB device initiates communication with the
*   host by pull-up D+ line. This is the preferred method to begin component
*   operation.
*
*   Note that global interrupts have to be enabled because interrupts are
*   required for USBFS component operation.
*
*   PSoC 4200L devices: when USBFS component configured to DMA with Automatic
*   Buffer Management, the DMA interrupt priority is changed to the highest
*   (priority 0) inside this function.
*
*   PSoC 3/PSoC 5LP devices: when USBFS component configured to DMA with
*   Automatic Buffer Management, the Arbiter interrupt priority is changed to
*   the highest (priority 0) inside this function.
*
*  \param device
*          Contains the device number of the desired device descriptor.
*          The device number can be found in the Device Descriptor Tab of
*          "Configure" dialog, under the settings of desired Device Descriptor,
*          in the "Device Number" field.
*  \param mode:
*   The operating voltage. This determines whether the voltage regulator
*   is enabled for 5V operation or if pass through mode is used for 3.3V
*   operation. Symbolic names and their associated values are given in the
*   following list.
*
*       *myUSB_3V_OPERATION* - Disable voltage regulator and pass-
*                                      through Vcc for pull-up
*
*       *myUSB_5V_OPERATION* - Enable voltage regulator and use
*                                      regulator for pull-up
*
*       *myUSB_DWR_POWER_OPERATION* - Enable or disable the voltage
*                                      regulator depending on the power supply
*                                      voltage configuration in the DWR tab.
*                                      For PSoC 3/5LP devices, the VDDD supply
*                                      voltage is considered and for PSoC 4A-L,
*                                      the VBUS supply voltage is considered.*
* \globalvars
*  \ref myUSB_initVar
*
* \sideeffect
*   This function will reset all communication states to default.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_Start(uint8 device, uint8 mode) 
{
    if (0u == myUSB_initVar)
    {
        myUSB_Init();
        myUSB_initVar = 1u;
    }

    myUSB_InitComponent(device, mode);
}


/*******************************************************************************
* Function Name: myUSB_Init
****************************************************************************//**
*
* This function initializes or restores the component according to the
* customizer Configure dialog settings. It is not necessary to call
* myUSB_Init() because the myUSB_Start() routine calls
* this function and is the preferred method to begin component operation.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_Init(void) 
{
#if (CY_PSOC4)
    /* Enable clock to USB IP. */
    myUSB_USB_CLK_EN_REG = myUSB_USB_CLK_CSR_CLK_EN;

    /* The internal regulator (CR1.REG_ENABLE) is enabled in
    * myUSB_InitComponent() if it is required.
    */

    /* Enable USBIO control on drive mode of D+ and D- pins. */
    myUSB_USBIO_CR1_REG &= ~ (uint32) myUSB_USBIO_CR1_IOMODE;

    /* Set number of LF CLK to detect UBS bus reset. */
    myUSB_BUS_RST_CNT_REG = myUSB_DEFUALT_BUS_RST_CNT;

    /* Select VBUS detection source and clear PHY isolate. The application level
    * must ensure that VBUS is valid. There is no need to wait 2us before VBUS is valid.
    */
    myUSB_POWER_CTRL_REG = myUSB_DEFAULT_POWER_CTRL_VBUS;

    /* Enable PHY detector and single-ended and differential receivers. */
    myUSB_POWER_CTRL_REG |= myUSB_DEFAULT_POWER_CTRL_PHY;

    /* Suspend clear sequence. */
    myUSB_POWER_CTRL_REG &= (uint32) ~myUSB_POWER_CTRL_SUSPEND;
    CyDelayUs(myUSB_WAIT_SUSPEND_DEL_DISABLE);
    myUSB_POWER_CTRL_REG &= (uint32) ~myUSB_POWER_CTRL_SUSPEND_DEL;

    /* Sets IMO lock options and clear all other bits. */
    myUSB_CR1_REG = myUSB_DEFUALT_CR1;

    /* Configure level (hi, lo, med) for each interrupt source. */
    myUSB_INTR_LVL_SEL_REG = myUSB_DEFAULT_INTR_LVL_SEL;

    /* Configure interrupt sources from: SOF, Bus Reset and EP0. */
    myUSB_INTR_SIE_MASK_REG = myUSB_DEFAULT_INTR_SIE_MASK;

#else
    uint8 enableInterrupts = CyEnterCriticalSection();

#if (myUSB_EP_MANAGEMENT_DMA)
    uint16 i;
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

    /* Enable USB block. */
    myUSB_PM_ACT_CFG_REG |= myUSB_PM_ACT_EN_FSUSB;
    /* Enable USB block for Standby Power Mode. */
    myUSB_PM_STBY_CFG_REG |= myUSB_PM_STBY_EN_FSUSB;

    /* Enable core clock. */
    myUSB_USB_CLK_EN_REG = myUSB_USB_CLK_ENABLE;

    myUSB_CR1_REG = myUSB_CR1_ENABLE_LOCK;

    /* ENABLING USBIO PADS IN USB MODE FROM I/O MODE. */
    /* Ensure USB transmit enable is low (USB_USBIO_CR0.ten). - Manual Transmission - Disabled. */
    myUSB_USBIO_CR0_REG &= (uint8) ~myUSB_USBIO_CR0_TEN;
    CyDelayUs(myUSB_WAIT_REG_STABILITY_50NS);  /* ~50ns delay. */
    /* Disable USBIO by asserting PM.USB_CR0.fsusbio_pd_n(Inverted.
    *  high. These bits will be set low by the power manager out-of-reset.
    *  Also confirm USBIO pull-up is disabled.
    */
    myUSB_PM_USB_CR0_REG &= (uint8) ~(myUSB_PM_USB_CR0_PD_N |
                                                 myUSB_PM_USB_CR0_PD_PULLUP_N);

    /* Select IOMODE to USB mode. */
    myUSB_USBIO_CR1_REG &= (uint8) ~myUSB_USBIO_CR1_IOMODE;

    /* Enable USBIO reference by setting PM.USB_CR0.fsusbio_ref_en. */
    myUSB_PM_USB_CR0_REG |= myUSB_PM_USB_CR0_REF_EN;
    /* Reference is available for 1us after regulator is enabled. */
    CyDelayUs(myUSB_WAIT_REG_STABILITY_1US);
    /* OR 40us after power is restored. */
    CyDelayUs(myUSB_WAIT_VREF_RESTORE);
    /* Ensure single-ended disable bits are low (PRT15.INP_DIS[7:6])(input receiver enabled). */
    myUSB_DM_INP_DIS_REG &= (uint8) ~myUSB_DM_MASK;
    myUSB_DP_INP_DIS_REG &= (uint8) ~myUSB_DP_MASK;

    /* Enable USBIO. */
    myUSB_PM_USB_CR0_REG |= myUSB_PM_USB_CR0_PD_N;
    CyDelayUs(myUSB_WAIT_PD_PULLUP_N_ENABLE);
    /* Set USBIO pull-up enable. */
    myUSB_PM_USB_CR0_REG |= myUSB_PM_USB_CR0_PD_PULLUP_N;

    /* Reset Arbiter Write Address register for endpoint 1. */
    CY_SET_REG8(myUSB_ARB_RW1_WA_PTR,     0u);
    CY_SET_REG8(myUSB_ARB_RW1_WA_MSB_PTR, 0u);

#if (myUSB_EP_MANAGEMENT_DMA)
    /* Initialize transfer descriptor. This will be used to detect DMA state - initialized or not. */
    for (i = 0u; i < myUSB_MAX_EP; ++i)
    {
        myUSB_DmaTd[i] = DMA_INVALID_TD;

    #if (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u))
        myUSB_DmaNextTd[i] = DMA_INVALID_TD;
    #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u)) */
    }
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

    CyExitCriticalSection(enableInterrupts);
#endif /* (CY_PSOC4) */

    /* Configure interrupts from USB block. */
#if (CY_PSOC4)
    /* Configure hi_int: set handler and priority. */
    CyIntSetPriority     (myUSB_INTR_HI_VECT_NUM,  myUSB_INTR_HI_PRIORITY);
    (void) CyIntSetVector(myUSB_INTR_HI_VECT_NUM, &myUSB_INTR_HI_ISR);

    /* Configure lo_int: set handler and priority. */
    CyIntSetPriority     (myUSB_INTR_LO_VECT_NUM,  myUSB_INTR_LO_PRIORITY);
    (void) CyIntSetVector(myUSB_INTR_LO_VECT_NUM, &myUSB_INTR_LO_ISR);

    /* Configure med_int: set handler and priority (routed through DSI). */
    CyIntSetPriority     (myUSB_INTR_MED_VECT_NUM,  myUSB_INTR_MED_PRIORITY);
    (void) CyIntSetVector(myUSB_INTR_MED_VECT_NUM, &myUSB_INTR_MED_ISR);

#else
    /* Set bus reset interrupt. */
    CyIntSetPriority(myUSB_BUS_RESET_VECT_NUM, myUSB_BUS_RESET_PRIOR);
    (void) CyIntSetVector(myUSB_BUS_RESET_VECT_NUM,   &myUSB_BUS_RESET_ISR);

    /* Set Control Endpoint Interrupt. */
    CyIntSetPriority(myUSB_EP_0_VECT_NUM, myUSB_EP_0_PRIOR);
    (void) CyIntSetVector(myUSB_EP_0_VECT_NUM,   &myUSB_EP_0_ISR);

    /* Set SOF interrupt. */
    #if (myUSB_SOF_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_SOF_VECT_NUM,  myUSB_SOF_PRIOR);
        (void) CyIntSetVector(myUSB_SOF_VECT_NUM, &myUSB_SOF_ISR);
    #endif /* (myUSB_SOF_ISR_ACTIVE) */

    /* Set Data Endpoint 1 Interrupt. */
    #if (myUSB_EP1_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_1_VECT_NUM,  myUSB_EP_1_PRIOR);
        (void) CyIntSetVector(myUSB_EP_1_VECT_NUM,  &myUSB_EP_1_ISR);
    #endif /* (myUSB_EP1_ISR_ACTIVE) */

    /* Set Data Endpoint 2 Interrupt. */
    #if (myUSB_EP2_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_2_VECT_NUM,  myUSB_EP_2_PRIOR);
        (void) CyIntSetVector(myUSB_EP_2_VECT_NUM, &myUSB_EP_2_ISR);
    #endif /* (myUSB_EP2_ISR_ACTIVE) */

    /* Set Data Endpoint 3 Interrupt. */
    #if (myUSB_EP3_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_3_VECT_NUM,  myUSB_EP_3_PRIOR);
        (void) CyIntSetVector(myUSB_EP_3_VECT_NUM, &myUSB_EP_3_ISR);
    #endif /* (myUSB_EP3_ISR_ACTIVE) */

    /* Set Data Endpoint 4 Interrupt. */
    #if (myUSB_EP4_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_4_VECT_NUM,  myUSB_EP_4_PRIOR);
        (void) CyIntSetVector(myUSB_EP_4_VECT_NUM, &myUSB_EP_4_ISR);
    #endif /* (myUSB_EP4_ISR_ACTIVE) */

    /* Set Data Endpoint 5 Interrupt. */
    #if (myUSB_EP5_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_5_VECT_NUM,  myUSB_EP_5_PRIOR);
        (void) CyIntSetVector(myUSB_EP_5_VECT_NUM, &myUSB_EP_5_ISR);
    #endif /* (myUSB_EP5_ISR_ACTIVE) */

    /* Set Data Endpoint 6 Interrupt. */
    #if (myUSB_EP6_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_6_VECT_NUM,  myUSB_EP_6_PRIOR);
        (void) CyIntSetVector(myUSB_EP_6_VECT_NUM, &myUSB_EP_6_ISR);
    #endif /* (myUSB_EP6_ISR_ACTIVE) */

     /* Set Data Endpoint 7 Interrupt. */
    #if (myUSB_EP7_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_7_VECT_NUM,  myUSB_EP_7_PRIOR);
        (void) CyIntSetVector(myUSB_EP_7_VECT_NUM, &myUSB_EP_7_ISR);
    #endif /* (myUSB_EP7_ISR_ACTIVE) */

    /* Set Data Endpoint 8 Interrupt. */
    #if (myUSB_EP8_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_EP_8_VECT_NUM,  myUSB_EP_8_PRIOR);
        (void) CyIntSetVector(myUSB_EP_8_VECT_NUM, &myUSB_EP_8_ISR);
    #endif /* (myUSB_EP8_ISR_ACTIVE) */

    /* Set ARB Interrupt. */
    #if (myUSB_EP_MANAGEMENT_DMA && myUSB_ARB_ISR_ACTIVE)
        CyIntSetPriority     (myUSB_ARB_VECT_NUM,  myUSB_ARB_PRIOR);
        (void) CyIntSetVector(myUSB_ARB_VECT_NUM, &myUSB_ARB_ISR);
    #endif /* (myUSB_EP_MANAGEMENT_DMA && myUSB_ARB_ISR_ACTIVE) */
#endif /* (CY_PSOC4) */

    /* Common: Configure GPIO interrupt for wakeup. */
#if (myUSB_DP_ISR_ACTIVE)
    CyIntSetPriority     (myUSB_DP_INTC_VECT_NUM,  myUSB_DP_INTC_PRIORITY);
    (void) CyIntSetVector(myUSB_DP_INTC_VECT_NUM, &myUSB_DP_ISR);
#endif /* (myUSB_DP_ISR_ACTIVE) */

#if (myUSB_EP_MANAGEMENT_DMA && CY_PSOC4)
    /* Initialize DMA channels. */
    myUSB_InitEpDma();
#endif /* (myUSB_EP_MANAGEMENT_DMA && CY_PSOC4) */
}


/*******************************************************************************
* Function Name: myUSB_InitComponent
****************************************************************************//**
*
*   This function initializes the component’s global variables and initiates
*   communication with the host by pull-up D+ line.
*
* \param device:
*   Contains the device number of the desired device descriptor. The device
*   number can be found in the Device Descriptor Tab of "Configure" dialog,
*   under the settings of desired Device Descriptor, in the *Device Number*
*   field.
*  \param mode:
*   The operating voltage. This determines whether the voltage regulator
*   is enabled for 5V operation or if pass through mode is used for 3.3V
*   operation. Symbolic names and their associated values are given in the
*   following list.
*
*       *myUSB_3V_OPERATION* - Disable voltage regulator and pass-
*                                      through Vcc for pull-up
*
*       *myUSB_5V_OPERATION* - Enable voltage regulator and use
*                                      regulator for pull-up
*
*       *myUSB_DWR_POWER_OPERATION* - Enable or disable the voltage
*                                      regulator depending on the power supply
*                                      voltage configuration in the DWR tab.
*                                      For PSoC 3/5LP devices, the VDDD supply
*                                      voltage is considered and for PSoC 4A-L,
*                                      the VBUS supply voltage is considered.
*
* \globalvars
*   \ref myUSB_device
*   \ref myUSB_transferState
*   \ref myUSB_configuration
*   \ref myUSB_deviceStatus
*
*   \ref myUSB_deviceAddress - Contains the current device address. This
*       variable is initialized to zero in this API. The Host starts to communicate
*      to the device with address 0 and then sets it to a whatever value using a
*      SET_ADDRESS request.
*
*   \ref myUSB_lastPacketSize - Initialized to 0;
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_InitComponent(uint8 device, uint8 mode) 
{
    /* Initialize _hidProtocol variable to comply with
    *  HID 7.2.6 Set_Protocol Request:
    *  "When initialized, all devices default to report protocol."
    */
#if defined(myUSB_ENABLE_HID_CLASS)
    uint8 i;

    for (i = 0u; i < myUSB_MAX_INTERFACES_NUMBER; i++)
    {
        myUSB_hidProtocol[i] = myUSB_PROTOCOL_REPORT;
    }
#endif /* myUSB_ENABLE_HID_CLASS */

    /* Store device number to access descriptor. */
    myUSB_device = device;

    /* Reset component internal variables. */
    myUSB_transferState   = myUSB_TRANS_STATE_IDLE;
    myUSB_configurationChanged = 0u;
    myUSB_configuration   = 0u;
    myUSB_interfaceNumber = 0u;
    myUSB_deviceAddress   = 0u;
    myUSB_deviceStatus    = 0u;
    myUSB_lastPacketSize  = 0u;

    /* Enable component interrupts. */
#if (CY_PSOC4)
    CyIntEnable(myUSB_INTR_HI_VECT_NUM);
    CyIntEnable(myUSB_INTR_MED_VECT_NUM);
    CyIntEnable(myUSB_INTR_LO_VECT_NUM);
#else
    CyIntEnable(myUSB_BUS_RESET_VECT_NUM);
    CyIntEnable(myUSB_EP_0_VECT_NUM);

    #if (myUSB_SOF_ISR_ACTIVE)
        CyIntEnable(myUSB_SOF_VECT_NUM);
    #endif /* (myUSB_SOF_ISR_ACTIVE) */

    #if (myUSB_EP1_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_1_VECT_NUM);
    #endif /* (myUSB_EP1_ISR_ACTIVE) */

    #if (myUSB_EP2_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_2_VECT_NUM);
    #endif /* (myUSB_EP5_ISR_ACTIVE) */

    #if (myUSB_EP3_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_3_VECT_NUM);
    #endif /* (myUSB_EP5_ISR_ACTIVE) */

    #if (myUSB_EP4_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_4_VECT_NUM);
    #endif /* (myUSB_EP5_ISR_ACTIVE) */

    #if (myUSB_EP5_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_5_VECT_NUM);
    #endif /* (myUSB_EP5_ISR_ACTIVE) */

    #if (myUSB_EP6_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_6_VECT_NUM);
    #endif /* myUSB_EP6_ISR_REMOVE */

    #if (myUSB_EP7_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_7_VECT_NUM);
    #endif /* (myUSB_EP7_ISR_ACTIVE) */

    #if (myUSB_EP8_ISR_ACTIVE)
        CyIntEnable(myUSB_EP_8_VECT_NUM);
    #endif /* (myUSB_EP8_ISR_ACTIVE) */
#endif /* (CY_PSOC4) */

#if (myUSB_EP_MANAGEMENT_DMA && myUSB_ARB_ISR_ACTIVE)
    /* Enable ARB EP interrupt sources. */
    myUSB_ARB_INT_EN_REG = myUSB_DEFAULT_ARB_INT_EN;

    #if (CY_PSOC3 || CY_PSOC5)
        CyIntEnable(myUSB_ARB_VECT_NUM);
    #endif /* (CY_PSOC3 || CY_PSOC5) */
#endif   /* (myUSB_EP_MANAGEMENT_DMA && myUSB_ARB_ISR_ACTIVE) */

/* Arbiter configuration for DMA transfers. */
#if (myUSB_EP_MANAGEMENT_DMA)
    /* Configure Arbiter for Manual or Auto DMA operation and clear configuration completion. */
    myUSB_ARB_CFG_REG = myUSB_DEFAULT_ARB_CFG;

    #if (CY_PSOC4)
        /* Enable DMA operation. */
        CyDmaEnable();

        #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
            /* Change DMA priority to be highest. */
             CyIntSetPriority(CYDMA_INTR_NUMBER, myUSB_DMA_AUTO_INTR_PRIO);
        #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
    #endif /* (CY_PSOC4) */

    #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
        #if (CY_PSOC4)
            /* Enable DMA interrupt to handle DMA management. */
            CyIntEnable(CYDMA_INTR_NUMBER);
        #else
            #if (myUSB_EP_DMA_AUTO_OPT == 0u)
                /* Initialize interrupts which handle verification of successful DMA transaction. */
                myUSB_EP_DMA_Done_isr_StartEx(&myUSB_EP_DMA_DONE_ISR);
                myUSB_EP17_DMA_Done_SR_InterruptEnable();
                myUSB_EP8_DMA_Done_SR_InterruptEnable();
            #endif /* (myUSB_EP_DMA_AUTO_OPT == 0u) */
        #endif /* (CY_PSOC4) */
    #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

    /* Enable USB regulator depends on operation voltage. IMO Locking is enabled in Init(). */
    switch(mode)
    {
    case myUSB_3V_OPERATION:
        /* Disable regulator for 3V operation. */
        myUSB_CR1_REG &= (uint8) ~myUSB_CR1_REG_ENABLE;
        break;

    case myUSB_5V_OPERATION:
        /* Enable regulator for 5V operation. */
        myUSB_CR1_REG |= (uint8)  myUSB_CR1_REG_ENABLE;
        break;

    default: /* Check DWR settings of USB power supply. */
    #if (myUSB_VDDD_MV < myUSB_3500MV)
        /* Disable regulator for 3V operation. */
        myUSB_CR1_REG &= (uint8) ~myUSB_CR1_REG_ENABLE;
    #else
        /* Enable regulator for 5V operation. */
        myUSB_CR1_REG |= (uint8)  myUSB_CR1_REG_ENABLE;
    #endif /* (myUSB_VDDD_MV < myUSB_3500MV) */
        break;
    }

#if (CY_PSOC4)
    /* Clear bus activity. */
    myUSB_CR1_REG &= (uint32) ~myUSB_CR1_BUS_ACTIVITY;

    /* Clear EP0 count register. */
    myUSB_EP0_CNT_REG = myUSB_CLEAR_REG;

    /* Set EP0.CR: ACK Setup, NAK IN/OUT. */
    myUSB_EP0_CR_REG = myUSB_MODE_NAK_IN_OUT;

    #if (myUSB_LPM_ACTIVE)
        if (NULL != myUSB_GetBOSPtr())
        {
            /* Enable LPM and acknowledge LPM packets for active device.
            * Reset NYET_EN and SUB_RESP bits in the LPM_CTRL register.
            */
            myUSB_LPM_CTRL_REG = (myUSB_LPM_CTRL_LPM_EN | \
                                             myUSB_LPM_CTRL_LPM_ACK_RESP);
        }
        else
        {
            /* Disable LPM for active device. */
            myUSB_LPM_CTRL_REG &= (uint32) ~myUSB_LPM_CTRL_LPM_EN;
        }
    #endif /* (myUSB_LPM_ACTIVE) */

    /* Enable device to responds to USB traffic with address 0. */
    myUSB_CR0_REG = myUSB_DEFUALT_CR0;

#else
    /* Set EP0.CR: ACK Setup, STALL IN/OUT. */
    myUSB_EP0_CR_REG = myUSB_MODE_STALL_IN_OUT;

    /* Enable device to respond to USB traffic with address 0. */
    myUSB_CR0_REG = myUSB_DEFUALT_CR0;
    CyDelayCycles(myUSB_WAIT_CR0_REG_STABILITY);
#endif /* (CY_PSOC4) */

    /* Enable D+ pull-up and keep USB control on IO. */
    myUSB_USBIO_CR1_REG = myUSB_USBIO_CR1_USBPUEN;
}


/*******************************************************************************
* Function Name: myUSB_ReInitComponent
****************************************************************************//**
*
*  This function reinitialize the component configuration and is
*  intend to be called from the Reset interrupt.
*
* \globalvars
*   myUSB_device - Contains the device number of the desired Device
*        Descriptor. The device number can be found in the Device Descriptor tab
*       of the Configure dialog, under the settings of the desired Device Descriptor,
*       in the Device Number field.
*   myUSB_transferState - This variable is used by the communication
*       functions to handle the current transfer state. Initialized to
*       TRANS_STATE_IDLE in this API.
*   myUSB_configuration - Contains the current configuration number
*       set by the Host using a SET_CONFIGURATION request.
*       Initialized to zero in this API.
*   myUSB_deviceAddress - Contains the current device address. This
*       variable is initialized to zero in this API. The Host starts to communicate
*      to the device with address 0 and then sets it to a whatever value using
*      a SET_ADDRESS request.
*   myUSB_deviceStatus - Initialized to 0.
*       This is a two-bit variable which contains the power status in the first bit
*       (DEVICE_STATUS_BUS_POWERED or DEVICE_STATUS_SELF_POWERED) and the remote
*       wakeup status (DEVICE_STATUS_REMOTE_WAKEUP) in the second bit.
*   myUSB_lastPacketSize - Initialized to 0;
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_ReInitComponent(void) 
{
    /* Initialize _hidProtocol variable to comply with HID 7.2.6 Set_Protocol
    *  Request: "When initialized, all devices default to report protocol."
    */
#if defined(myUSB_ENABLE_HID_CLASS)
    uint8 i;

    for (i = 0u; i < myUSB_MAX_INTERFACES_NUMBER; i++)
    {
        myUSB_hidProtocol[i] = myUSB_PROTOCOL_REPORT;
    }
#endif /* myUSB_ENABLE_HID_CLASS */

    /* Reset component internal variables. */
    myUSB_transferState   = myUSB_TRANS_STATE_IDLE;
    myUSB_configurationChanged = 0u;
    myUSB_configuration   = 0u;
    myUSB_interfaceNumber = 0u;
    myUSB_deviceAddress   = 0u;
    myUSB_deviceStatus    = 0u;
    myUSB_lastPacketSize  = 0u;

#if (CY_PSOC4)
    /* Set EP0.CR: ACK Setup, NAK IN/OUT. */
    myUSB_EP0_CR_REG = myUSB_MODE_NAK_IN_OUT;
#else
    /* Set EP0.CR: ACK Setup, STALL IN/OUT. */
    myUSB_EP0_CR_REG = myUSB_MODE_STALL_IN_OUT;
#endif /* (CY_PSOC4) */

    /* Enable device to respond to USB traffic with address 0. */
    myUSB_CR0_REG = myUSB_DEFUALT_CR0;
}


/*******************************************************************************
* Function Name: myUSB_Stop
****************************************************************************//**
*
*  This function shuts down the USB function including to release
*  the D+ pull-up and disabling the SIE.
*
* \globalvars
*   \ref myUSB_configuration
*
*   myUSB_deviceAddress - Contains the current device address. This
*       variable is initialized to zero in this API. The Host starts to communicate
*      to the device with address 0 and then sets it to a whatever value using
*      a SET_ADDRESS request.
*
*   \ref myUSB_deviceStatus
*
*   \ref myUSB_configurationChanged
*
*   myUSB_intiVar -  This variable is set to zero
*
*******************************************************************************/
void myUSB_Stop(void) 
{
    uint8 enableInterrupts;

#if (myUSB_EP_MANAGEMENT_DMA)
    /* Stop all DMA channels. */
    myUSB_Stop_DMA(myUSB_MAX_EP);
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

    enableInterrupts = CyEnterCriticalSection();

    /* Disable USB IP to respond to USB traffic. */
    myUSB_CR0_REG &= (uint8) ~myUSB_CR0_ENABLE;

    /* Disable D+ pull-up. */
    myUSB_USBIO_CR1_REG &= (uint8) ~ myUSB_USBIO_CR1_USBPUEN;

#if (CY_PSOC4)
    /* Disable USBFS block. */
    myUSB_POWER_CTRL_REG &= (uint32) ~myUSB_POWER_CTRL_ENABLE;
#else
    /* Clear power active and standby mode templates. */
    myUSB_PM_ACT_CFG_REG  &= (uint8) ~myUSB_PM_ACT_EN_FSUSB;
    myUSB_PM_STBY_CFG_REG &= (uint8) ~myUSB_PM_STBY_EN_FSUSB;

    /* Ensure single-ended disable bits are high (PRT15.INP_DIS[7:6])
     * (input receiver disabled). */
    myUSB_DM_INP_DIS_REG |= (uint8) myUSB_DM_MASK;
    myUSB_DP_INP_DIS_REG |= (uint8) myUSB_DP_MASK;

#endif /* (CY_PSOC4) */

    CyExitCriticalSection(enableInterrupts);

    /* Disable component interrupts. */
#if (CY_PSOC4)
    CyIntDisable(myUSB_INTR_HI_VECT_NUM);
    CyIntDisable(myUSB_INTR_LO_VECT_NUM);
    CyIntDisable(myUSB_INTR_MED_VECT_NUM);
#else

    CyIntDisable(myUSB_BUS_RESET_VECT_NUM);
    CyIntDisable(myUSB_EP_0_VECT_NUM);

    #if (myUSB_SOF_ISR_ACTIVE)
        CyIntDisable(myUSB_SOF_VECT_NUM);
    #endif /* (myUSB_SOF_ISR_ACTIVE) */

    #if (myUSB_EP1_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_1_VECT_NUM);
    #endif /* (myUSB_EP1_ISR_ACTIVE) */

    #if (myUSB_EP2_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_2_VECT_NUM);
    #endif /* (myUSB_EP2_ISR_ACTIVE) */

    #if (myUSB_EP3_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_3_VECT_NUM);
    #endif /* (myUSB_EP3_ISR_ACTIVE) */

    #if (myUSB_EP4_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_4_VECT_NUM);
    #endif /* (myUSB_EP4_ISR_ACTIVE) */

    #if (myUSB_EP5_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_5_VECT_NUM);
    #endif /* (myUSB_EP5_ISR_ACTIVE) */

    #if (myUSB_EP6_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_6_VECT_NUM);
    #endif /* myUSB_EP6_ISR_REMOVE */

    #if (myUSB_EP7_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_7_VECT_NUM);
    #endif /* (myUSB_EP7_ISR_ACTIVE) */

    #if (myUSB_EP8_ISR_ACTIVE)
        CyIntDisable(myUSB_EP_8_VECT_NUM);
    #endif /* (myUSB_EP8_ISR_ACTIVE) */

    #if (myUSB_DP_ISR_ACTIVE)
        /* Clear active mode Dp interrupt source history. */
        (void) myUSB_Dp_ClearInterrupt();
        CyIntClearPending(myUSB_DP_INTC_VECT_NUM);
    #endif /* (myUSB_DP_ISR_ACTIVE). */

#endif /* (CY_PSOC4) */

    /* Reset component internal variables. */
    myUSB_configurationChanged = 0u;
    myUSB_configuration   = 0u;
    myUSB_interfaceNumber = 0u;
    myUSB_deviceAddress   = 0u;
    myUSB_deviceStatus    = 0u;

    /* It is mandatory for correct device startup. */
    myUSB_initVar = 0u;
}


/*******************************************************************************
* Function Name: myUSB_CheckActivity
****************************************************************************//**
*
*  This function returns the activity status of the bus. It clears the hardware
*  status to provide updated status on the next call of this function. It
*  provides a way to determine whether any USB bus activity occurred. The
*  application should use this function to determine if the USB suspend
*  conditions are met.
*
*
* \return
*  cystatus: Status of the bus since the last call of the function.
*  Return Value |   Description
*  -------------|---------------------------------------------------------------
*  1            |Bus activity was detected since the last call to this function
*  0            |Bus activity was not detected since the last call to this function
*
*
*******************************************************************************/
uint8 myUSB_CheckActivity(void) 
{
    uint8 cr1Reg = myUSB_CR1_REG;

    /* Clear bus activity. */
    myUSB_CR1_REG = (cr1Reg & (uint8) ~myUSB_CR1_BUS_ACTIVITY);

    /* Get bus activity. */
    return ((0u != (cr1Reg & myUSB_CR1_BUS_ACTIVITY)) ? (1u) : (0u));
}


/*******************************************************************************
* Function Name: myUSB_GetConfiguration
****************************************************************************//**
*
*  This function gets the current configuration of the USB device.
*
* \return
*  Returns the currently assigned configuration. Returns 0 if the device
*  is not configured
*
*******************************************************************************/
uint8 myUSB_GetConfiguration(void) 
{
    return (myUSB_configuration);
}


/*******************************************************************************
* Function Name: myUSB_IsConfigurationChanged
****************************************************************************//**
*
*  This function returns the clear-on-read configuration state.  It is useful
*  when the host sends double SET_CONFIGURATION request with the same 
*  configuration number or changes alternate settings of the interface. 
*  After configuration has been changed the OUT endpoints must be enabled and IN 
*  endpoint must be loaded with data to start communication with the host.
*
* \return
*  None-zero value when new configuration has been changed, otherwise zero is
*  returned.
*
* \globalvars
*
*  \ref myUSB_configurationChanged - This variable is set to 1 after
*   a SET_CONFIGURATION request and cleared in this function.
*
*******************************************************************************/
uint8 myUSB_IsConfigurationChanged(void) 
{
    uint8 res = 0u;

    if (myUSB_configurationChanged != 0u)
    {
        res = myUSB_configurationChanged;
        myUSB_configurationChanged = 0u;
    }

    return (res);
}


/*******************************************************************************
* Function Name: myUSB_GetInterfaceSetting
****************************************************************************//**
*
*  This function gets the current alternate setting for the specified interface.
*  It is useful to identify which alternate settings are active in the specified 
*  interface.
*
*  \param
*  interfaceNumber interface number
*
* \return
*  Returns the current alternate setting for the specified interface.
*
*******************************************************************************/
uint8  myUSB_GetInterfaceSetting(uint8 interfaceNumber)
                                                    
{
    return (myUSB_interfaceSetting[interfaceNumber]);
}


/*******************************************************************************
* Function Name: myUSB_GetEPState
****************************************************************************//**
*
*  This function returns the state of the requested endpoint.
*
* \param epNumber Data endpoint number
*
* \return
*  Returns the current state of the specified USBFS endpoint. Symbolic names and
*  their associated values are given in the following table. Use these constants
*  whenever you write code to change the state of the endpoints, such as ISR
*  code, to handle data sent or received.
*
*  Return Value           | Description
*  -----------------------|-----------------------------------------------------
*  USBFS_NO_EVENT_PENDING |The endpoint is awaiting SIE action
*  USBFS_EVENT_PENDING    |The endpoint is awaiting CPU action
*  USBFS_NO_EVENT_ALLOWED |The endpoint is locked from access
*  USBFS_IN_BUFFER_FULL   |The IN endpoint is loaded and the mode is set to ACK IN
*  USBFS_IN_BUFFER_EMPTY  |An IN transaction occurred and more data can be loaded
*  USBFS_OUT_BUFFER_EMPTY |The OUT endpoint is set to ACK OUT and is waiting for data
*  USBFS_OUT_BUFFER_FULL  |An OUT transaction has occurred and data can be read
*
*******************************************************************************/
uint8 myUSB_GetEPState(uint8 epNumber) 
{
    return (myUSB_EP[epNumber].apiEpState);
}


/*******************************************************************************
* Function Name: myUSB_GetEPCount
****************************************************************************//**
*
*  This function supports Data Endpoints only(EP1-EP8).
*  Returns the transfer count for the requested endpoint.  The value from
*  the count registers includes 2 counts for the two byte checksum of the
*  packet.  This function subtracts the two counts.
*
*  \param epNumber Data Endpoint Number.
*                   Valid values are between 1 and 8.
*
* \return
*  Returns the current byte count from the specified endpoint or 0 for an
*  invalid endpoint.
*
*******************************************************************************/
uint16 myUSB_GetEPCount(uint8 epNumber) 
{
    uint16 cntr = 0u;

    if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
        /* Get 11-bits EP counter where epCnt0 - 3 bits MSB and epCnt1 - 8 bits LSB. */
        cntr  = ((uint16) myUSB_SIE_EP_BASE.sieEp[epNumber].epCnt0) & myUSB_EPX_CNT0_MASK;
        cntr  = ((uint16) (cntr << 8u)) | ((uint16) myUSB_SIE_EP_BASE.sieEp[epNumber].epCnt1);
        cntr -= myUSB_EPX_CNTX_CRC_COUNT;
    }

    return (cntr);
}


#if (myUSB_EP_MANAGEMENT_DMA)
#if (CY_PSOC4)
    /*******************************************************************************
    * Function Name: myUSB_InitEpDma
    ****************************************************************************//**
    *
    *  This function configures priority for all DMA channels utilized by the
    *  component. Also sets callbacks for DMA auto mode.
    *
    *******************************************************************************/
    static void myUSB_InitEpDma(void)
    {
    #if (myUSB_DMA1_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep1_dma_CHANNEL] = myUSB_ep1_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA1_ACTIVE) */

    #if (myUSB_DMA2_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep2_dma_CHANNEL] = myUSB_ep2_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA2_ACTIVE) */

    #if (myUSB_DMA3_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep3_dma_CHANNEL] = myUSB_ep3_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA3_ACTIVE) */

    #if (myUSB_DMA4_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep4_dma_CHANNEL] = myUSB_ep4_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA4_ACTIVE) */

    #if (myUSB_DMA5_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep5_dma_CHANNEL] = myUSB_ep5_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA5_ACTIVE) */

    #if (myUSB_DMA6_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep6_dma_CHANNEL] = myUSB_ep6_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA6_ACTIVE) */

    #if (myUSB_DMA7_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep7_dma_CHANNEL] = myUSB_ep7_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA7_ACTIVE) */

    #if (myUSB_DMA8_ACTIVE)
        CYDMA_CH_CTL_BASE.ctl[myUSB_ep8_dma_CHANNEL] = myUSB_ep8_dma_CHANNEL_CFG;
    #endif /* (myUSB_DMA8_ACTIVE) */

    #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
        /* Initialize DMA channel callbacks. */
        #if (myUSB_DMA1_ACTIVE)
            (void) myUSB_ep1_dma_SetInterruptCallback(&myUSB_EP1_DMA_DONE_ISR);
        #endif /* (myUSB_DMA1_ACTIVE) */

        #if (myUSB_DMA2_ACTIVE)
            (void) myUSB_ep2_dma_SetInterruptCallback(&myUSB_EP2_DMA_DONE_ISR);
        #endif /* (myUSB_DMA2_ACTIVE) */

        #if (myUSB_DMA3_ACTIVE)
            (void) myUSB_ep3_dma_SetInterruptCallback(&myUSB_EP3_DMA_DONE_ISR);
        #endif /* (myUSB_DMA3_ACTIVE) */

        #if (myUSB_DMA4_ACTIVE)
            (void) myUSB_ep4_dma_SetInterruptCallback(&myUSB_EP4_DMA_DONE_ISR);
        #endif /* (myUSB_DMA4_ACTIVE) */

        #if (myUSB_DMA5_ACTIVE)
            (void) myUSB_ep5_dma_SetInterruptCallback(&myUSB_EP5_DMA_DONE_ISR);
        #endif /* (myUSB_DMA5_ACTIVE) */

        #if (myUSB_DMA6_ACTIVE)
            (void) myUSB_ep6_dma_SetInterruptCallback(&myUSB_EP6_DMA_DONE_ISR);
        #endif /* (myUSB_DMA6_ACTIVE) */

        #if (myUSB_DMA7_ACTIVE)
            (void) myUSB_ep7_dma_SetInterruptCallback(&myUSB_EP7_DMA_DONE_ISR);
        #endif /* (myUSB_DMA7_ACTIVE) */

        #if (myUSB_DMA8_ACTIVE)
            (void) myUSB_ep8_dma_SetInterruptCallback(&myUSB_EP8_DMA_DONE_ISR);
        #endif /* (myUSB_DMA8_ACTIVE) */
    #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
    }
#else


    /***************************************************************************
    * Function Name: myUSB_InitEP_DMA
    ************************************************************************//**
    *
    *  This function allocates and initializes a DMA channel to be used by the
    *  myUSB_LoadInEP() or myUSB_ReadOutEP() APIs for data
    *  transfer. It is available when the Endpoint Memory Management parameter
    *  is set to DMA.
    *
    *  This function is automatically called from the USBFS_LoadInEP() and USBFS_ReadOutEP() APIs.
    *
    *  \param epNumber Contains the data endpoint number.
    *            Valid values are between 1 and 8.
    *  \param *pData Pointer to a data array that is related to the EP transfers.
    *
    * \reentrant No.
    *
    ***************************************************************************/
    void myUSB_InitEP_DMA(uint8 epNumber, const uint8 *pData)
                                                                    
    {
        uint16 src;
        uint16 dst;

    #if (CY_PSOC3)
        src = HI16(CYDEV_SRAM_BASE);
        dst = HI16(CYDEV_PERIPH_BASE);
        pData = pData;
    #else
        if ((myUSB_EP[epNumber].addr & myUSB_DIR_IN) != 0u)
        {
            /* IN endpoint: source is memory buffer. */
            src = HI16(pData);
            dst = HI16(CYDEV_PERIPH_BASE);
        }
        else
        {
            /* OUT endpoint: source is USB IP memory buffer. */
            src = HI16(CYDEV_PERIPH_BASE);
            dst = HI16(pData);
        }
    #endif  /* (CY_PSOC3) */

        switch(epNumber)
        {
    #if (myUSB_DMA1_ACTIVE)
        case myUSB_EP1:
            myUSB_DmaChan[epNumber] = myUSB_ep1_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                    myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA1_ACTIVE) */

    #if (myUSB_DMA2_ACTIVE)
        case myUSB_EP2:
            myUSB_DmaChan[epNumber] = myUSB_ep2_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                    myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA2_ACTIVE) */

    #if (myUSB_DMA3_ACTIVE)
            case myUSB_EP3:
                myUSB_DmaChan[epNumber] = myUSB_ep3_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                        myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA3_ACTIVE) */

    #if (myUSB_DMA4_ACTIVE)
            case myUSB_EP4:
                myUSB_DmaChan[epNumber] = myUSB_ep4_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                        myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA4_ACTIVE) */

    #if (myUSB_DMA5_ACTIVE)
            case myUSB_EP5:
                myUSB_DmaChan[epNumber] = myUSB_ep5_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                        myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA5_ACTIVE) */

    #if (myUSB_DMA6_ACTIVE)
        case myUSB_EP6:
            myUSB_DmaChan[epNumber] = myUSB_ep6_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                    myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA6_ACTIVE) */

    #if (myUSB_DMA7_ACTIVE)
        case myUSB_EP7:
                myUSB_DmaChan[epNumber] = myUSB_ep7_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                        myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA7_ACTIVE) */

    #if (myUSB_DMA8_ACTIVE)
        case myUSB_EP8:
                myUSB_DmaChan[epNumber] = myUSB_ep8_DmaInitialize(myUSB_DMA_BYTES_PER_BURST,
                                                                                        myUSB_DMA_REQUEST_PER_BURST, src, dst);
            break;
    #endif /* (myUSB_DMA8_ACTIVE) */

        default:
            /* Do nothing for endpoints other than 1-8. */
            break;
        }

        if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
        {
            myUSB_DmaTd[epNumber] = CyDmaTdAllocate();

        #if (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u))
            myUSB_DmaNextTd[epNumber] = CyDmaTdAllocate();
        #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u)) */
        }
    }
#endif /* (CY_PSOC4) */

    /***************************************************************************
    * Function Name: myUSB_Stop_DMA
    ************************************************************************//**
    *  
    *  This function stops DMA channel associated with endpoint. It is available 
    *  when the Endpoint Buffer Management parameter is set to DMA. Call this 
    *  function when endpoint direction is changed from IN to OUT or vice versa 
    *  to trigger DMA re-configuration when myUSB_LoadInEP() or 
    *  myUSB_ReadOutEP() functions are called the first time. 
    *  
    *  \param epNumber: The data endpoint number for which associated DMA 
    *  channel is stopped. The range of valid values is between 1 and 8. To stop 
    *  all DMAs associated with endpoints call this function with 
    *  myUSB_MAX_EP argument.
    *
    * \reentrant
    *  No.
    *
    ***************************************************************************/
    void myUSB_Stop_DMA(uint8 epNumber) 
    {
        uint8 i;

        i = (epNumber < myUSB_MAX_EP) ? epNumber : myUSB_EP1;

        do
        {
        #if (CY_PSOC4)
            if (0u != myUSB_DmaChan[i])
            {
                myUSB_CyDmaChDisable(myUSB_DmaChan[i]);
            }
        #else
            if(myUSB_DmaTd[i] != DMA_INVALID_TD)
            {
                (void) CyDmaChDisable(myUSB_DmaChan[i]);
                CyDmaTdFree(myUSB_DmaTd[i]);
                myUSB_DmaTd[i] = DMA_INVALID_TD;
            }

            #if (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u))
                if(myUSB_DmaNextTd[i] != DMA_INVALID_TD)
                {
                    CyDmaTdFree(myUSB_DmaNextTd[i]);
                    myUSB_DmaNextTd[i] = DMA_INVALID_TD;
                }
            #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u)) */

        #endif /* (CY_PSOC4) */
            i++;
        }
        while ((i < myUSB_MAX_EP) && (epNumber == myUSB_MAX_EP));
    }
#endif /* (myUSB_EP_MANAGEMENT_DMA) */


#if (CY_PSOC3 || CY_PSOC5)
#if (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u))
    /***************************************************************************
    * Function Name: myUSB_LoadNextInEP
    ************************************************************************//**
    *
    * Summary:
    *  This internal function is used for IN endpoint DMA reconfiguration in
    *  Auto DMA mode.
    *
    * Parameters:
    *  epNumber: Contains the data endpoint number.
    *  mode:   0 - Configure DMA to send the the rest of data.
    *          1 - Configure DMA to repeat 2 last bytes of the first burst.
    *
    ***************************************************************************/
    void myUSB_LoadNextInEP(uint8 epNumber, uint8 mode) 
    {
        reg16 *convert;

        if (mode == 0u)
        {
            /* Configure DMA to send rest of data. */
            /* CyDmaTdSetConfiguration API is optimized to change transfer length only and configure TD. */
            convert = (reg16 *) &CY_DMA_TDMEM_STRUCT_PTR[myUSB_DmaTd[epNumber]].TD0[0u];
            /* Set transfer length. */
            CY_SET_REG16(convert, myUSB_inLength[epNumber] - myUSB_DMA_BYTES_PER_BURST);
            /* CyDmaTdSetAddress API is optimized to change source address only. */
            convert = (reg16 *) &CY_DMA_TDMEM_STRUCT_PTR[myUSB_DmaTd[epNumber]].TD1[0u];
            CY_SET_REG16(convert, LO16((uint32)myUSB_inDataPointer[epNumber] +
                                            myUSB_DMA_BYTES_PER_BURST));
            myUSB_inBufFull[epNumber] = 1u;
        }
        else
        {
            /* Configure DMA to repeat 2 last bytes of the first burst. */
            /* CyDmaTdSetConfiguration API is optimized to change transfer length only and configure TD. */
            convert = (reg16 *) &CY_DMA_TDMEM_STRUCT_PTR[myUSB_DmaTd[epNumber]].TD0[0u];
            /* Set transfer length. */
            CY_SET_REG16(convert, myUSB_DMA_BYTES_REPEAT);
            /* CyDmaTdSetAddress API is optimized to change source address only. */
            convert = (reg16 *) &CY_DMA_TDMEM_STRUCT_PTR[myUSB_DmaTd[epNumber]].TD1[0u];
            CY_SET_REG16(convert,  LO16((uint32)myUSB_inDataPointer[epNumber] +
                                   (myUSB_DMA_BYTES_PER_BURST - myUSB_DMA_BYTES_REPEAT)));
        }

        /* CyDmaChSetInitialTd API is optimized to initialize TD. */
        CY_DMA_CH_STRUCT_PTR[myUSB_DmaChan[epNumber]].basic_status[1u] = myUSB_DmaTd[epNumber];
    }
#endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u)) */
#endif /* (CY_PSOC3 || CY_PSOC5) */


/*******************************************************************************
* Function Name: myUSB_LoadInEP
****************************************************************************//**
*
*  This function performs different functionality depending on the Component’s
*  configured Endpoint Buffer Management. This parameter is defined in
*  the Descriptor Root in Component Configure window.
*
*  *Manual (Static/Dynamic Allocation):*
*  This function loads and enables the specified USB data endpoint for an IN
*  data transfer.
*
*  *DMA with Manual Buffer Management:*
*  Configures DMA for a data transfer from system RAM to endpoint buffer.
*  Generates request for a transfer.
*
*  *DMA with Automatic Buffer Management:*
*  Configures DMA. This is required only once, so it is done only when parameter
*  pData is not NULL. When the pData pointer is NULL, the function skips this
*  task. Sets Data ready status: This generates the first DMA transfer and
*  prepares data in endpoint buffer.
*
*  \param epNumber Contains the data endpoint number.
*            Valid values are between 1 and 8.
*  \param *pData A pointer to a data array from which the data for the endpoint space
*          is loaded.
*  \param length The number of bytes to transfer from the array and then send as
*          a result of an IN request. Valid values are between 0 and 512
*          (1023 for DMA with Automatic Buffer Management mode). The value 512
*          is applicable if only one endpoint is used.
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_LoadInEP(uint8 epNumber, const uint8 pData[], uint16 length)
                                                                        
{
    if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
    #if (!myUSB_EP_MANAGEMENT_DMA_AUTO)
        /* Limit length to available buffer USB IP buffer size.*/
        if (length > (myUSB_EPX_DATA_BUF_MAX - myUSB_EP[epNumber].buffOffset))
        {
            length = myUSB_EPX_DATA_BUF_MAX - myUSB_EP[epNumber].buffOffset;
        }
    #endif /* (!myUSB_EP_MANAGEMENT_DMA_AUTO) */

        /* Set count and data toggle. */
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCnt0 = (uint8) HI8(length) | myUSB_EP[epNumber].epToggle;
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCnt1 = (uint8) LO8(length);

    #if (myUSB_EP_MANAGEMENT_MANUAL)
        if (NULL != pData)
        {
			/* Copy data using arbiter data register. */
            uint16 i;
            for (i = 0u; i < length; ++i)
            {
                myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr = pData[i];
            }
        }

        /* IN endpoint buffer is full - read to be read. */
        myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_PENDING;

        /* Arm IN endpoint. */
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_EP[epNumber].epMode;

    #else

        #if (CY_PSOC3 || CY_PSOC5LP)
            /* Initialize DMA if it was not initialized. */
            if (DMA_INVALID_TD == myUSB_DmaTd[epNumber])
            {
                myUSB_InitEP_DMA(epNumber, pData);
            }
        #endif /* (CY_PSOC3 || CY_PSOC5LP) */

        #if (myUSB_EP_MANAGEMENT_DMA_MANUAL)
            /* IN endpoint buffer will be fully loaded by DMA shortly. */
            myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_PENDING;

            if ((pData != NULL) && (length > 0u))
            {
            #if (CY_PSOC4)
                uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

                /* Disable DMA channel: start configuration. */
                myUSB_CyDmaChDisable(channelNum);

                /* Configure source and destination. */
                myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) pData);
                myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr);

                /* Configure DMA descriptor. */
                --length;
                myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG | length |
                                                        CYDMA_BYTE | CYDMA_ELEMENT_WORD | CYDMA_INC_SRC_ADDR | CYDMA_INVALIDATE | CYDMA_PREEMPTABLE);

                /* Validate descriptor to execute on following DMA request. */
                myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);

                /* Enable DMA channel: configuration complete. */
                myUSB_CyDmaChEnable(channelNum);
            #else
                /* Configure DMA to transfer data. */
                (void) CyDmaChDisable(myUSB_DmaChan[epNumber]);
                (void) CyDmaTdSetConfiguration(myUSB_DmaTd[epNumber], length, CY_DMA_DISABLE_TD, TD_TERMIN_EN | TD_INC_SRC_ADR);
                (void) CyDmaTdSetAddress(myUSB_DmaTd[epNumber], LO16((uint32) pData), LO16((uint32) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr));

                /* Enable DMA channel. */
                (void) CyDmaChSetInitialTd(myUSB_DmaChan[epNumber], myUSB_DmaTd[epNumber]);
                (void) CyDmaChEnable(myUSB_DmaChan[epNumber], 1u);
            #endif /* (CY_PSOC4) */

                /* Generate DMA request. */
                myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg |=  (uint8)  myUSB_ARB_EPX_CFG_DMA_REQ;
                myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg &=  (uint8) ~myUSB_ARB_EPX_CFG_DMA_REQ;

                /* IN endpoint will be armed in ARB_ISR(source: IN_BUF_FULL) after first DMA transfer has been completed. */
            }
            else
            {
                /* When zero-length packet: arm IN endpoint directly. */
                myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_EP[epNumber].epMode;
            }
        #endif /* (myUSB_EP_MANAGEMENT_DMA_MANUAL) */

        #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
            if (pData != NULL)
            {
            #if (CY_PSOC4)
                uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

                /* Store address of buffer. */
                myUSB_DmaEpBufferAddrBackup[epNumber] = (uint32) pData;

                /* Disable DMA channel: start configuration. */
                myUSB_CyDmaChDisable(channelNum);

                /* Set destination address. */
                myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr);
                myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR1, (void*) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr);

                /* Configure DMA descriptor. */
                myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG  |
                                                        CYDMA_BYTE | CYDMA_ELEMENT_WORD | CYDMA_INC_SRC_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

                /* Configure DMA descriptor. */
                myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR1, myUSB_DMA_COMMON_CFG  |
                                                        CYDMA_BYTE | CYDMA_ELEMENT_WORD | CYDMA_INC_SRC_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

                /* Enable interrupt from DMA channel. */
                myUSB_CyDmaSetInterruptMask(channelNum);


                /* Enable DMA channel: configuration complete. */
                myUSB_CyDmaChEnable(channelNum);

            #else
                (void) CyDmaChDisable(myUSB_DmaChan[epNumber]);

                #if (myUSB_EP_DMA_AUTO_OPT == 0u)
                    myUSB_inLength[epNumber] = length;
                    myUSB_inDataPointer[epNumber] = pData;

                    /* Configure DMA to send data only for first burst */
                    (void) CyDmaTdSetConfiguration(myUSB_DmaTd[epNumber],
                        (length > myUSB_DMA_BYTES_PER_BURST) ? myUSB_DMA_BYTES_PER_BURST : length,
                        myUSB_DmaNextTd[epNumber], TD_TERMIN_EN | TD_INC_SRC_ADR);
                    (void) CyDmaTdSetAddress(myUSB_DmaTd[epNumber], LO16((uint32) pData),
                                                                               LO16((uint32) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr));

                    /* The second TD will be executed only when the first one fails.
                    *  The intention of this TD is to generate NRQ interrupt
                    *  and repeat 2 last bytes of the first burst.
                    */
                    (void) CyDmaTdSetConfiguration(myUSB_DmaNextTd[epNumber], 1u,
                                                   myUSB_DmaNextTd[epNumber],
                                                   myUSB_epX_TD_TERMOUT_EN[epNumber]);

                    /* Configure DmaNextTd to clear Data Ready status. */
                    (void) CyDmaTdSetAddress(myUSB_DmaNextTd[epNumber], LO16((uint32) &clearInDataRdyStatus),
                                                                                   LO16((uint32) &myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg));
                #else
                    /* Configure DMA to send all data. */
                    (void) CyDmaTdSetConfiguration(myUSB_DmaTd[epNumber], length,
                                                   myUSB_DmaTd[epNumber], TD_TERMIN_EN | TD_INC_SRC_ADR);
                    (void) CyDmaTdSetAddress(myUSB_DmaTd[epNumber], LO16((uint32) pData),
                                                                               LO16((uint32) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr));
                #endif /* (myUSB_EP_DMA_AUTO_OPT == 0u) */

                /* Clear any potential pending DMA requests before starting DMA channel to transfer data. */
                (void) CyDmaClearPendingDrq(myUSB_DmaChan[epNumber]);
                /* Enable DMA. */
                (void) CyDmaChSetInitialTd(myUSB_DmaChan[epNumber], myUSB_DmaTd[epNumber]);
                (void) CyDmaChEnable(myUSB_DmaChan[epNumber], 1u);
            #endif /* (CY_PSOC4) */
            }
            else
            {
                /* IN endpoint buffer (32 bytes) will shortly be preloaded by DMA. */
                myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_PENDING;

                if (length > 0u)
                {
                #if (CY_PSOC4)
                    uint32 lengthDescr0, lengthDescr1;
                    uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

                    /* Get number of full bursts. */
                    myUSB_DmaEpBurstCnt[epNumber] = (uint8) (length / myUSB_DMA_BYTES_PER_BURST);

                    /* Get number of elements in the last burst. */
                    myUSB_DmaEpLastBurstEl[epNumber] = (uint8) (length % myUSB_DMA_BYTES_PER_BURST);

                    /* Get total number of bursts. */
                    myUSB_DmaEpBurstCnt[epNumber] += (0u != myUSB_DmaEpLastBurstEl[epNumber]) ? 1u : 0u;

                    /* Adjust number of data elements transferred in last burst. */
                    myUSB_DmaEpLastBurstEl[epNumber] = (0u != myUSB_DmaEpLastBurstEl[epNumber]) ?
                                                                          (myUSB_DmaEpLastBurstEl[epNumber] - 1u) :
                                                                          (myUSB_DMA_BYTES_PER_BURST - 1u);

                    /* Get number of data elements to transfer for descriptor 0 and 1. */
                    lengthDescr0 = (1u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_BYTES_PER_BURST - 1u);
                    lengthDescr1 = (2u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_BYTES_PER_BURST - 1u);


                    /* Mark which descriptor is last one to execute. */
                    myUSB_DmaEpLastBurstEl[epNumber] |= (0u != (myUSB_DmaEpBurstCnt[epNumber] & 0x1u)) ?
                                                                            myUSB_DMA_DESCR0_MASK : myUSB_DMA_DESCR1_MASK;

                    /* Restore DMA settings for current transfer. */
                    myUSB_CyDmaChDisable(channelNum);

                    /* Restore destination address for input endpoint. */
                    myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) ((uint32) myUSB_DmaEpBufferAddrBackup[epNumber]));
                    myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR1, (void*) ((uint32) myUSB_DmaEpBufferAddrBackup[epNumber] +
                                                                                                                   myUSB_DMA_BYTES_PER_BURST));

                    /* Set number of elements to transfer. */
                    myUSB_CyDmaSetNumDataElements(channelNum, myUSB_DMA_DESCR0, lengthDescr0);
                    myUSB_CyDmaSetNumDataElements(channelNum, myUSB_DMA_DESCR1, lengthDescr1);

                    /* Validate descriptor 0 and command to start with it. */
                    myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);
                    myUSB_CyDmaSetDescriptor0Next(channelNum);

                    /* Validate descriptor 1. */
                    if (myUSB_DmaEpBurstCnt[epNumber] > 1u)
                    {
                        myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR1); 
                    }                   

                    /* Adjust burst counter taking to account: 2 valid descriptors and interrupt trigger after valid descriptor were executed. */
                    myUSB_DmaEpBurstCnt[epNumber] = myUSB_DMA_GET_BURST_CNT(myUSB_DmaEpBurstCnt[epNumber]);

                    /* Enable DMA channel: configuration complete. */
                    myUSB_CyDmaChEnable(channelNum);

                #elif (myUSB_EP_DMA_AUTO_OPT == 0u)
                    myUSB_inLength[epNumber]  = length;
                    myUSB_inBufFull[epNumber] = 0u;

                    (void) CyDmaChDisable(myUSB_DmaChan[epNumber]);
                    /* Configure DMA to send data only for first burst. */
                    (void) CyDmaTdSetConfiguration(
                        myUSB_DmaTd[epNumber], (length > myUSB_DMA_BYTES_PER_BURST) ?
                        myUSB_DMA_BYTES_PER_BURST : length,
                        myUSB_DmaNextTd[epNumber], TD_TERMIN_EN | TD_INC_SRC_ADR );
                    (void) CyDmaTdSetAddress(myUSB_DmaTd[epNumber], LO16((uint32)  myUSB_inDataPointer[epNumber]),
                                                                               LO16((uint32) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr));
                    /* Clear Any potential pending DMA requests before starting DMA channel to transfer data. */
                    (void) CyDmaClearPendingDrq(myUSB_DmaChan[epNumber]);
                    /* Enable DMA. */
                    (void) CyDmaChSetInitialTd(myUSB_DmaChan[epNumber], myUSB_DmaTd[epNumber]);
                    (void) CyDmaChEnable(myUSB_DmaChan[epNumber], 1u);
                #endif /* (CY_PSOC4) */

                #if !defined (myUSB_MANUAL_IN_EP_ARM)
                    /* Set IN data ready to generate DMA request to load data into endpoint buffer. */
                    myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg |= myUSB_ARB_EPX_CFG_IN_DATA_RDY;
                #endif  /* (myUSB_MANUAL_IN_EP_ARM) */

                    /* IN endpoint will be armed in ARB_ISR(source: IN_BUF_FULL) after first DMA transfer has been completed. */
                }
                else
                {
                    /* When zero-length packet: arm IN endpoint directly. */
                    myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_EP[epNumber].epMode;
                }
            }
        #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
    #endif /* (myUSB_EP_MANAGEMENT_MANUAL) */
    }
}


/*******************************************************************************
* Function Name: myUSB_ReadOutEP
****************************************************************************//**
*
*   This function performs different functionality depending on the Component’s
*   configured Endpoint Buffer Management. This parameter is defined in the
*   Descriptor Root in Component Configure window.
*
*   *Manual (Static/Dynamic Allocation):*
*   This function moves the specified number of bytes from endpoint buffer to
*   system RAM. The number of bytes actually transferred from endpoint buffer to
*   system RAM is the lesser of the actual number of bytes sent by the host or
*   the number of bytes requested by the length parameter.
*
*   *DMA with Manual Buffer Management:*
*   Configure DMA to transfer data from endpoint buffer to system RAM. Generate
*   a DMA request. The firmware must wait until the DMA completes the data
*   transfer after calling the myUSB_ReadOutEP() API. For example,
*   by checking EPstate:
*
*   \snippet /USBFS_sut_02.cydsn/main.c checking EPstatey
*
*   The USBFS_EnableOutEP() has to be called to allow host to write data into
*   the endpoint buffer after DMA has completed transfer data from OUT endpoint
*   buffer to SRAM.
*
*   *DMA with Automatic Buffer Management:*
*   Configure DMA. This is required only once and automatically generates DMA
*   requests as data arrives
*
*  \param epNumber: Contains the data endpoint number.
*            Valid values are between 1 and 8.
*  \param pData: A pointer to a data array from which the data for the endpoint
*         space is loaded.
*  \param length: The number of bytes to transfer from the USB Out endpoint and
*          loads it into data array. Valid values are between 0 and 1023. The
*          function moves fewer than the requested number of bytes if the host
*          sends fewer bytes than requested.
*
* \return
*  Number of bytes received, 0 for an invalid endpoint.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint16 myUSB_ReadOutEP(uint8 epNumber, uint8 pData[], uint16 length)
                                                                        
{
    if ((pData != NULL) && (epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
    #if (!myUSB_EP_MANAGEMENT_DMA_AUTO)
        /* Adjust requested length to available data. */
        length = (length > myUSB_GetEPCount(epNumber)) ? myUSB_GetEPCount(epNumber) : length;
    #endif /* (!myUSB_EP_MANAGEMENT_DMA_AUTO) */

    #if (myUSB_EP_MANAGEMENT_MANUAL)
        {
            /* Copy data using arbiter data register. */
            uint16 i;
            for (i = 0u; i < length; ++i)
            {
                pData[i] = (uint8) myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr;
            }
        }

        /* Arm OUT endpoint after data has been copied from endpoint buffer. */
        myUSB_EnableOutEP(epNumber);
    #else

        #if (CY_PSOC3 || CY_PSOC5LP)
            /* Initialize DMA if it was not initialized. */
            if (DMA_INVALID_TD == myUSB_DmaTd[epNumber])
            {
                myUSB_InitEP_DMA(epNumber, pData);
            }
        #endif /* (CY_PSOC3 || CY_PSOC5LP) */

        #if (myUSB_EP_MANAGEMENT_DMA_MANUAL)
        #if (CY_PSOC4)
        {
            uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

            /* Disable DMA channel: start configuration. */
            myUSB_CyDmaChDisable(channelNum);

            /* Configure source and destination. */
            myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr);
            myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) pData);

            /* Configure DMA descriptor. */
            myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG | (uint16)(length - 1u) |
                                                    CYDMA_BYTE | CYDMA_WORD_ELEMENT | CYDMA_INC_DST_ADDR | CYDMA_INVALIDATE | CYDMA_PREEMPTABLE);

            /* Validate descriptor to execute on following DMA request. */
            myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);

            /* Enable DMA channel: configuration complete. */
            myUSB_CyDmaChEnable(channelNum);
        }
        #else
            /* Configure DMA to transfer data. */
            (void) CyDmaChDisable(myUSB_DmaChan[epNumber]);
            (void) CyDmaTdSetConfiguration(myUSB_DmaTd[epNumber], length, CY_DMA_DISABLE_TD, TD_TERMIN_EN | TD_INC_DST_ADR);
            (void) CyDmaTdSetAddress(myUSB_DmaTd[epNumber], LO16((uint32) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr), LO16((uint32)pData));

            /* Enable DMA channel. */
            (void) CyDmaChSetInitialTd(myUSB_DmaChan[epNumber], myUSB_DmaTd[epNumber]);
            (void) CyDmaChEnable(myUSB_DmaChan[epNumber], 1u);
        #endif /* (CY_PSOC4) */

            /* Generate DMA request. */
            myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg |=  (uint8)  myUSB_ARB_EPX_CFG_DMA_REQ;
            myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg &=  (uint8) ~myUSB_ARB_EPX_CFG_DMA_REQ;

            /* OUT endpoint has to be armed again by user when DMA transfers have been completed.
            * NO_EVENT_PENDING: notifies that data has been copied from endpoint buffer.
            */

        #endif /* (myUSB_EP_MANAGEMENT_DMA_MANUAL) */

        #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
        #if (CY_PSOC4)
        {
            uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];
            uint32 lengthDescr0, lengthDescr1;

            /* Get number of full bursts. */
            myUSB_DmaEpBurstCnt[epNumber] = (uint8) (length / myUSB_DMA_BYTES_PER_BURST);

            /* Get number of elements in the last burst. */
            myUSB_DmaEpLastBurstEl[epNumber] = (uint8) (length % myUSB_DMA_BYTES_PER_BURST);

            /* Get total number of bursts. */
            myUSB_DmaEpBurstCnt[epNumber] += (0u != myUSB_DmaEpLastBurstEl[epNumber]) ? 1u : 0u;

            /* Adjust number of the data elements transfered in last burst. */
            myUSB_DmaEpLastBurstEl[epNumber] = (0u != myUSB_DmaEpLastBurstEl[epNumber]) ?
                                                                  (myUSB_DmaEpLastBurstEl[epNumber] - 1u) :
                                                                  (myUSB_DMA_BYTES_PER_BURST - 1u);

            /* Get number of data elements to transfer for descriptor 0 and 1. */
            lengthDescr0 = (1u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_BYTES_PER_BURST - 1u);
            lengthDescr1 = (2u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_BYTES_PER_BURST - 1u);

            /* Mark if revert number of data elements in descriptor after transfer completion. */
            myUSB_DmaEpLastBurstEl[epNumber] |= (myUSB_DmaEpBurstCnt[epNumber] > 2u) ? myUSB_DMA_DESCR_REVERT : 0u;

            /* Mark last descriptor to be executed. */
            myUSB_DmaEpLastBurstEl[epNumber] |= (0u != (myUSB_DmaEpBurstCnt[epNumber] & 0x1u)) ?
                                                                    myUSB_DMA_DESCR0_MASK : myUSB_DMA_DESCR1_MASK;

            /* Store address of buffer and burst counter for endpoint. */
            myUSB_DmaEpBufferAddrBackup[epNumber] = (uint32) pData;
            myUSB_DmaEpBurstCntBackup[epNumber]   = myUSB_DmaEpBurstCnt[epNumber];

            /* Adjust burst counter taking to account: 2 valid descriptors and interrupt trigger after valid descriptor were executed. */
            myUSB_DmaEpBurstCnt[epNumber] = myUSB_DMA_GET_BURST_CNT(myUSB_DmaEpBurstCnt[epNumber]);

            /* Disable DMA channel: start configuration. */
            myUSB_CyDmaChDisable(channelNum);

            /* Set destination address. */
            myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr);
            myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR1, (void*) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr);

            myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) ((uint32) pData));
            myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR1, (void*) ((uint32) pData + myUSB_DMA_BYTES_PER_BURST));

            /* Configure DMA descriptor. */
            myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG  | lengthDescr0 |
                                                    CYDMA_BYTE | CYDMA_WORD_ELEMENT | CYDMA_INC_DST_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

            /* Configure DMA descriptor. */
            myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR1, myUSB_DMA_COMMON_CFG  | lengthDescr1 |
                                                    CYDMA_BYTE | CYDMA_WORD_ELEMENT | CYDMA_INC_DST_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

            /* Enable interrupt from DMA channel. */
            myUSB_CyDmaSetInterruptMask(channelNum);

            /* Validate DMA descriptor 0 and 1. */
            myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);

            if (myUSB_DmaEpBurstCntBackup[epNumber] > 1u)
            {
                myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR1);
            }

            /* Enable DMA channel: configuration complete. */
            myUSB_CyDmaChEnable(channelNum);
        }
        #else
            (void) CyDmaChDisable(myUSB_DmaChan[epNumber]);
            (void) CyDmaTdSetConfiguration(myUSB_DmaTd[epNumber], length,  myUSB_DmaTd[epNumber], TD_TERMIN_EN | TD_INC_DST_ADR);
            (void) CyDmaTdSetAddress(myUSB_DmaTd[epNumber], LO16((uint32) &myUSB_ARB_EP_BASE.arbEp[epNumber].rwDr), LO16((uint32) pData));

            /* Clear Any potential pending DMA requests before starting DMA channel to transfer data. */
            (void) CyDmaClearPendingDrq(myUSB_DmaChan[epNumber]);

            /* Enable DMA channel. */
            (void) CyDmaChSetInitialTd(myUSB_DmaChan[epNumber], myUSB_DmaTd[epNumber]);
            (void) CyDmaChEnable(myUSB_DmaChan[epNumber], 1u);
        #endif /* (CY_PSOC4) */

            /* OUT endpoint has to be armed again by user when DMA transfers have been completed.
            * NO_EVENT_PENDING: notifies that data has been copied from endpoint buffer.
            */

        #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
    #endif /* (myUSB_EP_MANAGEMENT_MANUAL) */
    }
    else
    {
        length = 0u;
    }

    return (length);
}


#if (myUSB_16BITS_EP_ACCESS_ENABLE)
/*******************************************************************************
* Function Name: myUSB_LoadInEP16
****************************************************************************//**
*
*  This function performs different functionality depending on the Component’s
*  configured Endpoint Buffer Management. This parameter is defined in
*  the Descriptor Root in Component Configure window.
*
*  *Manual (Static/Dynamic Allocation):*
*  This function loads and enables the specified USB data endpoint for an IN
*  data transfer.
*
*  *DMA with Manual Buffer Management:*
*  Configures DMA for a data transfer from system RAM to endpoint buffer.
*  Generates request for a transfer.
*
*  *DMA with Automatic Buffer Management:*
*  Configures DMA. This is required only once, so it is done only when parameter
*  pData is not NULL. When the pData pointer is NULL, the function skips this
*  task. Sets Data ready status: This generates the first DMA transfer and
*  prepares data in endpoint buffer.
*
*  \param epNumber Contains the data endpoint number.
*        Valid values are between 1 and 8.
*  \param *pData A pointer to a data array from which the data for the endpoint
*        space is loaded. It shall be ensured that this pointer address is even
*        to ensure the 16-bit transfer is aligned to even address. Else, a hard
*        fault condition can occur.
*  \param length The number of bytes to transfer from the array and then send as
*        a result of an IN request. Valid values are between 0 and 512 (1023 for
*        DMA with Automatic Buffer Management mode). The value 512 is applicable
*        if only one endpoint is used.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_LoadInEP16(uint8 epNumber, const uint8 pData[], uint16 length)
{
    /* Check array alignment on half-word boundary. */
    CYASSERT(0u == (((uint32) pData) & 0x01u));

    if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
    #if (!myUSB_EP_MANAGEMENT_DMA_AUTO)
        /* Limit length to available buffer USB IP buffer size. */
        if (length > (myUSB_EPX_DATA_BUF_MAX - myUSB_EP[epNumber].buffOffset))
        {
            length = myUSB_EPX_DATA_BUF_MAX - myUSB_EP[epNumber].buffOffset;
        }
    #endif /* (!myUSB_EP_MANAGEMENT_DMA_AUTO) */

        /* Set count and data toggle. */
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCnt0 = (uint32) HI8(length) | myUSB_EP[epNumber].epToggle;
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCnt1 = (uint32) LO8(length);

        /* Adjust requested length: 2 bytes are handled at one data register access. */
        length += (length & 0x01u);

    #if (myUSB_EP_MANAGEMENT_MANUAL)
        if (NULL != pData)
        {
            /* Convert uint8 array to uint16. */
            const uint16 *dataBuf = (uint16 *) pData;

            /* Copy data using 16-bits arbiter data register. */
            uint16 i;
            for (i = 0u; i < (length >> 1u); ++i)
            {
                myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16 = dataBuf[i];
            }
        }

        /* IN endpoint buffer is full - read to be read. */
        myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_PENDING;

        /* Arm IN endpoint. */
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_EP[epNumber].epMode;

    #else

        #if (myUSB_EP_MANAGEMENT_DMA_MANUAL)
            /* IN endpoint buffer will be fully loaded by DMA shortly. */
            myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_PENDING;

            if ((pData != NULL) && (length > 0u))
            {
                uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

                /* Disable DMA channel: start configuration. */
                myUSB_CyDmaChDisable(channelNum);

                /* Configure source and destination. */
                myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) pData);
                myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16);

                /* Configure DMA descriptor. */
                length = (length >> 1u) - 1u;
                myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG | length |
                                                        CYDMA_HALFWORD | CYDMA_ELEMENT_WORD | CYDMA_INC_SRC_ADDR | CYDMA_INVALIDATE | CYDMA_PREEMPTABLE);

                /* Validate descriptor to execute on following DMA request. */
                myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);

                /* Enable DMA channel: configuration complete. */
                myUSB_CyDmaChEnable(channelNum);

                /* Generate DMA request. */
                myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg |=  (uint32)  myUSB_ARB_EPX_CFG_DMA_REQ;
                myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg &=  (uint32) ~myUSB_ARB_EPX_CFG_DMA_REQ;

                /* IN endpoint will be armed in ARB_ISR(source: IN_BUF_FULL) after first DMA transfer has been completed. */
            }
            else
            {
                /* When zero-length packet: arm IN endpoint directly. */
                myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_EP[epNumber].epMode;
            }
        #endif /* (myUSB_EP_MANAGEMENT_DMA_MANUAL) */

        #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
            if (pData != NULL)
            {
                uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

                /* Store address of buffer. */
                myUSB_DmaEpBufferAddrBackup[epNumber] = (uint32) pData;

                /* Disable DMA channel: start configuration. */
                myUSB_CyDmaChDisable(channelNum);

                /* Set destination address. */
                myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16);
                myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR1, (void*) &myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16);

                /* Configure DMA descriptor. */
                myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG  |
                                                        CYDMA_HALFWORD | CYDMA_ELEMENT_WORD | CYDMA_INC_SRC_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

                /* Configure DMA descriptor. */
                myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR1, myUSB_DMA_COMMON_CFG  |
                                                        CYDMA_HALFWORD | CYDMA_ELEMENT_WORD | CYDMA_INC_SRC_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

                /* Enable interrupt from DMA channel. */
                myUSB_CyDmaSetInterruptMask(channelNum);

                /* Enable DMA channel: configuration complete. */
                myUSB_CyDmaChEnable(channelNum);
            }
            else
            {
                /* IN endpoint buffer (32 bytes) will shortly be preloaded by DMA. */
                myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_PENDING;

                if (length > 0u)
                {
                    uint32 lengthDescr0, lengthDescr1;
                    uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

                    /* Get number of full bursts. */
                    myUSB_DmaEpBurstCnt[epNumber] = (uint8) (length / myUSB_DMA_BYTES_PER_BURST);

                    /* Get number of elements in the last burst. */
                    myUSB_DmaEpLastBurstEl[epNumber] = (uint8) (length % myUSB_DMA_BYTES_PER_BURST);

                    /* Get total number of bursts. */
                    myUSB_DmaEpBurstCnt[epNumber] += (0u != myUSB_DmaEpLastBurstEl[epNumber]) ? 1u : 0u;

                    /* Adjust number of data elements transferred in last burst. */
                    myUSB_DmaEpLastBurstEl[epNumber] = (0u != myUSB_DmaEpLastBurstEl[epNumber]) ?
                                                                          ((myUSB_DmaEpLastBurstEl[epNumber] >> 1u) - 1u) :
                                                                           (myUSB_DMA_HALFWORDS_PER_BURST - 1u);

                    /* Get number of data elements to transfer for descriptor 0 and 1. */
                    lengthDescr0 = (1u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_HALFWORDS_PER_BURST - 1u);
                    lengthDescr1 = (2u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_HALFWORDS_PER_BURST - 1u);

                    /* Mark which descriptor is last one to execute. */
                    myUSB_DmaEpLastBurstEl[epNumber] |= (0u != (myUSB_DmaEpBurstCnt[epNumber] & 0x1u)) ?
                                                                            myUSB_DMA_DESCR0_MASK : myUSB_DMA_DESCR1_MASK;

                    /* Restore DMA settings for current transfer. */
                    myUSB_CyDmaChDisable(channelNum);

                    /* Restore destination address for input endpoint. */
                    myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) ((uint32) myUSB_DmaEpBufferAddrBackup[epNumber]));
                    myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR1, (void*) ((uint32) myUSB_DmaEpBufferAddrBackup[epNumber] +
                                                                                                                   myUSB_DMA_BYTES_PER_BURST));

                    /* Set number of elements to transfer. */
                    myUSB_CyDmaSetNumDataElements(channelNum, myUSB_DMA_DESCR0, lengthDescr0);
                    myUSB_CyDmaSetNumDataElements(channelNum, myUSB_DMA_DESCR1, lengthDescr1);

                    /* Validate descriptor 0 and command to start with it. */
                    myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);
                    myUSB_CyDmaSetDescriptor0Next(channelNum);

                    /* Validate descriptor 1. */
                    if (myUSB_DmaEpBurstCnt[epNumber] > 1u)
                    {
                        myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR1);
                    }

                    /* Adjust burst counter taking to account: 2 valid descriptors and interrupt trigger after valid descriptor were executed. */
                    myUSB_DmaEpBurstCnt[epNumber] = myUSB_DMA_GET_BURST_CNT(myUSB_DmaEpBurstCnt[epNumber]);

                    /* Enable DMA channel: configuration complete. */
                    myUSB_CyDmaChEnable(channelNum);

                #if !defined (myUSB_MANUAL_IN_EP_ARM)
                    /* Set IN data ready to generate DMA request to load data into endpoint buffer. */
                    myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg |= myUSB_ARB_EPX_CFG_IN_DATA_RDY;
                #endif  /* (myUSB_MANUAL_IN_EP_ARM) */

                    /* IN endpoint will be armed in ARB_ISR(source: IN_BUF_FULL) after first DMA transfer has been completed. */
                }
                else
                {
                    /* When zero-length packet: arm IN endpoint directly. */
                    myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_EP[epNumber].epMode;
                }
            }
        #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
    #endif /* (myUSB_EP_MANAGEMENT_MANUAL) */
    }
}


/*******************************************************************************
* Function Name: myUSB_ReadOutEP16
****************************************************************************//**
*
*   This function performs different functionality depending on the Component’s
*   configured Endpoint Buffer Management. This parameter is defined in the
*   Descriptor Root in Component Configure window.
*
*   *Manual (Static/Dynamic Allocation):*
*   This function moves the specified number of bytes from endpoint buffer to
*   system RAM. The number of bytes actually transferred from endpoint buffer to
*   system RAM is the lesser of the actual number of bytes sent by the host or
*   the number of bytes requested by the length parameter.
*
*   *DMA with Manual Buffer Management:*
*   Configure DMA to transfer data from endpoint buffer to system RAM. Generate
*   a DMA request. The firmware must wait until the DMA completes the data
*   transfer after calling the myUSB_ReadOutEP() API. For example,
*   by checking EPstate:
*
*   \snippet /USBFS_sut_02.cydsn/main.c checking EPstatey
*
*   The USBFS_EnableOutEP() has to be called to allow host to write data into
*   the endpoint buffer after DMA has completed transfer data from OUT endpoint
*   buffer to SRAM.
*
*   *DMA with Automatic Buffer Management:*
*   Configure DMA. This is required only once and automatically generates DMA
*   requests as data arrives
*
*  \param epNumber: Contains the data endpoint number.
*         Valid values are between 1 and 8.
*  \param pData: A pointer to a data array into which the data for the endpoint
*         space is copied. It shall be ensured that this pointer address is
*         even to ensure the 16-bit transfer is aligned to even address. Else,
*         a hard fault condition can occur.
*  \param length: The number of bytes to transfer from the USB Out endpoint and
*         loads it into data array. Valid values are between 0 and 1023. The
*         function moves fewer than the requested number of bytes if the host
*         sends fewer bytes than requested.
*
* \return
*  Number of bytes received, 0 for an invalid endpoint.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint16 myUSB_ReadOutEP16(uint8 epNumber, uint8 pData[], uint16 length)
{
    uint32 adjLength;

    /* Check array alignment on half-word boundary */
    CYASSERT(0u == (((uint32) pData) & 0x01u));

    if ((pData != NULL) && (epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
    #if (!myUSB_EP_MANAGEMENT_DMA_AUTO)
        /* Adjust requested length to available data. */
        length = (length > myUSB_GetEPCount(epNumber)) ? myUSB_GetEPCount(epNumber) : length;
    #endif /* (!myUSB_EP_MANAGEMENT_DMA_AUTO) */

    /* Adjust requested length: 2 bytes are handled at one data register access. */
    adjLength =  length + ((uint32)length & 1u);

    #if (myUSB_EP_MANAGEMENT_MANUAL)
        {
            /* Convert uint8 array to uint16. */
            uint16 *dataBuf = (uint16 *) pData;

            /* Copy data using 16-bits arbiter data register. */
            uint16 i;
            for (i = 0u; i < (adjLength >> 1u); ++i)
            {
                dataBuf[i] = (uint16) myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16;
            }
        }

        /* Arm OUT endpoint after data has been read from endpoint buffer. */
        myUSB_EnableOutEP(epNumber);
    #else

        #if (myUSB_EP_MANAGEMENT_DMA_MANUAL)
        {
            uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

            /* Disable DMA channel: start configuration. */
            myUSB_CyDmaChDisable(channelNum);

            /* Configure source and destination. */
            myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16);
            myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) pData);

            /* Configure DMA descriptor. */
            myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG | (uint16)((adjLength >> 1u) - 1u) |
                                                    CYDMA_HALFWORD | CYDMA_WORD_ELEMENT | CYDMA_INC_DST_ADDR | CYDMA_INVALIDATE | CYDMA_PREEMPTABLE);

            /* Validate descriptor to execute on following DMA request. */
            myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);

            /* Enable DMA channel: configuration complete. */
            myUSB_CyDmaChEnable(channelNum);

            /* Generate DMA request. */
            myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg |=  (uint32)  myUSB_ARB_EPX_CFG_DMA_REQ;
            myUSB_ARB_EP_BASE.arbEp[epNumber].epCfg &=  (uint32) ~myUSB_ARB_EPX_CFG_DMA_REQ;

            /* OUT endpoint has to be armed again by user when DMA transfers have been completed.
            * NO_EVENT_PENDING: notifies that data has been copied from endpoint buffer.
            */
        }
        #endif /* (myUSB_EP_MANAGEMENT_DMA_MANUAL) */

        #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
        {
            uint32 lengthDescr0, lengthDescr1;
            uint32 channelNum = (uint32) myUSB_DmaChan[epNumber];

            /* Get number of full bursts. */
            myUSB_DmaEpBurstCnt[epNumber] = (uint8) (adjLength / myUSB_DMA_BYTES_PER_BURST);

            /* Get number of elements in last burst. */
            myUSB_DmaEpLastBurstEl[epNumber] = (uint8) (adjLength % myUSB_DMA_BYTES_PER_BURST);

            /* Get total number of bursts. */
            myUSB_DmaEpBurstCnt[epNumber] += (0u != myUSB_DmaEpLastBurstEl[epNumber]) ? 1u : 0u;

            /* Adjust number of data elements transferred in last burst. */
            myUSB_DmaEpLastBurstEl[epNumber] = (0u != myUSB_DmaEpLastBurstEl[epNumber]) ?
                                                                  ((myUSB_DmaEpLastBurstEl[epNumber] >> 1u) - 1u) :
                                                                   (myUSB_DMA_HALFWORDS_PER_BURST - 1u);

            /* Get number of data elements to transfer for descriptor 0 and 1. */
            lengthDescr0 = (1u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_HALFWORDS_PER_BURST - 1u);
            lengthDescr1 = (2u == myUSB_DmaEpBurstCnt[epNumber]) ? myUSB_DmaEpLastBurstEl[epNumber] : (myUSB_DMA_HALFWORDS_PER_BURST - 1u);

            /* Mark last descriptor to be executed. */
            myUSB_DmaEpLastBurstEl[epNumber] |= (0u != (myUSB_DmaEpBurstCnt[epNumber] & 0x1u)) ?
                                                                    myUSB_DMA_DESCR0_MASK : myUSB_DMA_DESCR1_MASK;

            /* Mark if revert number of data elements in descriptor after transfer completion. */
            myUSB_DmaEpLastBurstEl[epNumber] |= (myUSB_DmaEpBurstCnt[epNumber] > 2u) ? myUSB_DMA_DESCR_REVERT : 0u;

            /* Mark that 16-bits access to data register is performed. */
            myUSB_DmaEpLastBurstEl[epNumber] |= myUSB_DMA_DESCR_16BITS;

            /* Store address of buffer and burst counter for endpoint. */
            myUSB_DmaEpBufferAddrBackup[epNumber] = (uint32) pData;
            myUSB_DmaEpBurstCntBackup[epNumber]   = myUSB_DmaEpBurstCnt[epNumber];

            /* Adjust burst counter taking to account: 2 valid descriptors and interrupt trigger after valid descriptor were executed. */
            myUSB_DmaEpBurstCnt[epNumber] = myUSB_DMA_GET_BURST_CNT(myUSB_DmaEpBurstCnt[epNumber]);

            /* Disable DMA channel: start configuration. */
            myUSB_CyDmaChDisable(channelNum);

            /* Set destination address. */
            myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR0, (void*) &myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16);
            myUSB_CyDmaSetSrcAddress(channelNum, myUSB_DMA_DESCR1, (void*) &myUSB_ARB_EP16_BASE.arbEp[epNumber].rwDr16);

            myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR0, (void*) ((uint32) pData));
            myUSB_CyDmaSetDstAddress(channelNum, myUSB_DMA_DESCR1, (void*) ((uint32) pData + myUSB_DMA_BYTES_PER_BURST));

            /* Configure DMA descriptor 0. */
            myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR0, myUSB_DMA_COMMON_CFG  | lengthDescr0 |
                                                    CYDMA_HALFWORD | CYDMA_WORD_ELEMENT | CYDMA_INC_DST_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

            /* Configure DMA descriptor 1. */
            myUSB_CyDmaSetConfiguration(channelNum, myUSB_DMA_DESCR1, myUSB_DMA_COMMON_CFG  | lengthDescr1 |
                                                    CYDMA_HALFWORD | CYDMA_WORD_ELEMENT | CYDMA_INC_DST_ADDR | CYDMA_INVALIDATE | CYDMA_CHAIN);

            /* Enable interrupt from DMA channel. */
            myUSB_CyDmaSetInterruptMask(channelNum);

            /* Validate DMA descriptor 0 and 1. */
            myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR0);
            
            if (myUSB_DmaEpBurstCntBackup[epNumber] > 1u)
            {
                myUSB_CyDmaValidateDescriptor(channelNum, myUSB_DMA_DESCR1);
            }

            /* Enable DMA channel: configuration complete. */
            myUSB_CyDmaChEnable(channelNum);

            /* OUT endpoint has to be armed again by user when DMA transfers have been completed.
            * NO_EVENT_PENDING: notifies that data has been copied from endpoint buffer.
            */
        }
        #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
    #endif /* (myUSB_EP_MANAGEMENT_MANUAL) */
    }
    else
    {
        length = 0u;
    }

    return (length);
}
#endif /* (myUSB_16BITS_EP_ACCESS_ENABLE) */


/*******************************************************************************
* Function Name: myUSB_EnableOutEP
****************************************************************************//**
*
*  This function enables the specified endpoint for OUT transfers. Do not call
*  this function for IN endpoints.
*
*  \param epNumber: Contains the data endpoint number. Valid values are between
*  1 and 8.
*
* \globalvars
*
*  \ref myUSB_EP[epNumber].apiEpState - set to NO_EVENT_PENDING
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_EnableOutEP(uint8 epNumber) 
{
    if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
        myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_PENDING;

        /* Enable OUT endpoint to be written by Host. */
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_EP[epNumber].epMode;
        
    }
}


/*******************************************************************************
* Function Name: myUSB_DisableOutEP
****************************************************************************//**
*
*  This function disables the specified USBFS OUT endpoint. Do not call this
*  function for IN endpoints.
*
*  \param epNumber: Contains the data endpoint number.
*            Valid values are between 1 and 8.
*
*******************************************************************************/
void myUSB_DisableOutEP(uint8 epNumber) 
{
    if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
        /* Set NAK response for OUT endpoint. */
        myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_MODE_NAK_OUT;
    }
}


/*******************************************************************************
* Function Name: myUSB_Force
****************************************************************************//**
*
*  This function forces a USB J, K, or SE0 state on the D+/D– lines. It provides
*  the necessary mechanism for a USB device application to perform a USB Remote
*  Wakeup. For more information, see the USB 2.0 Specification for details on
*  Suspend and Resume.
*
*  \param state A byte indicating which of the four bus states to enable.
*        Symbolic names  and their associated values are listed here:
*    State                      |Description
*    ---------------------------|----------------------------------------------
*    myUSB_FORCE_J   | Force a J State onto the D+/D– lines
*    myUSB_FORCE_K   | Force a K State onto the D+/D– lines
*    myUSB_FORCE_SE0 | Force a Single Ended 0 onto the D+/D– lines
*    myUSB_FORCE_NONE| Return bus to SIE control
*
*
*******************************************************************************/
void myUSB_Force(uint8 bState) 
{
    /* This registers is used only for manual control of SIE (no masking is
    * needed before write into it).
    */
    myUSB_USBIO_CR0_REG = bState;
}


/*******************************************************************************
* Function Name: myUSB_GetEPAckState
****************************************************************************//**
*
*  This function determines whether an ACK transaction occurred on this endpoint
*  by reading the ACK bit in the control register of the endpoint. It does not
*  clear the ACK bit.
*
*  \param epNumber Contains the data endpoint number.
*            Valid values are between 1 and 8.
*
*  \return
*  If an ACKed transaction occurred, this function returns a non-zero value.
*  Otherwise, it returns zero.
*
*******************************************************************************/
uint8 myUSB_GetEPAckState(uint8 epNumber) 
{
    uint8 cr = 0u;

    if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
        cr = myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 & myUSB_MODE_ACKD;
    }

    return ((uint8) cr);
}


/*******************************************************************************
* Function Name: myUSB_SetPowerStatus
****************************************************************************//**
*
*  This function sets the current power status. The device replies to USB
*  GET_STATUS requests based on this value. This allows the device to properly
*  report its status for USB Chapter 9 compliance. Devices can change their
*  power source from self powered to bus powered at any time and report their
*  current power source as part of the device status. You should call this
*  function any time your device changes from self powered to bus powered or
*  vice versa, and set the status appropriately.
*
*  \param powerStatus: Contains the desired power status, one for self powered
*        or zero for bus powered. Symbolic names and their associated values are
*        given here:
*  Power Status                                |Description
*  --------------------------------------------|---------------------------
*  myUSB_DEVICE_STATUS_BUS_POWERED  | Set the device to bus powered
*  myUSB_DEVICE_STATUS_SELF_POWERED | Set the device to self powered
*
* \globalvars
*
*  \ref myUSB_deviceStatus - set power status
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_SetPowerStatus(uint8 powerStatus) 
{
    if (powerStatus != myUSB_DEVICE_STATUS_BUS_POWERED)
    {
        myUSB_deviceStatus |= (uint8)  myUSB_DEVICE_STATUS_SELF_POWERED;
    }
    else
    {
        myUSB_deviceStatus &= (uint8) ~myUSB_DEVICE_STATUS_SELF_POWERED;
    }
}


#if (myUSB_VBUS_MONITORING_ENABLE)
    /***************************************************************************
    * Function Name: myUSB_VBusPresent
    ************************************************************************//**
    *
    *  Determines VBUS presence for self-powered devices. This function is
    *  available when the VBUS Monitoring option is enabled in the Advanced tab.
    *
    * \return
    *  The return value can be the following:
    *  Return Value | Description
    *  -------------|-----------------
    *  1            | VBUS is present
    *  0            | VBUS is absent
    *
    *
    ***************************************************************************/
    uint8 myUSB_VBusPresent(void) 
    {
        return ((0u != (myUSB_VBUS_STATUS_REG & myUSB_VBUS_VALID)) ? (uint8) 1u : (uint8) 0u);
    }
#endif /* (myUSB_VBUS_MONITORING_ENABLE) */


/*******************************************************************************
* Function Name: myUSB_RWUEnabled
****************************************************************************//**
*
*  This function returns the current remote wakeup status.
*  If the device supports remote wakeup, the application should use this
*  function to determine if remote wakeup was enabled by the host. When the
*  device is suspended and it determines the conditions to initiate a remote
*  wakeup are met, the application should use the USBFS_Force() function to
*  force the appropriate J and K states onto the USB bus, signaling a remote
*  wakeup.
*
*
* \return
*  Returns non-zero value if remote wakeup is enabled and zero otherwise.
*
* \globalvars
*  myUSB_deviceStatus - checked to determine remote status
*
*******************************************************************************/
uint8 myUSB_RWUEnabled(void) 
{
    uint8 result = myUSB_FALSE;

    if (0u != (myUSB_deviceStatus & myUSB_DEVICE_STATUS_REMOTE_WAKEUP))
    {
        result = myUSB_TRUE;
    }

    return (result);
}


/*******************************************************************************
* Function Name: myUSB_GetDeviceAddress
****************************************************************************//**
*
*  This function returns the currently assigned address for the USB device.
*
* \return
*  Returns the currently assigned address.
*  Returns 0 if the device has not yet been assigned an address.
*
*******************************************************************************/
uint8 myUSB_GetDeviceAddress(void) 
{
    return (uint8)(myUSB_CR0_REG & myUSB_CR0_DEVICE_ADDRESS_MASK);
}


/*******************************************************************************
* Function Name: myUSB_EnableSofInt
****************************************************************************//**
*
*  This function enables interrupt generation when a Start-of-Frame (SOF)
*  packet is received from the host.
*
*******************************************************************************/
void myUSB_EnableSofInt(void) 
{
#if (CY_PSOC4)
    /* Enable SOF interrupt interrupt source. */
    myUSB_INTR_SIE_MASK_REG |= (uint32) myUSB_INTR_SIE_SOF_INTR;
#else
    /* Enable SOF interrupt if it is present. */
    #if (myUSB_SOF_ISR_ACTIVE)
        CyIntEnable(myUSB_SOF_VECT_NUM);
    #endif /* (myUSB_SOF_ISR_ACTIVE) */
#endif /* (CY_PSOC4) */
}


/*******************************************************************************
* Function Name: myUSB_DisableSofInt
****************************************************************************//**
*
*  This function disables interrupt generation when a Start-of-Frame (SOF)
*  packet is received from the host.
*
*******************************************************************************/
void myUSB_DisableSofInt(void) 
{
#if (CY_PSOC4)
    /* Disable SOF interrupt interrupt source. */
    myUSB_INTR_SIE_MASK_REG &= (uint32) ~myUSB_INTR_SIE_SOF_INTR;
#else
    /* Disable SOF interrupt if it is present. */
    #if (myUSB_SOF_ISR_ACTIVE)
        CyIntDisable(myUSB_SOF_VECT_NUM);
    #endif /* (myUSB_SOF_ISR_ACTIVE) */
#endif /* (CY_PSOC4) */
}


#if (myUSB_BATT_CHARG_DET_ENABLE)
    /***************************************************************************
    * Function Name: myUSB_DetectPortType
    ************************************************************************//**
    *
    *   This function implements the USB  Battery Charger Detection (BCD)
    *   algorithm to determine the type of USB host downstream port. This API
    *   is available only for PSoC 4 devices, and should be called when the VBUS
    *   voltage transition (OFF to ON) is detected on the bus. If the USB device
    *   functionality is enabled, this API first calls USBFS_Stop() API
    *   internally to disable the USB device functionality, and then proceeds to
    *   implement the BCD algorithm to detect the USB host port type.
    *   The USBFS_Start() API should be called after this API if the USB
    *   communication needs to be initiated with the host.
    *   *Note* This API is generated only if the “Enable Battery Charging 
    *   Detection” option is enabled in the “Advanced” tab of the component GUI.
    *   *Note* API implements the steps 2-4 of the BCD algorithm which are 
    *   - Data Contact Detect
    *   - Primary Detection 
    *   - Secondary Detection 
    * 
    *   The first step of BCD algorithm, namely, VBUS detection shall be handled 
    *   at the application firmware level.
    *
    * \return
    *   The return value can be the following:
    *   Return Value                      |Description
    *   ----------------------------------|-------------------------------------
    *   myUSB_BCD_PORT_SDP     | Standard downstream port detected
    *   myUSB_BCD_PORT_CDP     | Charging downstream port detected
    *   myUSB_BCD_PORT_DCP     | Dedicated charging port detected
    *   myUSB_BCD_PORT_UNKNOWN | Unable to detect charging port type (proprietary charger type)
    *   myUSB_BCD_PORT_ERR     | Error condition in detection process
    *
    *
    * \sideeffects
    *
    *  USB device functionality is disabled by this API if not already disabled.
    *
    ***************************************************************************/
    uint8 myUSB_Bcd_DetectPortType(void)
    {
        uint32 bkPwrCtrl;
        uint32 cr1RegVal;
        uint32 secondaryDetection = 0u;
        uint8 result = myUSB_BCD_PORT_UNKNOWN;

        /*Check USB Started and Stop it*/
        if(0u != myUSB_initVar)
        {
            myUSB_Stop();
        }
        /*Initialize USBFS IP for Charger detection*/

        /*Enable clock to USB IP. */
        myUSB_USB_CLK_EN_REG = myUSB_USB_CLK_CSR_CLK_EN;

        /* Enable USBIO control on drive mode of D+ and D- pins. */
        myUSB_USBIO_CR1_REG &= ~ (uint32) myUSB_USBIO_CR1_IOMODE;

        /* Select VBUS detection source and clear PHY isolate. The application
        *  level must ensure that VBUS is valid. There is no need to wait 2us
        *  before VBUS is valid.
        */
        bkPwrCtrl = myUSB_POWER_CTRL_REG;
        myUSB_POWER_CTRL_REG = myUSB_DEFAULT_POWER_CTRL_VBUS\
                            & (~myUSB_POWER_CTRL_ENABLE_VBUS_PULLDOWN)\
                            & (~myUSB_POWER_CTRL_ENABLE_DM_PULLDOWN);


        /* Enable PHY detector and single-ended and differential receivers.
         * Enable charger detection.  */
        myUSB_POWER_CTRL_REG |= myUSB_DEFAULT_POWER_CTRL_PHY\
                                         | myUSB_POWER_CTRL_ENABLE_CHGDET;

        /* Suspend clear sequence. */
        myUSB_POWER_CTRL_REG &= (uint32) ~myUSB_POWER_CTRL_SUSPEND;
        CyDelayUs(myUSB_WAIT_SUSPEND_DEL_DISABLE);
        myUSB_POWER_CTRL_REG &= (uint32) ~myUSB_POWER_CTRL_SUSPEND_DEL;

        /* Data connection detection
        * Realization with delay as Hard IP does not support DCD 300 ms.
        */
        #if defined (myUSB_NO_DCD)
        CyDelay(myUSB_BCD_TIMEOUT);
        #else
        /* DCD implementation:*/

        {
            uint16 timeout = myUSB_BCD_TIMEOUT;
            uint8 connectionApproved = 0u;
            uint8 connected = 0u;

            /*   BCD spec 1.2: Turns on Idp_src and D- pull-down resistor */
            myUSB_POWER_CTRL_REG |= myUSB_POWER_CTRL_ENABLE_DM_PULLDOWN;
            myUSB_CHGDET_CTRL_REG |= myUSB_CHGDET_CTRL_DCD_SRC_EN;

            /* BCD spec 1.2: Waits for D+ to be low for a time of Tdcd_dbnc*/
            while ((0u != timeout) && (0u == connectionApproved))
            {
                if (0u == (myUSB_USBIO_CR1_REG & myUSB_USBIO_CR1_DP0))
                {
                    connected++;
                }
                else
                {
                    connected = 0u;
                }
                connectionApproved = (myUSB_BCD_TDCD_DBNC < connected) ? 1u:0u;
                CyDelay(1u);
                timeout--;
            }

            /*   BCD spec 1.2: Turns off Idp_src. */
            myUSB_CHGDET_CTRL_REG &= ~myUSB_CHGDET_CTRL_DCD_SRC_EN;
        }
        #endif /*(myUSB_NO_DCD)*/

        /* Primary detection: enable VDP_SRC on D+ and IDM_SINK on D-. */
        myUSB_CHGDET_CTRL_REG = myUSB_CHGDET_CTRL_PRIMARY;
        CyDelay(myUSB_BCD_PRIMARY_WAIT);
        cr1RegVal = myUSB_USBIO_CR1_REG;

        /* Check is it SDP or DCP/CDP, read comparator 2 output. */
        if (0u == (myUSB_CHGDET_CTRL_REG & myUSB_CHGDET_CTRL_COMP_OUT))
        {
            /* Check status of D- line. */
            if (0u == (cr1RegVal & myUSB_USBIO_CR1_DM0))
            {
                result = myUSB_BCD_PORT_SDP;
            }
            else
            {
                /* ERROR: such combination is impossible. Abort charger
                 * detection.
                */
                result = myUSB_BCD_PORT_ERR;
            }
        }
        else
        {
                /* Need Secondary detection. Charging port: DCP or proprietary*/
                secondaryDetection = 1u;
        }

        /* Secondary detection: Set CHGDET_CTRL register to enable VDM_SRC on D- and IDP_SINK on D+. */

        if (0u != secondaryDetection)
        {
            myUSB_CHGDET_CTRL_REG = myUSB_CHGDET_CTRL_DEFAULT \
                                                | myUSB_CHGDET_CTRL_SECONDARY;
            CyDelay(myUSB_BCD_SECONDARY_WAIT);
            cr1RegVal = myUSB_USBIO_CR1_REG;

            /* Check is it SDP or DCP/CDP, read comparator 1 output. */
            if (0u == (myUSB_CHGDET_CTRL_REG & myUSB_CHGDET_CTRL_COMP_OUT))
            {
                /* Check status of D+ line. */
                if (0u == (cr1RegVal & myUSB_USBIO_CR1_DP0))
                {
                    result = myUSB_BCD_PORT_CDP;
                }
                else
                {
                    /* ERROR: such combination is impossible. Abort charger
                     * detection.
                    */
                    result = myUSB_BCD_PORT_ERR;
                }
            }
            else
            {
                /* Check status of D+ line. */
                if (0u == (cr1RegVal & myUSB_USBIO_CR1_DP0))
                {
                    result = myUSB_BCD_PORT_DCP;
                }
                else
                {
                    /* It is may be proprietary charger. Proprietary charge is
                     * not supported byHardware IP block.
                    */
                    result = myUSB_BCD_PORT_UNKNOWN;
                }
            }
        }

        /* Restore CHGDET_CTRL. */
        myUSB_CHGDET_CTRL_REG = 0u;

        /*Revert registers back*/
        myUSB_POWER_CTRL_REG = bkPwrCtrl;
        myUSB_USBIO_CR1_REG |= (uint32) myUSB_USBIO_CR1_IOMODE;
        myUSB_USB_CLK_EN_REG = ~myUSB_USB_CLK_CSR_CLK_EN;

        return (result);
    }
#endif  /* (myUSB_BATT_CHARG_DET_ENABLE) */


#if (myUSB_LPM_ACTIVE)
    /***************************************************************************
    * Function Name: myUSB_Lpm_GetBeslValue
    ************************************************************************//**
    *
    *  This function returns the Best Effort Service Latency (BESL) value
    *  sent by the host as part of the LPM token transaction.
    *
    * \return
    *  4-bit BESL value received in the LPM token packet from the host
    *
    *
    ***************************************************************************/
    uint32 myUSB_Lpm_GetBeslValue(void)
    {
        return (uint32) (myUSB_LPM_STAT_REG & myUSB_LPM_STAT_LPM_BESL_MASK);
    }


    /***************************************************************************
    * Function Name: myUSB_Lpm_RemoteWakeUpAllowed
    ************************************************************************//**
    *
    *  This function returns the remote wakeup permission set for the device by
    *  the host as part of the LPM token transaction.
    *
    * \return
    *   0 - remote wakeup not allowed, 1 - remote wakeup allowed
    *
    *
    ***************************************************************************/
    uint32 myUSB_Lpm_RemoteWakeUpAllowed(void)
    {
        return (uint32) (myUSB_LPM_STAT_REG & myUSB_LPM_STAT_LPM_REMOTE_WAKE);
    }


    /***************************************************************************
    * Function Name: myUSB_Lpm_SetResponse
    ************************************************************************//**
    *
    *  This function configures the response in the handshake packet the device
    *  has to send when an LPM token packet is received.
    *
    * \param response
    *   type of response to return for an LPM token packet
    *   Allowed response values:
    *       - myUSB_LPM_REQ_ACK - next LPM request will be
    *                                           responded with ACK
    *       - myUSB_LPM_REQ_NACK - next LPM request will be
    *                                           responded with NACK
    *       - myUSB_LPM_REQ_NYET - next LPM request will be
    *                                           responded with NYET
    *
    ***************************************************************************/
    void myUSB_Lpm_SetResponse(uint32 response)
    {
        uint32 lpmCtrl = myUSB_LPM_CTRL_REG & (uint32) ~myUSB_LPM_CTRL_ACK_NYET_MASK;

        myUSB_LPM_CTRL_REG = lpmCtrl | ((uint32) response & myUSB_LPM_CTRL_ACK_NYET_MASK);
    }


    /***************************************************************************
    * Function Name: myUSB_Lpm_GetResponse
    ************************************************************************//**
    *
    *  This function returns the currently configured response value that the
    *  device will send as part of the handshake packet when an LPM token
    *  packet is received.
    *
    * \return
    *   type of handshake response that will be returned by the device
    *   for an LPM token packet
    *   Possible response values:
    *       - myUSB_LPM_REQ_ACK - next LPM request will be responded
    *                                        with ACK
    *       - myUSB_LPM_REQ_NACK - next LPM request will be responded
    *                                        with NACK
    *       - myUSB_LPM_REQ_NYET - next LPM request will be responded
    *                                        with NYET
    *
    ***************************************************************************/
    uint32 myUSB_Lpm_GetResponse(void)
    {

        return  ((uint32) myUSB_LPM_CTRL_REG & (uint32)myUSB_LPM_CTRL_ACK_NYET_MASK);
    }


#endif /* (myUSB_LPM_ACTIVE) */


/* [] END OF FILE */
