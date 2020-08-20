/***************************************************************************//**
* \file myUSB_cdc.c
* \version 3.20
*
* \brief
*  This file contains the USB MSC Class request handler and global API for MSC 
*  class.
*
* Related Document:
*  Universal Serial Bus Class Definitions for Communication Devices Version 1.1
*
********************************************************************************
* \copyright
* Copyright 2012-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myUSB_msc.h"
#include "myUSB_pvt.h"
#include "cyapicallbacks.h"

#if (myUSB_HANDLE_MSC_REQUESTS)

/***************************************
*          Internal variables
***************************************/

static uint8 myUSB_lunCount = myUSB_MSC_LUN_NUMBER;


/*******************************************************************************
* Function Name: myUSB_DispatchMSCClassRqst
****************************************************************************//**
*   
*  \internal 
*  This routine dispatches MSC class requests.
*
* \return
*  Status of request processing: handled or not handled.
*
* \globalvars
*  myUSB_lunCount - stores number of LUN (logical units).
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_DispatchMSCClassRqst(void) 
{
    uint8 requestHandled = myUSB_FALSE;
    
    /* Get request data. */
    uint16 value  = myUSB_GET_UINT16(myUSB_wValueHiReg,  myUSB_wValueLoReg);
    uint16 dataLength = myUSB_GET_UINT16(myUSB_wLengthHiReg, myUSB_wLengthLoReg);
       
    /* Check request direction: D2H or H2D. */
    if (0u != (myUSB_bmRequestTypeReg & myUSB_RQST_DIR_D2H))
    {
        /* Handle direction from device to host. */
        
        if (myUSB_MSC_GET_MAX_LUN == myUSB_bRequestReg)
        {
            /* Check request fields. */
            if ((value  == myUSB_MSC_GET_MAX_LUN_WVALUE) &&
                (dataLength == myUSB_MSC_GET_MAX_LUN_WLENGTH))
            {
                /* Reply to Get Max LUN request: setup control read. */
                myUSB_currentTD.pData = &myUSB_lunCount;
                myUSB_currentTD.count =  myUSB_MSC_GET_MAX_LUN_WLENGTH;
                
                requestHandled  = myUSB_InitControlRead();
            }
        }
    }
    else
    {
        /* Handle direction from host to device. */
        
        if (myUSB_MSC_RESET == myUSB_bRequestReg)
        {
            /* Check request fields. */
            if ((value  == myUSB_MSC_RESET_WVALUE) &&
                (dataLength == myUSB_MSC_RESET_WLENGTH))
            {
                /* Handle to Bulk-Only Reset request: no data control transfer. */
                myUSB_currentTD.count = myUSB_MSC_RESET_WLENGTH;
                
            #ifdef myUSB_DISPATCH_MSC_CLASS_MSC_RESET_RQST_CALLBACK
                myUSB_DispatchMSCClass_MSC_RESET_RQST_Callback();
            #endif /* (myUSB_DISPATCH_MSC_CLASS_MSC_RESET_RQST_CALLBACK) */
                
                requestHandled = myUSB_InitNoDataControlTransfer();
            }
        }
    }
    
    return (requestHandled);
}


/*******************************************************************************
* Function Name: myUSB_MSC_SetLunCount
****************************************************************************//**
*
*  This function sets the number of logical units supported in the application. 
*  The default number of logical units is set in the component customizer.
*
*  \param lunCount: Count of the logical units. Valid range is between 1 and 16.
*
*
* \globalvars
*  myUSB_lunCount - stores number of LUN (logical units).
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_MSC_SetLunCount(uint8 lunCount) 
{
    myUSB_lunCount = (lunCount - 1u);
}


/*******************************************************************************
* Function Name: myUSB_MSC_GetLunCount
****************************************************************************//**
*
*  This function returns the number of logical units.
*
* \return
*   Number of the logical units.
*
* \globalvars
*  myUSB_lunCount - stores number of LUN (logical units).
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_MSC_GetLunCount(void) 
{
    return (myUSB_lunCount + 1u);
}   

#endif /* (myUSB_HANDLE_MSC_REQUESTS) */


/* [] END OF FILE */
