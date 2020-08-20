/***************************************************************************//**
* \file .h
* \version 3.20
*
* \brief
*  This file provides private function prototypes and constants for the 
*  USBFS component. It is not intended to be used in the user project.
*
********************************************************************************
* \copyright
* Copyright 2013-2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_USBFS_myUSB_pvt_H)
#define CY_USBFS_myUSB_pvt_H

#include "myUSB.h"
   
#ifdef myUSB_ENABLE_AUDIO_CLASS
    #include "myUSB_audio.h"
#endif /* myUSB_ENABLE_AUDIO_CLASS */

#ifdef myUSB_ENABLE_CDC_CLASS
    #include "myUSB_cdc.h"
#endif /* myUSB_ENABLE_CDC_CLASS */

#if (myUSB_ENABLE_MIDI_CLASS)
    #include "myUSB_midi.h"
#endif /* (myUSB_ENABLE_MIDI_CLASS) */

#if (myUSB_ENABLE_MSC_CLASS)
    #include "myUSB_msc.h"
#endif /* (myUSB_ENABLE_MSC_CLASS) */

#if (myUSB_EP_MANAGEMENT_DMA)
    #if (CY_PSOC4)
        #include <CyDMA.h>
    #else
        #include <CyDmac.h>
        #if ((myUSB_EP_MANAGEMENT_DMA_AUTO) && (myUSB_EP_DMA_AUTO_OPT == 0u))
            #include "myUSB_EP_DMA_Done_isr.h"
            #include "myUSB_EP8_DMA_Done_SR.h"
            #include "myUSB_EP17_DMA_Done_SR.h"
        #endif /* ((myUSB_EP_MANAGEMENT_DMA_AUTO) && (myUSB_EP_DMA_AUTO_OPT == 0u)) */
    #endif /* (CY_PSOC4) */
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

#if (myUSB_DMA1_ACTIVE)
    #include "myUSB_ep1_dma.h"
    #define myUSB_EP1_DMA_CH     (myUSB_ep1_dma_CHANNEL)
#endif /* (myUSB_DMA1_ACTIVE) */

#if (myUSB_DMA2_ACTIVE)
    #include "myUSB_ep2_dma.h"
    #define myUSB_EP2_DMA_CH     (myUSB_ep2_dma_CHANNEL)
#endif /* (myUSB_DMA2_ACTIVE) */

#if (myUSB_DMA3_ACTIVE)
    #include "myUSB_ep3_dma.h"
    #define myUSB_EP3_DMA_CH     (myUSB_ep3_dma_CHANNEL)
#endif /* (myUSB_DMA3_ACTIVE) */

#if (myUSB_DMA4_ACTIVE)
    #include "myUSB_ep4_dma.h"
    #define myUSB_EP4_DMA_CH     (myUSB_ep4_dma_CHANNEL)
#endif /* (myUSB_DMA4_ACTIVE) */

#if (myUSB_DMA5_ACTIVE)
    #include "myUSB_ep5_dma.h"
    #define myUSB_EP5_DMA_CH     (myUSB_ep5_dma_CHANNEL)
#endif /* (myUSB_DMA5_ACTIVE) */

#if (myUSB_DMA6_ACTIVE)
    #include "myUSB_ep6_dma.h"
    #define myUSB_EP6_DMA_CH     (myUSB_ep6_dma_CHANNEL)
#endif /* (myUSB_DMA6_ACTIVE) */

#if (myUSB_DMA7_ACTIVE)
    #include "myUSB_ep7_dma.h"
    #define myUSB_EP7_DMA_CH     (myUSB_ep7_dma_CHANNEL)
#endif /* (myUSB_DMA7_ACTIVE) */

#if (myUSB_DMA8_ACTIVE)
    #include "myUSB_ep8_dma.h"
    #define myUSB_EP8_DMA_CH     (myUSB_ep8_dma_CHANNEL)
#endif /* (myUSB_DMA8_ACTIVE) */


/***************************************
*     Private Variables
***************************************/

/* Generated external references for descriptors. */
extern const uint8 CYCODE myUSB_DEVICE0_DESCR[18u];
extern const uint8 CYCODE myUSB_DEVICE0_CONFIGURATION0_DESCR[67u];
extern const T_myUSB_EP_SETTINGS_BLOCK CYCODE myUSB_DEVICE0_CONFIGURATION0_EP_SETTINGS_TABLE[3u];
extern const uint8 CYCODE myUSB_DEVICE0_CONFIGURATION0_INTERFACE_CLASS[2u];
extern const T_myUSB_LUT CYCODE myUSB_DEVICE0_CONFIGURATION0_TABLE[5u];
extern const T_myUSB_LUT CYCODE myUSB_DEVICE0_TABLE[3u];
extern const T_myUSB_LUT CYCODE myUSB_TABLE[1u];
extern const uint8 CYCODE myUSB_SN_STRING_DESCRIPTOR[2];
extern const uint8 CYCODE myUSB_STRING_DESCRIPTORS[159u];


extern const uint8 CYCODE myUSB_MSOS_DESCRIPTOR[myUSB_MSOS_DESCRIPTOR_LENGTH];
extern const uint8 CYCODE myUSB_MSOS_CONFIGURATION_DESCR[myUSB_MSOS_CONF_DESCR_LENGTH];
#if defined(myUSB_ENABLE_IDSN_STRING)
    extern uint8 myUSB_idSerialNumberStringDescriptor[myUSB_IDSN_DESCR_LENGTH];
#endif /* (myUSB_ENABLE_IDSN_STRING) */

extern volatile uint8 myUSB_interfaceNumber;
extern volatile uint8 myUSB_interfaceSetting[myUSB_MAX_INTERFACES_NUMBER];
extern volatile uint8 myUSB_interfaceSettingLast[myUSB_MAX_INTERFACES_NUMBER];
extern volatile uint8 myUSB_deviceAddress;
extern volatile uint8 myUSB_interfaceStatus[myUSB_MAX_INTERFACES_NUMBER];
extern const uint8 CYCODE *myUSB_interfaceClass;

extern volatile T_myUSB_EP_CTL_BLOCK myUSB_EP[myUSB_MAX_EP];
extern volatile T_myUSB_TD myUSB_currentTD;

#if (myUSB_EP_MANAGEMENT_DMA)
    #if (CY_PSOC4)
        extern const uint8 myUSB_DmaChan[myUSB_MAX_EP];
    #else
        extern uint8 myUSB_DmaChan[myUSB_MAX_EP];
        extern uint8 myUSB_DmaTd  [myUSB_MAX_EP];
    #endif /* (CY_PSOC4) */
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

#if (myUSB_EP_MANAGEMENT_DMA_AUTO)
#if (CY_PSOC4)
    extern uint8  myUSB_DmaEpBurstCnt   [myUSB_MAX_EP];
    extern uint8  myUSB_DmaEpLastBurstEl[myUSB_MAX_EP];

    extern uint8  myUSB_DmaEpBurstCntBackup  [myUSB_MAX_EP];
    extern uint32 myUSB_DmaEpBufferAddrBackup[myUSB_MAX_EP];
    
    extern const uint8 myUSB_DmaReqOut     [myUSB_MAX_EP];    
    extern const uint8 myUSB_DmaBurstEndOut[myUSB_MAX_EP];
#else
    #if (myUSB_EP_DMA_AUTO_OPT == 0u)
        extern uint8 myUSB_DmaNextTd[myUSB_MAX_EP];
        extern volatile uint16 myUSB_inLength [myUSB_MAX_EP];
        extern volatile uint8  myUSB_inBufFull[myUSB_MAX_EP];
        extern const uint8 myUSB_epX_TD_TERMOUT_EN[myUSB_MAX_EP];
        extern const uint8 *myUSB_inDataPointer[myUSB_MAX_EP];
    #endif /* (myUSB_EP_DMA_AUTO_OPT == 0u) */
#endif /* CY_PSOC4 */
#endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */

extern volatile uint8 myUSB_ep0Toggle;
extern volatile uint8 myUSB_lastPacketSize;
extern volatile uint8 myUSB_ep0Mode;
extern volatile uint8 myUSB_ep0Count;
extern volatile uint16 myUSB_transferByteCount;


/***************************************
*     Private Function Prototypes
***************************************/
void  myUSB_ReInitComponent(void)            ;
void  myUSB_HandleSetup(void)                ;
void  myUSB_HandleIN(void)                   ;
void  myUSB_HandleOUT(void)                  ;
void  myUSB_LoadEP0(void)                    ;
uint8 myUSB_InitControlRead(void)            ;
uint8 myUSB_InitControlWrite(void)           ;
void  myUSB_ControlReadDataStage(void)       ;
void  myUSB_ControlReadStatusStage(void)     ;
void  myUSB_ControlReadPrematureStatus(void) ;
uint8 myUSB_InitControlWrite(void)           ;
uint8 myUSB_InitZeroLengthControlTransfer(void) ;
void  myUSB_ControlWriteDataStage(void)      ;
void  myUSB_ControlWriteStatusStage(void)    ;
void  myUSB_ControlWritePrematureStatus(void);
uint8 myUSB_InitNoDataControlTransfer(void)  ;
void  myUSB_NoDataControlStatusStage(void)   ;
void  myUSB_InitializeStatusBlock(void)      ;
void  myUSB_UpdateStatusBlock(uint8 completionCode) ;
uint8 myUSB_DispatchClassRqst(void)          ;

void myUSB_Config(uint8 clearAltSetting) ;
void myUSB_ConfigAltChanged(void)        ;
void myUSB_ConfigReg(void)               ;
void myUSB_EpStateInit(void)             ;


const T_myUSB_LUT CYCODE *myUSB_GetConfigTablePtr(uint8 confIndex);
const T_myUSB_LUT CYCODE *myUSB_GetDeviceTablePtr(void)           ;
#if (myUSB_BOS_ENABLE)
    const T_myUSB_LUT CYCODE *myUSB_GetBOSPtr(void)               ;
#endif /* (myUSB_BOS_ENABLE) */
const uint8 CYCODE *myUSB_GetInterfaceClassTablePtr(void)                    ;
uint8 myUSB_ClearEndpointHalt(void)                                          ;
uint8 myUSB_SetEndpointHalt(void)                                            ;
uint8 myUSB_ValidateAlternateSetting(void)                                   ;

void myUSB_SaveConfig(void)      ;
void myUSB_RestoreConfig(void)   ;

#if (CY_PSOC3 || CY_PSOC5LP)
    #if (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u))
        void myUSB_LoadNextInEP(uint8 epNumber, uint8 mode)  ;
    #endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO && (myUSB_EP_DMA_AUTO_OPT == 0u)) */
#endif /* (CY_PSOC3 || CY_PSOC5LP) */

#if defined(myUSB_ENABLE_IDSN_STRING)
    void myUSB_ReadDieID(uint8 descr[])  ;
#endif /* myUSB_ENABLE_IDSN_STRING */

#if defined(myUSB_ENABLE_HID_CLASS)
    uint8 myUSB_DispatchHIDClassRqst(void) ;
#endif /* (myUSB_ENABLE_HID_CLASS) */

#if defined(myUSB_ENABLE_AUDIO_CLASS)
    uint8 myUSB_DispatchAUDIOClassRqst(void) ;
#endif /* (myUSB_ENABLE_AUDIO_CLASS) */

#if defined(myUSB_ENABLE_CDC_CLASS)
    uint8 myUSB_DispatchCDCClassRqst(void) ;
#endif /* (myUSB_ENABLE_CDC_CLASS) */

#if (myUSB_ENABLE_MSC_CLASS)
    #if (myUSB_HANDLE_MSC_REQUESTS)
        uint8 myUSB_DispatchMSCClassRqst(void) ;
    #endif /* (myUSB_HANDLE_MSC_REQUESTS) */
#endif /* (myUSB_ENABLE_MSC_CLASS */

CY_ISR_PROTO(myUSB_EP_0_ISR);
CY_ISR_PROTO(myUSB_BUS_RESET_ISR);

#if (myUSB_SOF_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_SOF_ISR);
#endif /* (myUSB_SOF_ISR_ACTIVE) */

#if (myUSB_EP1_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_1_ISR);
#endif /* (myUSB_EP1_ISR_ACTIVE) */

#if (myUSB_EP2_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_2_ISR);
#endif /* (myUSB_EP2_ISR_ACTIVE) */

#if (myUSB_EP3_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_3_ISR);
#endif /* (myUSB_EP3_ISR_ACTIVE) */

#if (myUSB_EP4_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_4_ISR);
#endif /* (myUSB_EP4_ISR_ACTIVE) */

#if (myUSB_EP5_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_5_ISR);
#endif /* (myUSB_EP5_ISR_ACTIVE) */

#if (myUSB_EP6_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_6_ISR);
#endif /* (myUSB_EP6_ISR_ACTIVE) */

#if (myUSB_EP7_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_7_ISR);
#endif /* (myUSB_EP7_ISR_ACTIVE) */

#if (myUSB_EP8_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_EP_8_ISR);
#endif /* (myUSB_EP8_ISR_ACTIVE) */

#if (myUSB_EP_MANAGEMENT_DMA)
    CY_ISR_PROTO(myUSB_ARB_ISR);
#endif /* (myUSB_EP_MANAGEMENT_DMA) */

#if (myUSB_DP_ISR_ACTIVE)
    CY_ISR_PROTO(myUSB_DP_ISR);
#endif /* (myUSB_DP_ISR_ACTIVE) */

#if (CY_PSOC4)
    CY_ISR_PROTO(myUSB_INTR_HI_ISR);
    CY_ISR_PROTO(myUSB_INTR_MED_ISR);
    CY_ISR_PROTO(myUSB_INTR_LO_ISR);
    #if (myUSB_LPM_ACTIVE)
        CY_ISR_PROTO(myUSB_LPM_ISR);
    #endif /* (myUSB_LPM_ACTIVE) */
#endif /* (CY_PSOC4) */

#if (myUSB_EP_MANAGEMENT_DMA_AUTO)
#if (CY_PSOC4)
    #if (myUSB_DMA1_ACTIVE)
        void myUSB_EP1_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA1_ACTIVE) */

    #if (myUSB_DMA2_ACTIVE)
        void myUSB_EP2_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA2_ACTIVE) */

    #if (myUSB_DMA3_ACTIVE)
        void myUSB_EP3_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA3_ACTIVE) */

    #if (myUSB_DMA4_ACTIVE)
        void myUSB_EP4_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA4_ACTIVE) */

    #if (myUSB_DMA5_ACTIVE)
        void myUSB_EP5_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA5_ACTIVE) */

    #if (myUSB_DMA6_ACTIVE)
        void myUSB_EP6_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA6_ACTIVE) */

    #if (myUSB_DMA7_ACTIVE)
        void myUSB_EP7_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA7_ACTIVE) */

    #if (myUSB_DMA8_ACTIVE)
        void myUSB_EP8_DMA_DONE_ISR(void);
    #endif /* (myUSB_DMA8_ACTIVE) */

#else
    #if (myUSB_EP_DMA_AUTO_OPT == 0u)
        CY_ISR_PROTO(myUSB_EP_DMA_DONE_ISR);
    #endif /* (myUSB_EP_DMA_AUTO_OPT == 0u) */
#endif /* (CY_PSOC4) */
#endif /* (myUSB_EP_MANAGEMENT_DMA_AUTO) */


/***************************************
*         Request Handlers
***************************************/

uint8 myUSB_HandleStandardRqst(void) ;
uint8 myUSB_DispatchClassRqst(void)  ;
uint8 myUSB_HandleVendorRqst(void)   ;


/***************************************
*    HID Internal references
***************************************/

#if defined(myUSB_ENABLE_HID_CLASS)
    void myUSB_FindReport(void)            ;
    void myUSB_FindReportDescriptor(void)  ;
    void myUSB_FindHidClassDecriptor(void) ;
#endif /* myUSB_ENABLE_HID_CLASS */


/***************************************
*    MIDI Internal references
***************************************/

#if defined(myUSB_ENABLE_MIDI_STREAMING)
    void myUSB_MIDI_IN_EP_Service(void)  ;
#endif /* (myUSB_ENABLE_MIDI_STREAMING) */


/***************************************
*    CDC Internal references
***************************************/

#if defined(myUSB_ENABLE_CDC_CLASS)

    typedef struct
    {
        uint8  bRequestType;
        uint8  bNotification;
        uint8  wValue;
        uint8  wValueMSB;
        uint8  wIndex;
        uint8  wIndexMSB;
        uint8  wLength;
        uint8  wLengthMSB;
        uint8  wSerialState;
        uint8  wSerialStateMSB;
    } t_myUSB_cdc_notification;

    uint8 myUSB_GetInterfaceComPort(uint8 interface) ;
    uint8 myUSB_Cdc_EpInit( const T_myUSB_EP_SETTINGS_BLOCK CYCODE *pEP, uint8 epNum, uint8 cdcComNums) ;

    extern volatile uint8  myUSB_cdc_dataInEpList[myUSB_MAX_MULTI_COM_NUM];
    extern volatile uint8  myUSB_cdc_dataOutEpList[myUSB_MAX_MULTI_COM_NUM];
    extern volatile uint8  myUSB_cdc_commInEpList[myUSB_MAX_MULTI_COM_NUM];
#endif /* (myUSB_ENABLE_CDC_CLASS) */


#endif /* CY_USBFS_myUSB_pvt_H */


/* [] END OF FILE */
