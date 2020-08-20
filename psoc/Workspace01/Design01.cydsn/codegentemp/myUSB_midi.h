/***************************************************************************//**
* \file myUSB_midi.h
* \version 3.20
*
* \brief
*  This file provides function prototypes and constants for the USBFS component 
*  MIDI class support.
*
* Related Document:
*  Universal Serial Bus Device Class Definition for MIDI Devices Release 1.0
*  MIDI 1.0 Detailed Specification Document Version 4.2
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_USBFS_myUSB_midi_H)
#define CY_USBFS_myUSB_midi_H

#include "myUSB.h"

/***************************************
*    Initial Parameter Constants
***************************************/

#define myUSB_ENABLE_MIDI_API    (0u != (1u))
#define myUSB_MIDI_EXT_MODE      (0u)


/* Number of external interfaces (UARTs). */
#define myUSB_ONE_EXT_INTRF  (0x01u)
#define myUSB_TWO_EXT_INTRF  (0x02u)
    
#define myUSB_ISR_SERVICE_MIDI_OUT \
            ((myUSB_ENABLE_MIDI_API != 0u) && (myUSB_MIDI_OUT_BUFF_SIZE > 0) && \
             (myUSB_EP_MANAGEMENT_DMA_AUTO))

#define myUSB_ISR_SERVICE_MIDI_IN \
            ((myUSB_ENABLE_MIDI_API != 0u) && (myUSB_MIDI_IN_BUFF_SIZE > 0))


/***************************************
*    External References
***************************************/

#if (myUSB_MIDI_EXT_MODE >= myUSB_ONE_EXT_INTRF)
    #include "MIDI1_UART.h"
#endif /* (myUSB_MIDI_EXT_MODE >= myUSB_ONE_EXT_INTRF) */

#if (myUSB_MIDI_EXT_MODE >= myUSB_TWO_EXT_INTRF)
    #include "MIDI2_UART.h"
#endif /* (myUSB_MIDI_EXT_MODE >= myUSB_TWO_EXT_INTRF) */


/***************************************
*    Data Structure Definition
***************************************/

/* The following structure is used to hold status information for
* building and parsing incoming MIDI messages. 
*/
typedef struct
{
    uint8    length;        /* expected length */
    uint8    count;         /* current byte count */
    uint8    size;          /* complete size */
    uint8    runstat;       /* running status */
    uint8    msgBuff[4u];   /* message buffer */
} myUSB_MIDI_RX_STATUS;


/***************************************
*       Function Prototypes
***************************************/
/**
* \addtogroup group_midi
* @{
*/
#if defined(myUSB_ENABLE_MIDI_STREAMING) && (myUSB_ENABLE_MIDI_API != 0u)
    void myUSB_MIDI_Init(void) ;

    #if (myUSB_MIDI_IN_BUFF_SIZE > 0u)
        void myUSB_MIDI_IN_Service(void) ;
        uint8 myUSB_PutUsbMidiIn(uint8 ic, const uint8 midiMsg[], uint8 cable) ;
    #endif /* (myUSB_MIDI_IN_BUFF_SIZE > 0u) */

    #if (myUSB_MIDI_OUT_BUFF_SIZE > 0u)
        void myUSB_MIDI_OUT_Service(void) ;
    #endif /* (myUSB_MIDI_OUT_BUFF_SIZE > 0u) */
#endif /*  (myUSB_ENABLE_MIDI_API != 0u) */


/*******************************************************************************
*   Callback Function Prototypes
*******************************************************************************/

/*******************************************************************************
* Function Name: myUSB_callbackLocalMidiEvent
****************************************************************************//**
*
*  This is a callback function that locally processes data received from the PC 
*  in main.c. You should implement this function if you want to use it. It is 
*  called from the USB output processing routine for each MIDI output event 
*  processed (decoded) from the output endpoint buffer. 
*
*  \param cable: Cable number
*  
*  \param midiMsg: Pointer to the 3-byte MIDI message
*
*
***************************************************************************/
void myUSB_callbackLocalMidiEvent(uint8 cable, uint8 *midiMsg)
                                                     ;
/** @} midi */
                                                    
/***************************************
*           MIDI Constants.
***************************************/

/* Flag definitions for use with MIDI device inquiry */
#define myUSB_INQ_SYSEX_FLAG             (0x01u)
#define myUSB_INQ_IDENTITY_REQ_FLAG      (0x02u)

/* USB-MIDI Code Index Number Classifications (MIDI Table 4-1) */
#define myUSB_CIN_MASK                   (0x0Fu)
#define myUSB_RESERVED0                  (0x00u)
#define myUSB_RESERVED1                  (0x01u)
#define myUSB_2BYTE_COMMON               (0x02u)
#define myUSB_3BYTE_COMMON               (0x03u)
#define myUSB_SYSEX                      (0x04u)
#define myUSB_1BYTE_COMMON               (0x05u)
#define myUSB_SYSEX_ENDS_WITH1           (0x05u)
#define myUSB_SYSEX_ENDS_WITH2           (0x06u)
#define myUSB_SYSEX_ENDS_WITH3           (0x07u)
#define myUSB_NOTE_OFF                   (0x08u)
#define myUSB_NOTE_ON                    (0x09u)
#define myUSB_POLY_KEY_PRESSURE          (0x0Au)
#define myUSB_CONTROL_CHANGE             (0x0Bu)
#define myUSB_PROGRAM_CHANGE             (0x0Cu)
#define myUSB_CHANNEL_PRESSURE           (0x0Du)
#define myUSB_PITCH_BEND_CHANGE          (0x0Eu)
#define myUSB_SINGLE_BYTE                (0x0Fu)

#define myUSB_CABLE_MASK                 (0xF0u)
#define myUSB_MIDI_CABLE_00              (0x00u)
#define myUSB_MIDI_CABLE_01              (0x10u)

#define myUSB_EVENT_BYTE0                (0x00u)
#define myUSB_EVENT_BYTE1                (0x01u)
#define myUSB_EVENT_BYTE2                (0x02u)
#define myUSB_EVENT_BYTE3                (0x03u)
#define myUSB_EVENT_LENGTH               (0x04u)

#define myUSB_MIDI_STATUS_BYTE_MASK      (0x80u)
#define myUSB_MIDI_STATUS_MASK           (0xF0u)
#define myUSB_MIDI_SINGLE_BYTE_MASK      (0x08u)
#define myUSB_MIDI_NOTE_OFF              (0x80u)
#define myUSB_MIDI_NOTE_ON               (0x90u)
#define myUSB_MIDI_POLY_KEY_PRESSURE     (0xA0u)
#define myUSB_MIDI_CONTROL_CHANGE        (0xB0u)
#define myUSB_MIDI_PROGRAM_CHANGE        (0xC0u)
#define myUSB_MIDI_CHANNEL_PRESSURE      (0xD0u)
#define myUSB_MIDI_PITCH_BEND_CHANGE     (0xE0u)
#define myUSB_MIDI_SYSEX                 (0xF0u)
#define myUSB_MIDI_EOSEX                 (0xF7u)
#define myUSB_MIDI_QFM                   (0xF1u)
#define myUSB_MIDI_SPP                   (0xF2u)
#define myUSB_MIDI_SONGSEL               (0xF3u)
#define myUSB_MIDI_TUNEREQ               (0xF6u)
#define myUSB_MIDI_ACTIVESENSE           (0xFEu)

/* MIDI Universal System Exclusive defines */
#define myUSB_MIDI_SYSEX_NON_REAL_TIME   (0x7Eu)
#define myUSB_MIDI_SYSEX_REALTIME        (0x7Fu)

/* ID of target device */
#define myUSB_MIDI_SYSEX_ID_ALL          (0x7Fu)

/* Sub-ID#1*/
#define myUSB_MIDI_SYSEX_GEN_INFORMATION (0x06u)
#define myUSB_MIDI_SYSEX_GEN_MESSAGE     (0x09u)

/* Sub-ID#2*/
#define myUSB_MIDI_SYSEX_IDENTITY_REQ    (0x01u)
#define myUSB_MIDI_SYSEX_IDENTITY_REPLY  (0x02u)
#define myUSB_MIDI_SYSEX_SYSTEM_ON       (0x01u)
#define myUSB_MIDI_SYSEX_SYSTEM_OFF      (0x02u)

/* UART TX and RX interrupt priority. */
#if (CY_PSOC4)
    #define myUSB_CUSTOM_UART_RX_PRIOR_NUM   (0x01u)
    #define myUSB_CUSTOM_UART_TX_PRIOR_NUM   (0x02u)
#else
    #define myUSB_CUSTOM_UART_RX_PRIOR_NUM   (0x02u)
    #define myUSB_CUSTOM_UART_TX_PRIOR_NUM   (0x04u)
#endif /* (CYPSOC4) */


/***************************************
*    Private Function Prototypes
***************************************/

void myUSB_PrepareInBuffer(uint8 ic, const uint8 srcBuff[], uint8 eventLen, uint8 cable)
                                                                ;
#if (myUSB_MIDI_EXT_MODE >= myUSB_ONE_EXT_INTRF)
    void  myUSB_MIDI_InitInterface(void)             ;
    uint8 myUSB_ProcessMidiIn(uint8 mData, myUSB_MIDI_RX_STATUS *rxStat)
                                                                ;
    uint8 myUSB_MIDI1_GetEvent(void)                 ;
    void  myUSB_MIDI1_ProcessUsbOut(const uint8 epBuf[])
                                                                ;

    #if (myUSB_MIDI_EXT_MODE >= myUSB_TWO_EXT_INTRF)
        uint8 myUSB_MIDI2_GetEvent(void)             ;
        void  myUSB_MIDI2_ProcessUsbOut(const uint8 epBuf[])
                                                                ;
    #endif /* (myUSB_MIDI_EXT_MODE >= myUSB_TWO_EXT_INTRF) */
#endif /* (myUSB_MIDI_EXT_MODE >= myUSB_ONE_EXT_INTRF) */


/***************************************
*     Vars with External Linkage
***************************************/

#if defined(myUSB_ENABLE_MIDI_STREAMING)

#if (myUSB_MIDI_IN_BUFF_SIZE > 0)
    #if (myUSB_MIDI_IN_BUFF_SIZE >= 256)
/**
* \addtogroup group_midi
* @{
*/  
        extern volatile uint16 myUSB_midiInPointer;                       /* Input endpoint buffer pointer */
/** @} midi*/
    #else
        extern volatile uint8 myUSB_midiInPointer;                        /* Input endpoint buffer pointer */
    #endif /* (myUSB_MIDI_IN_BUFF_SIZE >=256) */
/**
* \addtogroup group_midi
* @{
*/  
    extern volatile uint8 myUSB_midi_in_ep;                               /* Input endpoint number */
    extern uint8 myUSB_midiInBuffer[myUSB_MIDI_IN_BUFF_SIZE];  /* Input endpoint buffer */
#endif /* (myUSB_MIDI_IN_BUFF_SIZE > 0) */

#if (myUSB_MIDI_OUT_BUFF_SIZE > 0)
    extern volatile uint8 myUSB_midi_out_ep;                               /* Output endpoint number */
    extern uint8 myUSB_midiOutBuffer[myUSB_MIDI_OUT_BUFF_SIZE]; /* Output endpoint buffer */
#endif /* (myUSB_MIDI_OUT_BUFF_SIZE > 0) */

#if (myUSB_MIDI_EXT_MODE >= myUSB_ONE_EXT_INTRF)
    extern volatile uint8 myUSB_MIDI1_InqFlags;                            /* Device inquiry flag */
    
    #if (myUSB_MIDI_EXT_MODE >= myUSB_TWO_EXT_INTRF)
        extern volatile uint8 myUSB_MIDI2_InqFlags;                        /* Device inquiry flag */
    #endif /* (myUSB_MIDI_EXT_MODE >= myUSB_TWO_EXT_INTRF) */
#endif /* (myUSB_MIDI_EXT_MODE >= myUSB_ONE_EXT_INTRF) */
/** @} midi */
#endif /* (myUSB_ENABLE_MIDI_STREAMING) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

#define myUSB_MIDI_EP_Init           myUSB_MIDI_Init
#define myUSB_MIDI_OUT_EP_Service    myUSB_MIDI_OUT_Service

#endif /* (CY_USBFS_myUSB_midi_H) */


/* [] END OF FILE */
