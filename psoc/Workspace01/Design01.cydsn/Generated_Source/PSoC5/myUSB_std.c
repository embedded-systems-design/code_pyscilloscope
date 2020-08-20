/***************************************************************************//**
* \file myUSB_std.c
* \version 3.20
*
* \brief
*  This file contains the USB Standard request handler.
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myUSB_pvt.h"

/***************************************
*   Static data allocation
***************************************/

#if defined(myUSB_ENABLE_FWSN_STRING)
    static volatile uint8* myUSB_fwSerialNumberStringDescriptor;
    static volatile uint8  myUSB_snStringConfirm = myUSB_FALSE;
#endif  /* (myUSB_ENABLE_FWSN_STRING) */

#if defined(myUSB_ENABLE_FWSN_STRING)
    /***************************************************************************
    * Function Name: myUSB_SerialNumString
    ************************************************************************//**
    *
    *  This function is available only when the User Call Back option in the 
    *  Serial Number String descriptor properties is selected. Application 
    *  firmware can provide the source of the USB device serial number string 
    *  descriptor during run time. The default string is used if the application 
    *  firmware does not use this function or sets the wrong string descriptor.
    *
    *  \param snString:  Pointer to the user-defined string descriptor. The 
    *  string descriptor should meet the Universal Serial Bus Specification 
    *  revision 2.0 chapter 9.6.7
    *  Offset|Size|Value|Description
    *  ------|----|------|---------------------------------
    *  0     |1   |N     |Size of this descriptor in bytes
    *  1     |1   |0x03  |STRING Descriptor Type
    *  2     |N-2 |Number|UNICODE encoded string
    *  
    * *For example:* uint8 snString[16]={0x0E,0x03,'F',0,'W',0,'S',0,'N',0,'0',0,'1',0};
    *
    * \reentrant
    *  No.
    *
    ***************************************************************************/
    void  myUSB_SerialNumString(uint8 snString[]) 
    {
        myUSB_snStringConfirm = myUSB_FALSE;
        
        if (snString != NULL)
        {
            /* Check descriptor validation */
            if ((snString[0u] > 1u) && (snString[1u] == myUSB_DESCR_STRING))
            {
                myUSB_fwSerialNumberStringDescriptor = snString;
                myUSB_snStringConfirm = myUSB_TRUE;
            }
        }
    }
#endif  /* myUSB_ENABLE_FWSN_STRING */


/*******************************************************************************
* Function Name: myUSB_HandleStandardRqst
****************************************************************************//**
*
*  This Routine dispatches standard requests
*
*
* \return
*  TRUE if request handled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_HandleStandardRqst(void) 
{
    uint8 requestHandled = myUSB_FALSE;
    uint8 interfaceNumber;
    uint8 configurationN;
    uint8 bmRequestType = myUSB_bmRequestTypeReg;

#if defined(myUSB_ENABLE_STRINGS)
    volatile uint8 *pStr = 0u;
    #if defined(myUSB_ENABLE_DESCRIPTOR_STRINGS)
        uint8 nStr;
        uint8 descrLength;
    #endif /* (myUSB_ENABLE_DESCRIPTOR_STRINGS) */
#endif /* (myUSB_ENABLE_STRINGS) */
    
    static volatile uint8 myUSB_tBuffer[myUSB_STATUS_LENGTH_MAX];
    const T_myUSB_LUT CYCODE *pTmp;

    myUSB_currentTD.count = 0u;

    if (myUSB_RQST_DIR_D2H == (bmRequestType & myUSB_RQST_DIR_MASK))
    {
        /* Control Read */
        switch (myUSB_bRequestReg)
        {
            case myUSB_GET_DESCRIPTOR:
                if (myUSB_DESCR_DEVICE ==myUSB_wValueHiReg)
                {
                    pTmp = myUSB_GetDeviceTablePtr();
                    myUSB_currentTD.pData = (volatile uint8 *)pTmp->p_list;
                    myUSB_currentTD.count = myUSB_DEVICE_DESCR_LENGTH;
                    
                    requestHandled  = myUSB_InitControlRead();
                }
                else if (myUSB_DESCR_CONFIG == myUSB_wValueHiReg)
                {
                    pTmp = myUSB_GetConfigTablePtr((uint8) myUSB_wValueLoReg);
                    
                    /* Verify that requested descriptor exists */
                    if (pTmp != NULL)
                    {
                        myUSB_currentTD.pData = (volatile uint8 *)pTmp->p_list;
                        myUSB_currentTD.count = (uint16)((uint16)(myUSB_currentTD.pData)[myUSB_CONFIG_DESCR_TOTAL_LENGTH_HI] << 8u) | \
                                                                            (myUSB_currentTD.pData)[myUSB_CONFIG_DESCR_TOTAL_LENGTH_LOW];
                        requestHandled  = myUSB_InitControlRead();
                    }
                }
                
            #if(myUSB_BOS_ENABLE)
                else if (myUSB_DESCR_BOS == myUSB_wValueHiReg)
                {
                    pTmp = myUSB_GetBOSPtr();
                    
                    /* Verify that requested descriptor exists */
                    if (pTmp != NULL)
                    {
                        myUSB_currentTD.pData = (volatile uint8 *)pTmp;
                        myUSB_currentTD.count = ((uint16)((uint16)(myUSB_currentTD.pData)[myUSB_BOS_DESCR_TOTAL_LENGTH_HI] << 8u)) | \
                                                                             (myUSB_currentTD.pData)[myUSB_BOS_DESCR_TOTAL_LENGTH_LOW];
                        requestHandled  = myUSB_InitControlRead();
                    }
                }
            #endif /*(myUSB_BOS_ENABLE)*/
            
            #if defined(myUSB_ENABLE_STRINGS)
                else if (myUSB_DESCR_STRING == myUSB_wValueHiReg)
                {
                /* Descriptor Strings */
                #if defined(myUSB_ENABLE_DESCRIPTOR_STRINGS)
                    nStr = 0u;
                    pStr = (volatile uint8 *) &myUSB_STRING_DESCRIPTORS[0u];
                    
                    while ((myUSB_wValueLoReg > nStr) && (*pStr != 0u))
                    {
                        /* Read descriptor length from 1st byte */
                        descrLength = *pStr;
                        /* Move to next string descriptor */
                        pStr = &pStr[descrLength];
                        nStr++;
                    }
                #endif /* (myUSB_ENABLE_DESCRIPTOR_STRINGS) */
                
                /* Microsoft OS String */
                #if defined(myUSB_ENABLE_MSOS_STRING)
                    if (myUSB_STRING_MSOS == myUSB_wValueLoReg)
                    {
                        pStr = (volatile uint8 *)& myUSB_MSOS_DESCRIPTOR[0u];
                    }
                #endif /* (myUSB_ENABLE_MSOS_STRING) */
                
                /* SN string */
                #if defined(myUSB_ENABLE_SN_STRING)
                    if ((myUSB_wValueLoReg != 0u) && 
                        (myUSB_wValueLoReg == myUSB_DEVICE0_DESCR[myUSB_DEVICE_DESCR_SN_SHIFT]))
                    {
                    #if defined(myUSB_ENABLE_IDSN_STRING)
                        /* Read DIE ID and generate string descriptor in RAM */
                        myUSB_ReadDieID(myUSB_idSerialNumberStringDescriptor);
                        pStr = myUSB_idSerialNumberStringDescriptor;
                    #elif defined(myUSB_ENABLE_FWSN_STRING)
                        
                        if(myUSB_snStringConfirm != myUSB_FALSE)
                        {
                            pStr = myUSB_fwSerialNumberStringDescriptor;
                        }
                        else
                        {
                            pStr = (volatile uint8 *)&myUSB_SN_STRING_DESCRIPTOR[0u];
                        }
                    #else
                        pStr = (volatile uint8 *)&myUSB_SN_STRING_DESCRIPTOR[0u];
                    #endif  /* (myUSB_ENABLE_IDSN_STRING) */
                    }
                #endif /* (myUSB_ENABLE_SN_STRING) */
                
                    if (*pStr != 0u)
                    {
                        myUSB_currentTD.count = *pStr;
                        myUSB_currentTD.pData = pStr;
                        requestHandled  = myUSB_InitControlRead();
                    }
                }
            #endif /*  myUSB_ENABLE_STRINGS */
                else
                {
                    requestHandled = myUSB_DispatchClassRqst();
                }
                break;
                
            case myUSB_GET_STATUS:
                switch (bmRequestType & myUSB_RQST_RCPT_MASK)
                {
                    case myUSB_RQST_RCPT_EP:
                        myUSB_currentTD.count = myUSB_EP_STATUS_LENGTH;
                        myUSB_tBuffer[0u]     = myUSB_EP[myUSB_wIndexLoReg & myUSB_DIR_UNUSED].hwEpState;
                        myUSB_tBuffer[1u]     = 0u;
                        myUSB_currentTD.pData = &myUSB_tBuffer[0u];
                        
                        requestHandled  = myUSB_InitControlRead();
                        break;
                    case myUSB_RQST_RCPT_DEV:
                        myUSB_currentTD.count = myUSB_DEVICE_STATUS_LENGTH;
                        myUSB_tBuffer[0u]     = myUSB_deviceStatus;
                        myUSB_tBuffer[1u]     = 0u;
                        myUSB_currentTD.pData = &myUSB_tBuffer[0u];
                        
                        requestHandled  = myUSB_InitControlRead();
                        break;
                    default:    /* requestHandled is initialized as FALSE by default */
                        break;
                }
                break;
                
            case myUSB_GET_CONFIGURATION:
                myUSB_currentTD.count = 1u;
                myUSB_currentTD.pData = (volatile uint8 *) &myUSB_configuration;
                requestHandled  = myUSB_InitControlRead();
                break;
                
            case myUSB_GET_INTERFACE:
                myUSB_currentTD.count = 1u;
                myUSB_currentTD.pData = (volatile uint8 *) &myUSB_interfaceSetting[myUSB_wIndexLoReg];
                requestHandled  = myUSB_InitControlRead();
                break;
                
            default: /* requestHandled is initialized as FALSE by default */
                break;
        }
    }
    else
    {
        /* Control Write */
        switch (myUSB_bRequestReg)
        {
            case myUSB_SET_ADDRESS:
                /* Store address to be set in myUSB_NoDataControlStatusStage(). */
                myUSB_deviceAddress = (uint8) myUSB_wValueLoReg;
                requestHandled = myUSB_InitNoDataControlTransfer();
                break;
                
            case myUSB_SET_CONFIGURATION:
                configurationN = myUSB_wValueLoReg;
                
                /* Verify that configuration descriptor exists */
                if(configurationN > 0u)
                {
                    pTmp = myUSB_GetConfigTablePtr((uint8) configurationN - 1u);
                }
                
                /* Responds with a Request Error when configuration number is invalid */
                if (((configurationN > 0u) && (pTmp != NULL)) || (configurationN == 0u))
                {
                    /* Set new configuration if it has been changed */
                    if(configurationN != myUSB_configuration)
                    {
                        myUSB_configuration = (uint8) configurationN;
                        myUSB_configurationChanged = myUSB_TRUE;
                        myUSB_Config(myUSB_TRUE);
                    }
                    requestHandled = myUSB_InitNoDataControlTransfer();
                }
                break;
                
            case myUSB_SET_INTERFACE:
                if (0u != myUSB_ValidateAlternateSetting())
                {
                    /* Get interface number from the request. */
                    interfaceNumber = myUSB_wIndexLoReg;
                    myUSB_interfaceNumber = (uint8) myUSB_wIndexLoReg;
                     
                    /* Check if alternate settings is changed for interface. */
                    if (myUSB_interfaceSettingLast[interfaceNumber] != myUSB_interfaceSetting[interfaceNumber])
                    {
                        myUSB_configurationChanged = myUSB_TRUE;
                    
                        /* Change alternate setting for the endpoints. */
                    #if (myUSB_EP_MANAGEMENT_MANUAL && myUSB_EP_ALLOC_DYNAMIC)
                        myUSB_Config(myUSB_FALSE);
                    #else
                        myUSB_ConfigAltChanged();
                    #endif /* (myUSB_EP_MANAGEMENT_MANUAL && myUSB_EP_ALLOC_DYNAMIC) */
                    }
                    
                    requestHandled = myUSB_InitNoDataControlTransfer();
                }
                break;
                
            case myUSB_CLEAR_FEATURE:
                switch (bmRequestType & myUSB_RQST_RCPT_MASK)
                {
                    case myUSB_RQST_RCPT_EP:
                        if (myUSB_wValueLoReg == myUSB_ENDPOINT_HALT)
                        {
                            requestHandled = myUSB_ClearEndpointHalt();
                        }
                        break;
                    case myUSB_RQST_RCPT_DEV:
                        /* Clear device REMOTE_WAKEUP */
                        if (myUSB_wValueLoReg == myUSB_DEVICE_REMOTE_WAKEUP)
                        {
                            myUSB_deviceStatus &= (uint8)~myUSB_DEVICE_STATUS_REMOTE_WAKEUP;
                            requestHandled = myUSB_InitNoDataControlTransfer();
                        }
                        break;
                    case myUSB_RQST_RCPT_IFC:
                        /* Validate interfaceNumber */
                        if (myUSB_wIndexLoReg < myUSB_MAX_INTERFACES_NUMBER)
                        {
                            myUSB_interfaceStatus[myUSB_wIndexLoReg] &= (uint8) ~myUSB_wValueLoReg;
                            requestHandled = myUSB_InitNoDataControlTransfer();
                        }
                        break;
                    default:    /* requestHandled is initialized as FALSE by default */
                        break;
                }
                break;
                
            case myUSB_SET_FEATURE:
                switch (bmRequestType & myUSB_RQST_RCPT_MASK)
                {
                    case myUSB_RQST_RCPT_EP:
                        if (myUSB_wValueLoReg == myUSB_ENDPOINT_HALT)
                        {
                            requestHandled = myUSB_SetEndpointHalt();
                        }
                        break;
                        
                    case myUSB_RQST_RCPT_DEV:
                        /* Set device REMOTE_WAKEUP */
                        if (myUSB_wValueLoReg == myUSB_DEVICE_REMOTE_WAKEUP)
                        {
                            myUSB_deviceStatus |= myUSB_DEVICE_STATUS_REMOTE_WAKEUP;
                            requestHandled = myUSB_InitNoDataControlTransfer();
                        }
                        break;
                        
                    case myUSB_RQST_RCPT_IFC:
                        /* Validate interfaceNumber */
                        if (myUSB_wIndexLoReg < myUSB_MAX_INTERFACES_NUMBER)
                        {
                            myUSB_interfaceStatus[myUSB_wIndexLoReg] &= (uint8) ~myUSB_wValueLoReg;
                            requestHandled = myUSB_InitNoDataControlTransfer();
                        }
                        break;
                    
                    default:    /* requestHandled is initialized as FALSE by default */
                        break;
                }
                break;
                
            default:    /* requestHandled is initialized as FALSE by default */
                break;
        }
    }
    
    return (requestHandled);
}


#if defined(myUSB_ENABLE_IDSN_STRING)
    /***************************************************************************
    * Function Name: myUSB_ReadDieID
    ************************************************************************//**
    *
    *  This routine read Die ID and generate Serial Number string descriptor.
    *
    *  \param descr:  pointer on string descriptor. This string size has to be equal or
    *          greater than myUSB_IDSN_DESCR_LENGTH.
    *
    *
    * \reentrant
    *  No.
    *
    ***************************************************************************/
    void myUSB_ReadDieID(uint8 descr[]) 
    {
        const char8 CYCODE hex[] = "0123456789ABCDEF";
        uint8 i;
        uint8 j = 0u;
        uint8 uniqueId[8u];

        if (NULL != descr)
        {
            /* Initialize descriptor header. */
            descr[0u] = myUSB_IDSN_DESCR_LENGTH;
            descr[1u] = myUSB_DESCR_STRING;
            
            /* Unique ID size is 8 bytes. */
            CyGetUniqueId((uint32 *) uniqueId);

            /* Fill descriptor with unique device ID. */
            for (i = 2u; i < myUSB_IDSN_DESCR_LENGTH; i += 4u)
            {
                descr[i]      = (uint8) hex[(uniqueId[j] >> 4u)];
                descr[i + 1u] = 0u;
                descr[i + 2u] = (uint8) hex[(uniqueId[j] & 0x0Fu)];
                descr[i + 3u] = 0u;
                ++j;
            }
        }
    }
#endif /* (myUSB_ENABLE_IDSN_STRING) */


/*******************************************************************************
* Function Name: myUSB_ConfigReg
****************************************************************************//**
*
*  This routine configures hardware registers from the variables.
*  It is called from myUSB_Config() function and from RestoreConfig
*  after Wakeup.
*
*******************************************************************************/
void myUSB_ConfigReg(void) 
{
    uint8 ep;

#if (myUSB_EP_MANAGEMENT_DMA_AUTO)
    uint8 epType = 0u;
#endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */

    /* Go thought all endpoints and set hardware configuration */
    for (ep = myUSB_EP1; ep < myUSB_MAX_EP; ++ep)
    {
        myUSB_ARB_EP_BASE.arbEp[ep].epCfg = myUSB_ARB_EPX_CFG_DEFAULT;
        
    #if (myUSB_EP_MANAGEMENT_DMA)
        /* Enable arbiter endpoint interrupt sources */
        myUSB_ARB_EP_BASE.arbEp[ep].epIntEn = myUSB_ARB_EPX_INT_MASK;
    #endif /* (myUSB_EP_MANAGEMENT_DMA) */
    
        if (myUSB_EP[ep].epMode != myUSB_MODE_DISABLE)
        {
            if (0u != (myUSB_EP[ep].addr & myUSB_DIR_IN))
            {
                myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = myUSB_MODE_NAK_IN;
                
            #if (myUSB_EP_MANAGEMENT_DMA_AUTO && CY_PSOC4)
                /* Clear DMA_TERMIN for IN endpoint. */
                myUSB_ARB_EP_BASE.arbEp[ep].epIntEn &= (uint32) ~myUSB_ARB_EPX_INT_DMA_TERMIN;
            #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO && CY_PSOC4) */
            }
            else
            {
                myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = myUSB_MODE_NAK_OUT;

            #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
                /* (CY_PSOC4): DMA_TERMIN for OUT endpoint is set above. */
                
                /* Prepare endpoint type mask. */
                epType |= (uint8) (0x01u << (ep - myUSB_EP1));
            #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
            }
        }
        else
        {
            myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = myUSB_MODE_STALL_DATA_EP;
        }
        
    #if (!myUSB_EP_MANAGEMENT_DMA_AUTO)
        #if (CY_PSOC4)
            myUSB_ARB_EP16_BASE.arbEp[ep].rwRa16  = (uint32) myUSB_EP[ep].buffOffset;
            myUSB_ARB_EP16_BASE.arbEp[ep].rwWa16  = (uint32) myUSB_EP[ep].buffOffset;
        #else
            myUSB_ARB_EP_BASE.arbEp[ep].rwRa    = LO8(myUSB_EP[ep].buffOffset);
            myUSB_ARB_EP_BASE.arbEp[ep].rwRaMsb = HI8(myUSB_EP[ep].buffOffset);
            myUSB_ARB_EP_BASE.arbEp[ep].rwWa    = LO8(myUSB_EP[ep].buffOffset);
            myUSB_ARB_EP_BASE.arbEp[ep].rwWaMsb = HI8(myUSB_EP[ep].buffOffset);
        #endif /* (CY_PSOC4) */
    #endif /* (!myUSB_EP_MANAGEMENT_DMA_AUTO) */
    }

#if (myUSB_EP_MANAGEMENT_DMA_AUTO)
     /* BUF_SIZE depend on DMA_THRESS value:0x55-32 bytes  0x44-16 bytes 0x33-8 bytes 0x22-4 bytes 0x11-2 bytes */
    myUSB_BUF_SIZE_REG = myUSB_DMA_BUF_SIZE;

    /* Configure DMA burst threshold */
#if (CY_PSOC4)
    myUSB_DMA_THRES16_REG   = myUSB_DMA_BYTES_PER_BURST;
#else
    myUSB_DMA_THRES_REG     = myUSB_DMA_BYTES_PER_BURST;
    myUSB_DMA_THRES_MSB_REG = 0u;
#endif /* (CY_PSOC4) */
    myUSB_EP_ACTIVE_REG = myUSB_DEFAULT_ARB_INT_EN;
    myUSB_EP_TYPE_REG   = epType;
    
    /* Cfg_cmp bit set to 1 once configuration is complete. */
    /* Lock arbiter configtuation */
    myUSB_ARB_CFG_REG |= (uint8)  myUSB_ARB_CFG_CFG_CMP;
    /* Cfg_cmp bit set to 0 during configuration of PFSUSB Registers. */
    myUSB_ARB_CFG_REG &= (uint8) ~myUSB_ARB_CFG_CFG_CMP;

#endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */

    /* Enable interrupt SIE interurpt source from EP0-EP1 */
    myUSB_SIE_EP_INT_EN_REG = (uint8) myUSB_DEFAULT_SIE_EP_INT_EN;
}


/*******************************************************************************
* Function Name: myUSB_EpStateInit
****************************************************************************//**
*
*  This routine initialize state of Data end points based of its type: 
*   IN  - myUSB_IN_BUFFER_EMPTY (myUSB_EVENT_PENDING)
*   OUT - myUSB_OUT_BUFFER_EMPTY (myUSB_NO_EVENT_PENDING)
*
*******************************************************************************/
void myUSB_EpStateInit(void) 
{
    uint8 i;

    for (i = myUSB_EP1; i < myUSB_MAX_EP; i++)
    { 
        if (0u != (myUSB_EP[i].addr & myUSB_DIR_IN))
        {
            /* IN Endpoint */
            myUSB_EP[i].apiEpState = myUSB_EVENT_PENDING;
        }
        else
        {
            /* OUT Endpoint */
            myUSB_EP[i].apiEpState = myUSB_NO_EVENT_PENDING;
        }
    }
                    
}


/*******************************************************************************
* Function Name: myUSB_Config
****************************************************************************//**
*
*  This routine configures endpoints for the entire configuration by scanning
*  the configuration descriptor.
*
*  \param clearAltSetting: It configures the bAlternateSetting 0 for each interface.
*
* myUSB_interfaceClass - Initialized class array for each interface.
*   It is used for handling Class specific requests depend on interface class.
*   Different classes in multiple Alternate settings does not supported.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_Config(uint8 clearAltSetting) 
{
    uint8 ep;
    uint8 curEp;
    uint8 i;
    uint8 epType;
    const uint8 *pDescr;
    
    #if (!myUSB_EP_MANAGEMENT_DMA_AUTO)
        uint16 buffCount = 0u;
    #endif /* (!myUSB_EP_MANAGEMENT_DMA_AUTO) */

    const T_myUSB_LUT CYCODE *pTmp;
    const T_myUSB_EP_SETTINGS_BLOCK CYCODE *pEP;

    /* Clear endpoints settings */
    for (ep = 0u; ep < myUSB_MAX_EP; ++ep)
    {
        myUSB_EP[ep].attrib     = 0u;
        myUSB_EP[ep].hwEpState  = 0u;
        myUSB_EP[ep].epToggle   = 0u;
        myUSB_EP[ep].bufferSize = 0u;
        myUSB_EP[ep].interface  = 0u;
        myUSB_EP[ep].apiEpState = myUSB_NO_EVENT_PENDING;
        myUSB_EP[ep].epMode     = myUSB_MODE_DISABLE;   
    }

    /* Clear Alternate settings for all interfaces. */
    if (0u != clearAltSetting)
    {
        for (i = 0u; i < myUSB_MAX_INTERFACES_NUMBER; ++i)
        {
            myUSB_interfaceSetting[i]     = 0u;
            myUSB_interfaceSettingLast[i] = 0u;
        }
    }

    /* Init Endpoints and Device Status if configured */
    if (myUSB_configuration > 0u)
    {
        #if defined(myUSB_ENABLE_CDC_CLASS)
            uint8 cdcComNums = 0u;
        #endif  /* (myUSB_ENABLE_CDC_CLASS) */  

        pTmp = myUSB_GetConfigTablePtr(myUSB_configuration - 1u);
        
        /* Set Power status for current configuration */
        pDescr = (const uint8 *)pTmp->p_list;
        if ((pDescr[myUSB_CONFIG_DESCR_ATTRIB] & myUSB_CONFIG_DESCR_ATTRIB_SELF_POWERED) != 0u)
        {
            myUSB_deviceStatus |= (uint8)  myUSB_DEVICE_STATUS_SELF_POWERED;
        }
        else
        {
            myUSB_deviceStatus &= (uint8) ~myUSB_DEVICE_STATUS_SELF_POWERED;
        }
        
        /* Move to next element */
        pTmp = &pTmp[1u];
        ep = pTmp->c;  /* For this table, c is the number of endpoints configurations  */

        #if (myUSB_EP_MANAGEMENT_MANUAL && myUSB_EP_ALLOC_DYNAMIC)
            /* Configure for dynamic EP memory allocation */
            /* p_list points the endpoint setting table. */
            pEP = (T_myUSB_EP_SETTINGS_BLOCK *) pTmp->p_list;
            
            for (i = 0u; i < ep; i++)
            {     
                /* Compare current Alternate setting with EP Alt */
                if (myUSB_interfaceSetting[pEP->interface] == pEP->altSetting)
                {                                                          
                    curEp  = pEP->addr & myUSB_DIR_UNUSED;
                    epType = pEP->attributes & myUSB_EP_TYPE_MASK;
                    
                    myUSB_EP[curEp].addr       = pEP->addr;
                    myUSB_EP[curEp].attrib     = pEP->attributes;
                    myUSB_EP[curEp].bufferSize = pEP->bufferSize;

                    if (0u != (pEP->addr & myUSB_DIR_IN))
                    {
                        /* IN Endpoint */
                        myUSB_EP[curEp].epMode     = myUSB_GET_ACTIVE_IN_EP_CR0_MODE(epType);
                        myUSB_EP[curEp].apiEpState = myUSB_EVENT_PENDING;
                    
                    #if (defined(myUSB_ENABLE_MIDI_STREAMING) && (myUSB_MIDI_IN_BUFF_SIZE > 0))
                        if ((pEP->bMisc == myUSB_CLASS_AUDIO) && (epType == myUSB_EP_TYPE_BULK))
                        {
                            myUSB_midi_in_ep = curEp;
                        }
                    #endif  /* (myUSB_ENABLE_MIDI_STREAMING) */
                    }
                    else
                    {
                        /* OUT Endpoint */
                        myUSB_EP[curEp].epMode     = myUSB_GET_ACTIVE_OUT_EP_CR0_MODE(epType);
                        myUSB_EP[curEp].apiEpState = myUSB_NO_EVENT_PENDING;
                        
                    #if (defined(myUSB_ENABLE_MIDI_STREAMING) && (myUSB_MIDI_OUT_BUFF_SIZE > 0))
                        if ((pEP->bMisc == myUSB_CLASS_AUDIO) && (epType == myUSB_EP_TYPE_BULK))
                        {
                            myUSB_midi_out_ep = curEp;
                        }
                    #endif  /* (myUSB_ENABLE_MIDI_STREAMING) */
                    }

                #if(defined (myUSB_ENABLE_CDC_CLASS))
                    if((pEP->bMisc == myUSB_CLASS_CDC_DATA) ||(pEP->bMisc == myUSB_CLASS_CDC))
                    {
                        cdcComNums = myUSB_Cdc_EpInit(pEP, curEp, cdcComNums);
                    }
                #endif  /* (myUSB_ENABLE_CDC_CLASS) */
                }
                
                pEP = &pEP[1u];
            }
            
        #else
            for (i = myUSB_EP1; i < myUSB_MAX_EP; ++i)
            {
                /* p_list points the endpoint setting table. */
                pEP = (const T_myUSB_EP_SETTINGS_BLOCK CYCODE *) pTmp->p_list;
                /* Find max length for each EP and select it (length could be different in different Alt settings) */
                /* but other settings should be correct with regards to Interface alt Setting */
                
                for (curEp = 0u; curEp < ep; ++curEp)
                {
                    if (i == (pEP->addr & myUSB_DIR_UNUSED))
                    {
                        /* Compare endpoint buffers size with current size to find greater. */
                        if (myUSB_EP[i].bufferSize < pEP->bufferSize)
                        {
                            myUSB_EP[i].bufferSize = pEP->bufferSize;
                        }
                        
                        /* Compare current Alternate setting with EP Alt */
                        if (myUSB_interfaceSetting[pEP->interface] == pEP->altSetting)
                        {                            
                            myUSB_EP[i].addr = pEP->addr;
                            myUSB_EP[i].attrib = pEP->attributes;
                            
                            epType = pEP->attributes & myUSB_EP_TYPE_MASK;
                            
                            if (0u != (pEP->addr & myUSB_DIR_IN))
                            {
                                /* IN Endpoint */
                                myUSB_EP[i].epMode     = myUSB_GET_ACTIVE_IN_EP_CR0_MODE(epType);
                                myUSB_EP[i].apiEpState = myUSB_EVENT_PENDING;
                                
                            #if (defined(myUSB_ENABLE_MIDI_STREAMING) && (myUSB_MIDI_IN_BUFF_SIZE > 0))
                                if ((pEP->bMisc == myUSB_CLASS_AUDIO) && (epType == myUSB_EP_TYPE_BULK))
                                {
                                    myUSB_midi_in_ep = i;
                                }
                            #endif  /* (myUSB_ENABLE_MIDI_STREAMING) */
                            }
                            else
                            {
                                /* OUT Endpoint */
                                myUSB_EP[i].epMode     = myUSB_GET_ACTIVE_OUT_EP_CR0_MODE(epType);
                                myUSB_EP[i].apiEpState = myUSB_NO_EVENT_PENDING;
                                
                            #if (defined(myUSB_ENABLE_MIDI_STREAMING) && (myUSB_MIDI_OUT_BUFF_SIZE > 0))
                                if ((pEP->bMisc == myUSB_CLASS_AUDIO) && (epType == myUSB_EP_TYPE_BULK))
                                {
                                    myUSB_midi_out_ep = i;
                                }
                            #endif  /* (myUSB_ENABLE_MIDI_STREAMING) */
                            }

                        #if (defined(myUSB_ENABLE_CDC_CLASS))
                            if((pEP->bMisc == myUSB_CLASS_CDC_DATA) ||(pEP->bMisc == myUSB_CLASS_CDC))
                            {
                                cdcComNums = myUSB_Cdc_EpInit(pEP, i, cdcComNums);
                            }
                        #endif  /* (myUSB_ENABLE_CDC_CLASS) */

                            #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
                                break;  /* Use first EP setting in Auto memory management */
                            #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
                        }
                    }
                    
                    pEP = &pEP[1u];
                }
            }
        #endif /*  (myUSB_EP_MANAGEMENT_MANUAL && myUSB_EP_ALLOC_DYNAMIC) */

        /* Init class array for each interface and interface number for each EP.
        *  It is used for handling Class specific requests directed to either an
        *  interface or the endpoint.
        */
        /* p_list points the endpoint setting table. */
        pEP = (const T_myUSB_EP_SETTINGS_BLOCK CYCODE *) pTmp->p_list;
        for (i = 0u; i < ep; i++)
        {
            /* Configure interface number for each EP */
            myUSB_EP[pEP->addr & myUSB_DIR_UNUSED].interface = pEP->interface;
            pEP = &pEP[1u];
        }
        
        /* Init pointer on interface class table */
        myUSB_interfaceClass = myUSB_GetInterfaceClassTablePtr();
        
    /* Set the endpoint buffer addresses */
    #if (!myUSB_EP_MANAGEMENT_DMA_AUTO)
        buffCount = 0u;
        for (ep = myUSB_EP1; ep < myUSB_MAX_EP; ++ep)
        {
            myUSB_EP[ep].buffOffset = buffCount;        
            buffCount += myUSB_EP[ep].bufferSize;
            
        #if (myUSB_GEN_16BITS_EP_ACCESS)
            /* Align EP buffers to be event size to access 16-bits DR register. */
            buffCount += (0u != (buffCount & 0x01u)) ? 1u : 0u;
        #endif /* (myUSB_GEN_16BITS_EP_ACCESS) */            
        }
    #endif /* (!myUSB_EP_MANAGEMENT_DMA_AUTO) */

        /* Configure hardware registers */
        myUSB_ConfigReg();
    }
}


/*******************************************************************************
* Function Name: myUSB_ConfigAltChanged
****************************************************************************//**
*
*  This routine update configuration for the required endpoints only.
*  It is called after SET_INTERFACE request when Static memory allocation used.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_ConfigAltChanged(void) 
{
    uint8 ep;
    uint8 curEp;
    uint8 epType;
    uint8 i;
    uint8 interfaceNum;

    const T_myUSB_LUT CYCODE *pTmp;
    const T_myUSB_EP_SETTINGS_BLOCK CYCODE *pEP;

    /* Init Endpoints and Device Status if configured */
    if (myUSB_configuration > 0u)
    {
        /* Get number of endpoints configurations (ep). */
        pTmp = myUSB_GetConfigTablePtr(myUSB_configuration - 1u);
        pTmp = &pTmp[1u];
        ep = pTmp->c;

        /* Get pointer to endpoints setting table (pEP). */
        pEP = (const T_myUSB_EP_SETTINGS_BLOCK CYCODE *) pTmp->p_list;
        
        /* Look through all possible endpoint configurations. Find endpoints 
        * which belong to current interface and alternate settings for 
        * re-configuration.
        */
        interfaceNum = myUSB_interfaceNumber;
        for (i = 0u; i < ep; i++)
        {
            /* Find endpoints which belong to current interface and alternate settings. */
            if ((interfaceNum == pEP->interface) && 
                (myUSB_interfaceSetting[interfaceNum] == pEP->altSetting))
            {
                curEp  = ((uint8) pEP->addr & myUSB_DIR_UNUSED);
                epType = ((uint8) pEP->attributes & myUSB_EP_TYPE_MASK);
                
                /* Change the SIE mode for the selected EP to NAK ALL */
                myUSB_EP[curEp].epToggle   = 0u;
                myUSB_EP[curEp].addr       = pEP->addr;
                myUSB_EP[curEp].attrib     = pEP->attributes;
                myUSB_EP[curEp].bufferSize = pEP->bufferSize;

                if (0u != (pEP->addr & myUSB_DIR_IN))
                {
                    /* IN Endpoint */
                    myUSB_EP[curEp].epMode     = myUSB_GET_ACTIVE_IN_EP_CR0_MODE(epType);
                    myUSB_EP[curEp].apiEpState = myUSB_EVENT_PENDING;
                }
                else
                {
                    /* OUT Endpoint */
                    myUSB_EP[curEp].epMode     = myUSB_GET_ACTIVE_OUT_EP_CR0_MODE(epType);
                    myUSB_EP[curEp].apiEpState = myUSB_NO_EVENT_PENDING;
                }
                
                /* Make SIE to NAK any endpoint requests */
                myUSB_SIE_EP_BASE.sieEp[curEp].epCr0 = myUSB_MODE_NAK_IN_OUT;

            #if (myUSB_EP_MANAGEMENT_DMA_AUTO)
                /* Clear IN data ready. */
                myUSB_ARB_EP_BASE.arbEp[curEp].epCfg &= (uint8) ~myUSB_ARB_EPX_CFG_IN_DATA_RDY;

                /* Select endpoint number of reconfiguration */
                myUSB_DYN_RECONFIG_REG = (uint8) ((curEp - 1u) << myUSB_DYN_RECONFIG_EP_SHIFT);
                
                /* Request for dynamic re-configuration of endpoint. */
                myUSB_DYN_RECONFIG_REG |= myUSB_DYN_RECONFIG_ENABLE;
                
                /* Wait until block is ready for re-configuration */
                while (0u == (myUSB_DYN_RECONFIG_REG & myUSB_DYN_RECONFIG_RDY_STS))
                {
                }
                
                /* Once DYN_RECONFIG_RDY_STS bit is set, FW can change the EP configuration. */
                /* Change EP Type with new direction */
                if (0u != (pEP->addr & myUSB_DIR_IN))
                {
                    /* Set endpoint type: 0 - IN and 1 - OUT. */
                    myUSB_EP_TYPE_REG &= (uint8) ~(uint8)((uint8) 0x01u << (curEp - 1u));
                    
                #if (CY_PSOC4)
                    /* Clear DMA_TERMIN for IN endpoint */
                    myUSB_ARB_EP_BASE.arbEp[curEp].epIntEn &= (uint32) ~myUSB_ARB_EPX_INT_DMA_TERMIN;
                #endif /* (CY_PSOC4) */
                }
                else
                {
                    /* Set endpoint type: 0 - IN and 1- OUT. */
                    myUSB_EP_TYPE_REG |= (uint8) ((uint8) 0x01u << (curEp - 1u));
                    
                #if (CY_PSOC4)
                    /* Set DMA_TERMIN for OUT endpoint */
                    myUSB_ARB_EP_BASE.arbEp[curEp].epIntEn |= (uint32) myUSB_ARB_EPX_INT_DMA_TERMIN;
                #endif /* (CY_PSOC4) */
                }
                
                /* Complete dynamic re-configuration: all endpoint related status and signals 
                * are set into the default state.
                */
                myUSB_DYN_RECONFIG_REG &= (uint8) ~myUSB_DYN_RECONFIG_ENABLE;

            #else
                myUSB_SIE_EP_BASE.sieEp[curEp].epCnt0 = HI8(myUSB_EP[curEp].bufferSize);
                myUSB_SIE_EP_BASE.sieEp[curEp].epCnt1 = LO8(myUSB_EP[curEp].bufferSize);
                
                #if (CY_PSOC4)
                    myUSB_ARB_EP16_BASE.arbEp[curEp].rwRa16  = (uint32) myUSB_EP[curEp].buffOffset;
                    myUSB_ARB_EP16_BASE.arbEp[curEp].rwWa16  = (uint32) myUSB_EP[curEp].buffOffset;
                #else
                    myUSB_ARB_EP_BASE.arbEp[curEp].rwRa    = LO8(myUSB_EP[curEp].buffOffset);
                    myUSB_ARB_EP_BASE.arbEp[curEp].rwRaMsb = HI8(myUSB_EP[curEp].buffOffset);
                    myUSB_ARB_EP_BASE.arbEp[curEp].rwWa    = LO8(myUSB_EP[curEp].buffOffset);
                    myUSB_ARB_EP_BASE.arbEp[curEp].rwWaMsb = HI8(myUSB_EP[curEp].buffOffset);
                #endif /* (CY_PSOC4) */                
            #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */
            }
            
            pEP = &pEP[1u]; /* Get next EP element */
        }
        
        /* The main loop has to re-enable DMA and OUT endpoint */
    }
}


/*******************************************************************************
* Function Name: myUSB_GetConfigTablePtr
****************************************************************************//**
*
*  This routine returns a pointer a configuration table entry
*
*  \param confIndex:  Configuration Index
*
* \return
*  Device Descriptor pointer or NULL when descriptor does not exist.
*
*******************************************************************************/
const T_myUSB_LUT CYCODE *myUSB_GetConfigTablePtr(uint8 confIndex)
                                                        
{
    /* Device Table */
    const T_myUSB_LUT CYCODE *pTmp;

    pTmp = (const T_myUSB_LUT CYCODE *) myUSB_TABLE[myUSB_device].p_list;

    /* The first entry points to the Device Descriptor,
    *  the second entry point to the BOS Descriptor
    *  the rest configuration entries.
    *  Set pointer to the first Configuration Descriptor
    */
    pTmp = &pTmp[2u];
    /* For this table, c is the number of configuration descriptors  */
    if(confIndex >= pTmp->c)   /* Verify that required configuration descriptor exists */
    {
        pTmp = (const T_myUSB_LUT CYCODE *) NULL;
    }
    else
    {
        pTmp = (const T_myUSB_LUT CYCODE *) pTmp[confIndex].p_list;
    }

    return (pTmp);
}


#if (myUSB_BOS_ENABLE)
    /*******************************************************************************
    * Function Name: myUSB_GetBOSPtr
    ****************************************************************************//**
    *
    *  This routine returns a pointer a BOS table entry
    *
    *  
    *
    * \return
    *  BOS Descriptor pointer or NULL when descriptor does not exist.
    *
    *******************************************************************************/
    const T_myUSB_LUT CYCODE *myUSB_GetBOSPtr(void)
                                                            
    {
        /* Device Table */
        const T_myUSB_LUT CYCODE *pTmp;

        pTmp = (const T_myUSB_LUT CYCODE *) myUSB_TABLE[myUSB_device].p_list;

        /* The first entry points to the Device Descriptor,
        *  the second entry points to the BOS Descriptor
        */
        pTmp = &pTmp[1u];
        pTmp = (const T_myUSB_LUT CYCODE *) pTmp->p_list;
        return (pTmp);
    }
#endif /* (myUSB_BOS_ENABLE) */


/*******************************************************************************
* Function Name: myUSB_GetDeviceTablePtr
****************************************************************************//**
*
*  This routine returns a pointer to the Device table
*
* \return
*  Device Table pointer
*
*******************************************************************************/
const T_myUSB_LUT CYCODE *myUSB_GetDeviceTablePtr(void)
                                                            
{
    /* Device Table */
    return( (const T_myUSB_LUT CYCODE *) myUSB_TABLE[myUSB_device].p_list );
}


/*******************************************************************************
* Function Name: USB_GetInterfaceClassTablePtr
****************************************************************************//**
*
*  This routine returns Interface Class table pointer, which contains
*  the relation between interface number and interface class.
*
* \return
*  Interface Class table pointer.
*
*******************************************************************************/
const uint8 CYCODE *myUSB_GetInterfaceClassTablePtr(void)
                                                        
{
    const T_myUSB_LUT CYCODE *pTmp;
    const uint8 CYCODE *pInterfaceClass;
    uint8 currentInterfacesNum;

    pTmp = myUSB_GetConfigTablePtr(myUSB_configuration - 1u);
    if (pTmp != NULL)
    {
        currentInterfacesNum  = ((const uint8 *) pTmp->p_list)[myUSB_CONFIG_DESCR_NUM_INTERFACES];
        /* Third entry in the LUT starts the Interface Table pointers */
        /* The INTERFACE_CLASS table is located after all interfaces */
        pTmp = &pTmp[currentInterfacesNum + 2u];
        pInterfaceClass = (const uint8 CYCODE *) pTmp->p_list;
    }
    else
    {
        pInterfaceClass = (const uint8 CYCODE *) NULL;
    }

    return (pInterfaceClass);
}


/*******************************************************************************
* Function Name: myUSB_TerminateEP
****************************************************************************//**
*
*  This function terminates the specified USBFS endpoint.
*  This function should be used before endpoint reconfiguration.
*
*  \param ep Contains the data endpoint number.
*
*  \reentrant
*  No.
*
* \sideeffect
* 
* The device responds with a NAK for any transactions on the selected endpoint.
*   
*******************************************************************************/
void myUSB_TerminateEP(uint8 epNumber) 
{
    /* Get endpoint number */
    epNumber &= myUSB_DIR_UNUSED;

    if ((epNumber > myUSB_EP0) && (epNumber < myUSB_MAX_EP))
    {
        /* Set the endpoint Halt */
        myUSB_EP[epNumber].hwEpState |= myUSB_ENDPOINT_STATUS_HALT;

        /* Clear the data toggle */
        myUSB_EP[epNumber].epToggle = 0u;
        myUSB_EP[epNumber].apiEpState = myUSB_NO_EVENT_ALLOWED;

        if ((myUSB_EP[epNumber].addr & myUSB_DIR_IN) != 0u)
        {   
            /* IN Endpoint */
            myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_MODE_NAK_IN;
        }
        else
        {
            /* OUT Endpoint */
            myUSB_SIE_EP_BASE.sieEp[epNumber].epCr0 = myUSB_MODE_NAK_OUT;
        }
    }
}


/*******************************************************************************
* Function Name: myUSB_SetEndpointHalt
****************************************************************************//**
*
*  This routine handles set endpoint halt.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_SetEndpointHalt(void) 
{
    uint8 requestHandled = myUSB_FALSE;
    uint8 ep;
    
    /* Set endpoint halt */
    ep = myUSB_wIndexLoReg & myUSB_DIR_UNUSED;

    if ((ep > myUSB_EP0) && (ep < myUSB_MAX_EP))
    {
        /* Set the endpoint Halt */
        myUSB_EP[ep].hwEpState |= (myUSB_ENDPOINT_STATUS_HALT);

        /* Clear the data toggle */
        myUSB_EP[ep].epToggle = 0u;
        myUSB_EP[ep].apiEpState |= myUSB_NO_EVENT_ALLOWED;

        if ((myUSB_EP[ep].addr & myUSB_DIR_IN) != 0u)
        {
            /* IN Endpoint */
            myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = (myUSB_MODE_STALL_DATA_EP | 
                                                            myUSB_MODE_ACK_IN);
        }
        else
        {
            /* OUT Endpoint */
            myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = (myUSB_MODE_STALL_DATA_EP | 
                                                            myUSB_MODE_ACK_OUT);
        }
        requestHandled = myUSB_InitNoDataControlTransfer();
    }

    return (requestHandled);
}


/*******************************************************************************
* Function Name: myUSB_ClearEndpointHalt
****************************************************************************//**
*
*  This routine handles clear endpoint halt.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_ClearEndpointHalt(void) 
{
    uint8 requestHandled = myUSB_FALSE;
    uint8 ep;

    /* Clear endpoint halt */
    ep = myUSB_wIndexLoReg & myUSB_DIR_UNUSED;

    if ((ep > myUSB_EP0) && (ep < myUSB_MAX_EP))
    {
        /* Clear the endpoint Halt */
        myUSB_EP[ep].hwEpState &= (uint8) ~myUSB_ENDPOINT_STATUS_HALT;

        /* Clear the data toggle */
        myUSB_EP[ep].epToggle = 0u;
        
        /* Clear toggle bit for already armed packet */
        myUSB_SIE_EP_BASE.sieEp[ep].epCnt0 &= (uint8) ~(uint8)myUSB_EPX_CNT_DATA_TOGGLE;
        
        /* Return API State as it was defined before */
        myUSB_EP[ep].apiEpState &= (uint8) ~myUSB_NO_EVENT_ALLOWED;

        if ((myUSB_EP[ep].addr & myUSB_DIR_IN) != 0u)
        {
            /* IN Endpoint */
            if(myUSB_EP[ep].apiEpState == myUSB_IN_BUFFER_EMPTY)
            {       
                /* Wait for next packet from application */
                myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = myUSB_MODE_NAK_IN;
            }
            else    /* Continue armed transfer */
            {
                myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = myUSB_MODE_ACK_IN;
            }
        }
        else
        {
            /* OUT Endpoint */
            if (myUSB_EP[ep].apiEpState == myUSB_OUT_BUFFER_FULL)
            {       
                /* Allow application to read full buffer */
                myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = myUSB_MODE_NAK_OUT;
            }
            else    /* Mark endpoint as empty, so it will be reloaded */
            {
                myUSB_SIE_EP_BASE.sieEp[ep].epCr0 = myUSB_MODE_ACK_OUT;
            }
        }
        
        requestHandled = myUSB_InitNoDataControlTransfer();
    }

    return(requestHandled);
}


/*******************************************************************************
* Function Name: myUSB_ValidateAlternateSetting
****************************************************************************//**
*
*  Validates (and records) a SET INTERFACE request.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_ValidateAlternateSetting(void) 
{
    uint8 requestHandled = myUSB_FALSE;
    
    uint8 interfaceNum;
    uint8 curInterfacesNum;
    const T_myUSB_LUT CYCODE *pTmp;
    
    /* Get interface number from the request. */
    interfaceNum = (uint8) myUSB_wIndexLoReg;
    
    /* Get number of interfaces for current configuration. */
    pTmp = myUSB_GetConfigTablePtr(myUSB_configuration - 1u);
    curInterfacesNum  = ((const uint8 *) pTmp->p_list)[myUSB_CONFIG_DESCR_NUM_INTERFACES];

    /* Validate that interface number is within range. */
    if ((interfaceNum <= curInterfacesNum) || (interfaceNum <= myUSB_MAX_INTERFACES_NUMBER))
    {
        /* Save current and new alternate settings (come with request) to make 
        * desicion about following endpoint re-configuration.
        */
        myUSB_interfaceSettingLast[interfaceNum] = myUSB_interfaceSetting[interfaceNum];
        myUSB_interfaceSetting[interfaceNum]     = (uint8) myUSB_wValueLoReg;
        
        requestHandled = myUSB_TRUE;
    }

    return (requestHandled);
}


/* [] END OF FILE */
