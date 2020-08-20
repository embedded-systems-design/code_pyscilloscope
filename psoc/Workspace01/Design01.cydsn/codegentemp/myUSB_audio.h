/***************************************************************************//**
* \file myUSB_audio.h
* \version 3.20
*
* \brief
*  This file provides function prototypes and constants for the USBFS component 
*  Audio class.
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

#if !defined(CY_USBFS_myUSB_audio_H)
#define CY_USBFS_myUSB_audio_H

#include "myUSB.h"


/***************************************
* Custom Declarations
***************************************/

/* `#START CUSTOM_CONSTANTS` Place your declaration here */

/* `#END` */


/***************************************
*  Constants for myUSB_audio API.
***************************************/

/* Audio Class-Specific Request Codes (AUDIO Table A-9) */
#define myUSB_REQUEST_CODE_UNDEFINED     (0x00u)
#define myUSB_SET_CUR                    (0x01u)
#define myUSB_GET_CUR                    (0x81u)
#define myUSB_SET_MIN                    (0x02u)
#define myUSB_GET_MIN                    (0x82u)
#define myUSB_SET_MAX                    (0x03u)
#define myUSB_GET_MAX                    (0x83u)
#define myUSB_SET_RES                    (0x04u)
#define myUSB_GET_RES                    (0x84u)
#define myUSB_SET_MEM                    (0x05u)
#define myUSB_GET_MEM                    (0x85u)
#define myUSB_GET_STAT                   (0xFFu)

/* point Control Selectors (AUDIO Table A-19) */
#define myUSB_EP_CONTROL_UNDEFINED       (0x00u)
#define myUSB_SAMPLING_FREQ_CONTROL      (0x01u)
#define myUSB_PITCH_CONTROL              (0x02u)

/* Feature Unit Control Selectors (AUDIO Table A-11) */
#define myUSB_FU_CONTROL_UNDEFINED       (0x00u)
#define myUSB_MUTE_CONTROL               (0x01u)
#define myUSB_VOLUME_CONTROL             (0x02u)
#define myUSB_BASS_CONTROL               (0x03u)
#define myUSB_MID_CONTROL                (0x04u)
#define myUSB_TREBLE_CONTROL             (0x05u)
#define myUSB_GRAPHIC_EQUALIZER_CONTROL  (0x06u)
#define myUSB_AUTOMATIC_GAIN_CONTROL     (0x07u)
#define myUSB_DELAY_CONTROL              (0x08u)
#define myUSB_BASS_BOOST_CONTROL         (0x09u)
#define myUSB_LOUDNESS_CONTROL           (0x0Au)

#define myUSB_SAMPLE_FREQ_LEN            (3u)
#define myUSB_VOLUME_LEN                 (2u)

#if !defined(USER_SUPPLIED_DEFAULT_VOLUME_VALUE)
    #define myUSB_VOL_MIN_MSB            (0x80u)
    #define myUSB_VOL_MIN_LSB            (0x01u)
    #define myUSB_VOL_MAX_MSB            (0x7Fu)
    #define myUSB_VOL_MAX_LSB            (0xFFu)
    #define myUSB_VOL_RES_MSB            (0x00u)
    #define myUSB_VOL_RES_LSB            (0x01u)
#endif /* USER_SUPPLIED_DEFAULT_VOLUME_VALUE */


/***************************************
* External data references
***************************************/
/**
* \addtogroup group_audio
* @{
*/
extern volatile uint8 myUSB_currentSampleFrequency[myUSB_MAX_EP][myUSB_SAMPLE_FREQ_LEN];
extern volatile uint8 myUSB_frequencyChanged;
extern volatile uint8 myUSB_currentMute;
extern volatile uint8 myUSB_currentVolume[myUSB_VOLUME_LEN];
/** @} audio */

extern volatile uint8 myUSB_minimumVolume[myUSB_VOLUME_LEN];
extern volatile uint8 myUSB_maximumVolume[myUSB_VOLUME_LEN];
extern volatile uint8 myUSB_resolutionVolume[myUSB_VOLUME_LEN];

#endif /*  CY_USBFS_myUSB_audio_H */


/* [] END OF FILE */
