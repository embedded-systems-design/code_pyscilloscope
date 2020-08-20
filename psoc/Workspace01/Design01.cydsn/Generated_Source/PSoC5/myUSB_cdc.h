/***************************************************************************//**
* \file myUSB_cdc.h
* \version 3.20
*
* \brief
*  This file provides function prototypes and constants for the USBFS component 
*  CDC class.
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

#if !defined(CY_USBFS_myUSB_cdc_H)
#define CY_USBFS_myUSB_cdc_H

#include "myUSB.h"


/*******************************************************************************
* Prototypes of the myUSB_cdc API.
*******************************************************************************/
/**
* \addtogroup group_cdc
* @{
*/
#if (myUSB_ENABLE_CDC_CLASS_API != 0u)
    uint8 myUSB_CDC_Init(void)            ;
    void myUSB_PutData(const uint8* pData, uint16 length) ;
    void myUSB_PutString(const char8 string[])            ;
    void myUSB_PutChar(char8 txDataByte) ;
    void myUSB_PutCRLF(void)             ;
    uint16 myUSB_GetCount(void)          ;
    uint8  myUSB_CDCIsReady(void)        ;
    uint8  myUSB_DataIsReady(void)       ;
    uint16 myUSB_GetData(uint8* pData, uint16 length)     ;
    uint16 myUSB_GetAll(uint8* pData)    ;
    uint8  myUSB_GetChar(void)           ;
    uint8  myUSB_IsLineChanged(void)     ;
    uint32 myUSB_GetDTERate(void)        ;
    uint8  myUSB_GetCharFormat(void)     ;
    uint8  myUSB_GetParityType(void)     ;
    uint8  myUSB_GetDataBits(void)       ;
    uint16 myUSB_GetLineControl(void)    ;
    void myUSB_SendSerialState (uint16 serialState) ;
    uint16 myUSB_GetSerialState (void)   ;
    void myUSB_SetComPort (uint8 comNumber) ;
    uint8 myUSB_GetComPort (void)        ;
    uint8 myUSB_NotificationIsReady(void) ;

#endif  /* (myUSB_ENABLE_CDC_CLASS_API) */
/** @} cdc */

/*******************************************************************************
*  Constants for myUSB_cdc API.
*******************************************************************************/

/* CDC Class-Specific Request Codes (CDC ver 1.2 Table 19) */
#define myUSB_CDC_SET_LINE_CODING        (0x20u)
#define myUSB_CDC_GET_LINE_CODING        (0x21u)
#define myUSB_CDC_SET_CONTROL_LINE_STATE (0x22u)

/*PSTN Subclass Specific Notifications (CDC ver 1.2 Table 30)*/
#define myUSB_SERIAL_STATE               (0x20u)

#define myUSB_LINE_CODING_CHANGED        (0x01u)
#define myUSB_LINE_CONTROL_CHANGED       (0x02u)

#define myUSB_1_STOPBIT                  (0x00u)
#define myUSB_1_5_STOPBITS               (0x01u)
#define myUSB_2_STOPBITS                 (0x02u)

#define myUSB_PARITY_NONE                (0x00u)
#define myUSB_PARITY_ODD                 (0x01u)
#define myUSB_PARITY_EVEN                (0x02u)
#define myUSB_PARITY_MARK                (0x03u)
#define myUSB_PARITY_SPACE               (0x04u)

#define myUSB_LINE_CODING_SIZE           (0x07u)
#define myUSB_LINE_CODING_RATE           (0x00u)
#define myUSB_LINE_CODING_STOP_BITS      (0x04u)
#define myUSB_LINE_CODING_PARITY         (0x05u)
#define myUSB_LINE_CODING_DATA_BITS      (0x06u)

#define myUSB_LINE_CONTROL_DTR           (0x01u)
#define myUSB_LINE_CONTROL_RTS           (0x02u)

#define myUSB_MAX_MULTI_COM_NUM          (2u) 

#define myUSB_COM_PORT1                  (0u) 
#define myUSB_COM_PORT2                  (1u) 

#define myUSB_SUCCESS                    (0u)
#define myUSB_FAILURE                    (1u)

#define myUSB_SERIAL_STATE_SIZE          (10u)

/* SerialState constants*/
#define myUSB_SERIAL_STATE_REQUEST_TYPE  (0xA1u)
#define myUSB_SERIAL_STATE_LENGTH        (0x2u)

/*******************************************************************************
* External data references
*******************************************************************************/
/**
* \addtogroup group_cdc
* @{
*/
extern volatile uint8  myUSB_linesCoding[myUSB_MAX_MULTI_COM_NUM][myUSB_LINE_CODING_SIZE];
extern volatile uint8  myUSB_linesChanged[myUSB_MAX_MULTI_COM_NUM];
extern volatile uint16 myUSB_linesControlBitmap[myUSB_MAX_MULTI_COM_NUM];
extern volatile uint16 myUSB_serialStateBitmap[myUSB_MAX_MULTI_COM_NUM];
extern volatile uint8  myUSB_cdcDataInEp[myUSB_MAX_MULTI_COM_NUM];
extern volatile uint8  myUSB_cdcDataOutEp[myUSB_MAX_MULTI_COM_NUM];
extern volatile uint8  myUSB_cdcCommInInterruptEp[myUSB_MAX_MULTI_COM_NUM];
/** @} cdc */

/*******************************************************************************
* The following code is DEPRECATED and
* must not be used.
*******************************************************************************/


#define myUSB_lineCoding             myUSB_linesCoding[0]
#define myUSB_lineChanged            myUSB_linesChanged[0]
#define myUSB_lineControlBitmap      myUSB_linesControlBitmap[0]
#define myUSB_cdc_data_in_ep         myUSB_cdcDataInEp[0]
#define myUSB_cdc_data_out_ep        myUSB_cdcDataOutEp[0]

#endif /* (CY_USBFS_myUSB_cdc_H) */


/* [] END OF FILE */
