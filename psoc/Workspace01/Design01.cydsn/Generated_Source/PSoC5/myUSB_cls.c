/***************************************************************************//**
* \file myUSB_cls.c
* \version 3.20
*
* \brief
*  This file contains the USB Class request handler.
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

#if(myUSB_EXTERN_CLS == myUSB_FALSE)

/***************************************
* User Implemented Class Driver Declarations.
***************************************/
/* `#START USER_DEFINED_CLASS_DECLARATIONS` Place your declaration here */

/* `#END` */


/*******************************************************************************
* Function Name: myUSB_DispatchClassRqst
****************************************************************************//**
*  This routine dispatches class specific requests depend on interface class.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_DispatchClassRqst(void) 
{
    uint8 interfaceNumber;
    uint8 requestHandled = myUSB_FALSE;

    /* Get interface to which request is intended. */
    switch (myUSB_bmRequestTypeReg & myUSB_RQST_RCPT_MASK)
    {
        case myUSB_RQST_RCPT_IFC:
            /* Class-specific request directed to interface: wIndexLoReg 
            * contains interface number.
            */
            interfaceNumber = (uint8) myUSB_wIndexLoReg;
            break;
        
        case myUSB_RQST_RCPT_EP:
            /* Class-specific request directed to endpoint: wIndexLoReg contains 
            * endpoint number. Find interface related to endpoint. 
            */
            interfaceNumber = myUSB_EP[myUSB_wIndexLoReg & myUSB_DIR_UNUSED].interface;
            break;
            
        default:
            /* Default interface is zero. */
            interfaceNumber = 0u;
            break;
    }
    
    /* Check that interface is within acceptable range */
    if (interfaceNumber <= myUSB_MAX_INTERFACES_NUMBER)
    {
    #if (defined(myUSB_ENABLE_HID_CLASS)   || \
         defined(myUSB_ENABLE_AUDIO_CLASS) || \
         defined(myUSB_ENABLE_CDC_CLASS)   || \
         myUSB_ENABLE_MSC_CLASS)

        /* Handle class request depends on interface type. */
        switch (myUSB_interfaceClass[interfaceNumber])
        {
        #if defined(myUSB_ENABLE_HID_CLASS)
            case myUSB_CLASS_HID:
                requestHandled = myUSB_DispatchHIDClassRqst();
                break;
        #endif /* (myUSB_ENABLE_HID_CLASS) */
                
        #if defined(myUSB_ENABLE_AUDIO_CLASS)
            case myUSB_CLASS_AUDIO:
                requestHandled = myUSB_DispatchAUDIOClassRqst();
                break;
        #endif /* (myUSB_CLASS_AUDIO) */
                
        #if defined(myUSB_ENABLE_CDC_CLASS)
            case myUSB_CLASS_CDC:
                requestHandled = myUSB_DispatchCDCClassRqst();
                break;
        #endif /* (myUSB_ENABLE_CDC_CLASS) */
            
        #if (myUSB_ENABLE_MSC_CLASS)
            case myUSB_CLASS_MSD:
            #if (myUSB_HANDLE_MSC_REQUESTS)
                /* MSC requests are handled by the component. */
                requestHandled = myUSB_DispatchMSCClassRqst();
            #elif defined(myUSB_DISPATCH_MSC_CLASS_RQST_CALLBACK)
                /* MSC requests are handled by user defined callbcak. */
                requestHandled = myUSB_DispatchMSCClassRqst_Callback();
            #else
                /* MSC requests are not handled. */
                requestHandled = myUSB_FALSE;
            #endif /* (myUSB_HANDLE_MSC_REQUESTS) */
                break;
        #endif /* (myUSB_ENABLE_MSC_CLASS) */
            
            default:
                /* Request is not handled: unknown class request type. */
                requestHandled = myUSB_FALSE;
                break;
        }
    #endif /* Class support is enabled */
    }
    
    /* `#START USER_DEFINED_CLASS_CODE` Place your Class request here */

    /* `#END` */

#ifdef myUSB_DISPATCH_CLASS_RQST_CALLBACK
    if (myUSB_FALSE == requestHandled)
    {
        requestHandled = myUSB_DispatchClassRqst_Callback(interfaceNumber);
    }
#endif /* (myUSB_DISPATCH_CLASS_RQST_CALLBACK) */

    return (requestHandled);
}


/*******************************************************************************
* Additional user functions supporting Class Specific Requests
********************************************************************************/

/* `#START CLASS_SPECIFIC_FUNCTIONS` Place any additional functions here */

/* `#END` */

#endif /* myUSB_EXTERN_CLS */


/* [] END OF FILE */
