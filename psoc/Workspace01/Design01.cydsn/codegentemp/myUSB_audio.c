/***************************************************************************//**
* \file myUSB_audio.c
* \version 3.20
*
* \brief
*  This file contains the USB AUDIO Class request handler.
*
* Related Document:
*  Universal Serial Bus Device Class Definition for Audio Devices Release 1.0
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myUSB_audio.h"
#include "myUSB_pvt.h"
#include "cyapicallbacks.h"

#if defined(myUSB_ENABLE_AUDIO_CLASS)

/***************************************
* Custom Declarations
***************************************/

/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */


#if !defined(USER_SUPPLIED_AUDIO_HANDLER)

/***************************************
*    AUDIO Variables
***************************************/

#if defined(myUSB_ENABLE_AUDIO_STREAMING)
    /** Contains the current audio sample frequency. It is set by the host using a SET_CUR request to the endpoint.*/
    volatile uint8 myUSB_currentSampleFrequency[myUSB_MAX_EP][myUSB_SAMPLE_FREQ_LEN];
    /** Used as a flag for the user code, to inform it that the host has been sent a request 
     * to change the sample frequency. The sample frequency will be sent on the next OUT transaction. 
     * It contains the endpoint address when set. The following code is recommended for 
     * detecting new sample frequency in main code:
     * \snippet  /USBFS_sut_02.cydsn/main.c Detecting new Sample Frequency
     *     
     * The USBFS_transferState variable is checked to make sure that the transfer completes. */
    volatile uint8 myUSB_frequencyChanged;
    /** Contains the mute configuration set by the host.*/
    volatile uint8 myUSB_currentMute;
    /** Contains the volume level set by the host.*/
    volatile uint8 myUSB_currentVolume[myUSB_VOLUME_LEN];
    volatile uint8 myUSB_minimumVolume[myUSB_VOLUME_LEN] = {myUSB_VOL_MIN_LSB,
                                                                                  myUSB_VOL_MIN_MSB};
    volatile uint8 myUSB_maximumVolume[myUSB_VOLUME_LEN] = {myUSB_VOL_MAX_LSB,
                                                                                  myUSB_VOL_MAX_MSB};
    volatile uint8 myUSB_resolutionVolume[myUSB_VOLUME_LEN] = {myUSB_VOL_RES_LSB,
                                                                                     myUSB_VOL_RES_MSB};
#endif /*  myUSB_ENABLE_AUDIO_STREAMING */


/*******************************************************************************
* Function Name: myUSB_DispatchAUDIOClassRqst
****************************************************************************//**
*
*  This routine dispatches class requests
*
* \return
*  Results of Audio Class request handling: 
*  - myUSB_TRUE  - request was handled without errors
*  - myUSB_FALSE - error occurs during handling of request     
*
* \globalvars
*   myUSB_currentSampleFrequency: Contains the current audio Sample
*       Frequency. It is set by the Host using SET_CUR request to the endpoint.
*   myUSB_frequencyChanged: This variable is used as a flag for the
*       user code, to be aware that Host has been sent request for changing
*       Sample Frequency. Sample frequency will be sent on the next OUT
*       transaction. It is contains endpoint address when set. The following
*       code is recommended for detecting new Sample Frequency in main code:
*       
*  \snippet  /USBFS_sut_02.cydsn/main.c Detecting new Sample Frequency
*      
*   myUSB_transferState variable is checked to be sure that transfer
*              completes.
*   myUSB_currentMute: Contains mute configuration set by Host.
*   myUSB_currentVolume: Contains volume level set by Host.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_DispatchAUDIOClassRqst(void) 
{
    uint8 requestHandled = myUSB_FALSE;
    
    uint8 RqstRcpt = (uint8)(myUSB_bmRequestTypeReg & myUSB_RQST_RCPT_MASK);
#if defined(myUSB_ENABLE_AUDIO_STREAMING)
    uint8 wValueHi = (uint8) myUSB_wValueHiReg;
    uint8 epNumber = (uint8) myUSB_wIndexLoReg & myUSB_DIR_UNUSED;
#endif /* (myUSB_ENABLE_AUDIO_STREAMING) */
    
    /* Check request direction: D2H or H2D. */
    if (0u != (myUSB_bmRequestTypeReg & myUSB_RQST_DIR_D2H))
    {
        /* Handle direction from device to host. */
        
        if (myUSB_RQST_RCPT_EP == RqstRcpt)
        {
            /* Request recipient is to endpoint. */
            switch (myUSB_bRequestReg)
            {
                case myUSB_GET_CUR:
                #if defined(myUSB_ENABLE_AUDIO_STREAMING)
                    if (wValueHi == myUSB_SAMPLING_FREQ_CONTROL)
                    {
                         /* point Control Selector is Sampling Frequency */
                        myUSB_currentTD.wCount = myUSB_SAMPLE_FREQ_LEN;
                        myUSB_currentTD.pData  = myUSB_currentSampleFrequency[epNumber];
                        
                        requestHandled   = myUSB_InitControlRead();
                    }
                #endif /* (myUSB_ENABLE_AUDIO_STREAMING) */
                
                    /* `#START AUDIO_READ_REQUESTS` Place other request handler here */

                    /* `#END` */
                
                #ifdef myUSB_DISPATCH_AUDIO_CLASS_AUDIO_READ_REQUESTS_CALLBACK    
                    myUSB_DispatchAUDIOClass_AUDIO_READ_REQUESTS_Callback();
                #endif /* (myUSB_DISPATCH_AUDIO_CLASS_AUDIO_READ_REQUESTS_CALLBACK) */                   
                break;
                
                default:
                    /* Do not handle this request unless callback is defined. */
                    break;
            }
        
        }
        else if (myUSB_RQST_RCPT_IFC == RqstRcpt)
        {
            /* Request recipient is interface or entity ID. */
            switch (myUSB_bRequestReg)
            {
                case myUSB_GET_CUR:
                #if defined(myUSB_ENABLE_AUDIO_STREAMING)
                    if (wValueHi == myUSB_MUTE_CONTROL)
                    {
                        /* `#START MUTE_CONTROL_GET_REQUEST` Place multi-channel handler here */

                        /* `#END` */

                    #ifdef myUSB_DISPATCH_AUDIO_CLASS_MUTE_CONTROL_GET_REQUEST_CALLBACK
                        myUSB_DispatchAUDIOClass_MUTE_CONTROL_GET_REQUEST_Callback();
                    #endif /* (myUSB_DISPATCH_AUDIO_CLASS_MUTE_CONTROL_GET_REQUEST_CALLBACK) */

                        /* Entity ID Control Selector is MUTE */
                        myUSB_currentTD.wCount = 1u;
                        myUSB_currentTD.pData  = &myUSB_currentMute;
                        
                        requestHandled = myUSB_InitControlRead();
                    }
                    else if (wValueHi == myUSB_VOLUME_CONTROL)
                    {
                        /* `#START VOLUME_CONTROL_GET_REQUEST` Place multi-channel handler here */

                        /* `#END` */

                    #ifdef myUSB_DISPATCH_AUDIO_CLASS_VOLUME_CONTROL_GET_REQUEST_CALLBACK
                        myUSB_DispatchAUDIOClass_VOLUME_CONTROL_GET_REQUEST_Callback();
                    #endif /* (myUSB_DISPATCH_AUDIO_CLASS_VOLUME_CONTROL_GET_REQUEST_CALLBACK) */

                        /* Entity ID Control Selector is VOLUME, */
                        myUSB_currentTD.wCount = myUSB_VOLUME_LEN;
                        myUSB_currentTD.pData  = myUSB_currentVolume;
                        
                        requestHandled = myUSB_InitControlRead();
                    }
                    else
                    {
                        /* `#START OTHER_GET_CUR_REQUESTS` Place other request handler here */

                        /* `#END` */

                    #ifdef myUSB_DISPATCH_AUDIO_CLASS_OTHER_GET_CUR_REQUESTS_CALLBACK
                        myUSB_DispatchAUDIOClass_OTHER_GET_CUR_REQUESTS_Callback();
                    #endif /* (myUSB_DISPATCH_AUDIO_CLASS_OTHER_GET_CUR_REQUESTS_CALLBACK) */
                    }
                    break;
                    
                case myUSB_GET_MIN:
                    if (wValueHi == myUSB_VOLUME_CONTROL)
                    {
                        /* Entity ID Control Selector is VOLUME, */
                        myUSB_currentTD.wCount = myUSB_VOLUME_LEN;
                        myUSB_currentTD.pData  = &myUSB_minimumVolume[0];
                        
                        requestHandled = myUSB_InitControlRead();
                    }
                    break;
                    
                case myUSB_GET_MAX:
                    if (wValueHi == myUSB_VOLUME_CONTROL)
                    {
                        /* Entity ID Control Selector is VOLUME, */
                        myUSB_currentTD.wCount = myUSB_VOLUME_LEN;
                        myUSB_currentTD.pData  = &myUSB_maximumVolume[0];
                        
                        requestHandled = myUSB_InitControlRead();
                    }
                    break;
                    
                case myUSB_GET_RES:
                    if (wValueHi == myUSB_VOLUME_CONTROL)
                    {
                         /* Entity ID Control Selector is VOLUME, */
                        myUSB_currentTD.wCount = myUSB_VOLUME_LEN;
                        myUSB_currentTD.pData  = &myUSB_resolutionVolume[0];
                        
                        requestHandled   = myUSB_InitControlRead();
                    }
                    break;
                    
                /* The contents of the status message is reserved for future use.
                * For the time being, a null packet should be returned in the data stage of the
                * control transfer, and the received null packet should be ACKed.
                */
                case myUSB_GET_STAT:
                    myUSB_currentTD.wCount = 0u;    
                    
                    requestHandled = myUSB_InitControlWrite();

                #endif /* (myUSB_ENABLE_AUDIO_STREAMING) */
                
                    /* `#START AUDIO_WRITE_REQUESTS` Place other request handler here */

                    /* `#END` */
                
                #ifdef myUSB_DISPATCH_AUDIO_CLASS_AUDIO_WRITE_REQUESTS_CALLBACK
                    myUSB_DispatchAUDIOClass_AUDIO_WRITE_REQUESTS_Callback();
                #endif /* (myUSB_DISPATCH_AUDIO_CLASS_AUDIO_WRITE_REQUESTS_CALLBACK) */
                    break;
                
                default:
                    /* Do not handle this request. */
                    break;
            }
        }
        else
        {   
            /* Do not handle other requests recipients. */
        }
    }
    else
    {
        /* Handle direction from host to device. */
        
        if (myUSB_RQST_RCPT_EP == RqstRcpt)
        {
            /* Request recipient is endpoint. */
            switch (myUSB_bRequestReg)
            {
                case myUSB_SET_CUR:
                #if defined(myUSB_ENABLE_AUDIO_STREAMING)
                    if (wValueHi == myUSB_SAMPLING_FREQ_CONTROL)
                    {
                         /* point Control Selector is Sampling Frequency */
                        myUSB_currentTD.wCount = myUSB_SAMPLE_FREQ_LEN;
                        myUSB_currentTD.pData  = myUSB_currentSampleFrequency[epNumber];
                        myUSB_frequencyChanged = (uint8) epNumber;
                        
                        requestHandled   = myUSB_InitControlWrite();
                    }
                #endif /* (myUSB_ENABLE_AUDIO_STREAMING) */

                    /* `#START AUDIO_SAMPLING_FREQ_REQUESTS` Place other request handler here */

                    /* `#END` */

                #ifdef myUSB_DISPATCH_AUDIO_CLASS_AUDIO_SAMPLING_FREQ_REQUESTS_CALLBACK
                    myUSB_DispatchAUDIOClass_AUDIO_SAMPLING_FREQ_REQUESTS_Callback();
                #endif /* (myUSB_DISPATCH_AUDIO_CLASS_AUDIO_SAMPLING_FREQ_REQUESTS_CALLBACK) */
                    break;
                
                default:
                    /* Do not handle this request. */
                    break;
            }
        }
        else if(myUSB_RQST_RCPT_IFC == RqstRcpt)
        {
            /* Request recipient is interface or entity ID. */
            switch (myUSB_bRequestReg)
            {
                case myUSB_SET_CUR:
                #if defined(myUSB_ENABLE_AUDIO_STREAMING)
                    if (wValueHi == myUSB_MUTE_CONTROL)
                    {
                        /* `#START MUTE_SET_REQUEST` Place multi-channel handler here */

                        /* `#END` */

                    #ifdef myUSB_DISPATCH_AUDIO_CLASS_MUTE_SET_REQUEST_CALLBACK
                        myUSB_DispatchAUDIOClass_MUTE_SET_REQUEST_Callback();
                    #endif /* (myUSB_DISPATCH_AUDIO_CLASS_MUTE_SET_REQUEST_CALLBACK) */

                        /* Entity ID Control Selector is MUTE */
                        myUSB_currentTD.wCount = 1u;
                        myUSB_currentTD.pData  = &myUSB_currentMute;
                        
                        requestHandled = myUSB_InitControlWrite();
                    }
                    else if (wValueHi == myUSB_VOLUME_CONTROL)
                    {
                        /* `#START VOLUME_CONTROL_SET_REQUEST` Place multi-channel handler here */

                        /* `#END` */

                    #ifdef myUSB_DISPATCH_AUDIO_CLASS_VOLUME_CONTROL_SET_REQUEST_CALLBACK
                        myUSB_DispatchAUDIOClass_VOLUME_CONTROL_SET_REQUEST_Callback();
                    #endif /* (myUSB_DISPATCH_AUDIO_CLASS_VOLUME_CONTROL_SET_REQUEST_CALLBACK) */

                        /* Entity ID Control Selector is VOLUME */
                        myUSB_currentTD.wCount = myUSB_VOLUME_LEN;
                        myUSB_currentTD.pData  = myUSB_currentVolume;
                        
                        requestHandled = myUSB_InitControlWrite();
                    }
                    else
                    {
                        /* `#START OTHER_SET_CUR_REQUESTS` Place other request handler here */

                        /* `#END` */

                    #ifdef myUSB_DISPATCH_AUDIO_CLASS_OTHER_SET_CUR_REQUESTS_CALLBACK
                        myUSB_DispatchAUDIOClass_OTHER_SET_CUR_REQUESTS_Callback();
                    #endif /* (myUSB_DISPATCH_AUDIO_CLASS_OTHER_SET_CUR_REQUESTS_CALLBACK) */
                    }
                #endif /*  myUSB_ENABLE_AUDIO_STREAMING */
                
                
                    /* `#START AUDIO_CONTROL_SEL_REQUESTS` Place other request handler here */

                    /* `#END` */
                    
                #ifdef myUSB_DISPATCH_AUDIO_CLASS_AUDIO_CONTROL_SEL_REQUESTS_CALLBACK
                    myUSB_DispatchAUDIOClass_AUDIO_CONTROL_SEL_REQUESTS_Callback();
                #endif /* (myUSB_DISPATCH_AUDIO_CLASS_AUDIO_CONTROL_SEL_REQUESTS_CALLBACK) */
                break;

                default:
                    /* Do not handle this request. */
                break;
            }
        }
        else
        {
            /* Do not handle other requests recipients. */
        }
    }

    return (requestHandled);
}
#endif /* (USER_SUPPLIED_AUDIO_HANDLER) */


/*******************************************************************************
* Additional user functions supporting AUDIO Requests
********************************************************************************/

/* `#START AUDIO_FUNCTIONS` Place any additional functions here */

/* `#END` */

#endif  /* (myUSB_ENABLE_AUDIO_CLASS) */


/* [] END OF FILE */
