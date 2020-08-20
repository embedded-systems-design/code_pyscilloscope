/*******************************************************************************
* File Name: myADC_SEQ.h
* Version 2.10
*
* Description:
*  Contains the function prototypes, constants and register definition of the
*  ADC SAR Sequencer Component.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2012-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_myADC_SEQ_H)
    #define CY_myADC_SEQ_H

#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h"
#include "myADC_SEQ_TempBuf_dma.h"
#include "myADC_SEQ_FinalBuf_dma.h"
#include "myADC_SEQ_SAR.h"

#define myADC_SEQ_NUMBER_OF_CHANNELS    (3u)
#define myADC_SEQ_SAMPLE_MODE           (0u)
#define myADC_SEQ_CLOCK_SOURCE          (0u)

extern int16  myADC_SEQ_finalArray[myADC_SEQ_NUMBER_OF_CHANNELS];
extern uint32 myADC_SEQ_initVar;

/* Check to see if required defines such as CY_PSOC5A are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5A)
    #error Component ADC_SAR_SEQ_v2_10 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5A) */


/***************************************
*        Function Prototypes
***************************************/
void myADC_SEQ_Init(void);
void myADC_SEQ_Enable(void);
void myADC_SEQ_Disable(void);
void myADC_SEQ_Start(void);
void myADC_SEQ_Stop(void);

uint32 myADC_SEQ_IsEndConversion(uint8 retMode);
int16 myADC_SEQ_GetResult16(uint16 chan);
int16 myADC_SEQ_GetAdcResult(void);
void myADC_SEQ_SetOffset(int32 offset);
void myADC_SEQ_SetResolution(uint8 resolution);
void myADC_SEQ_SetScaledGain(int32 adcGain);
int32 myADC_SEQ_CountsTo_mVolts(int16 adcCounts);
int32 myADC_SEQ_CountsTo_uVolts(int16 adcCounts);
float32 myADC_SEQ_CountsTo_Volts(int16 adcCounts);
void myADC_SEQ_Sleep(void);
void myADC_SEQ_Wakeup(void);
void myADC_SEQ_SaveConfig(void);
void myADC_SEQ_RestoreConfig(void);

CY_ISR_PROTO( myADC_SEQ_ISR );

/* Obsolete API for backward compatibility.
*  Should not be used in new designs.
*/
void myADC_SEQ_SetGain(int32 adcGain);


/**************************************
*    Initial Parameter Constants
**************************************/
#define myADC_SEQ_IRQ_REMOVE             (0u)                /* Removes internal interrupt */


/***************************************
*             Registers
***************************************/
#define myADC_SEQ_CYCLE_COUNTER_AUX_CONTROL_REG \
                                               (*(reg8 *) myADC_SEQ_bSAR_SEQ_ChannelCounter__CONTROL_AUX_CTL_REG)
#define myADC_SEQ_CYCLE_COUNTER_AUX_CONTROL_PTR \
                                               ( (reg8 *) myADC_SEQ_bSAR_SEQ_ChannelCounter__CONTROL_AUX_CTL_REG)
#define myADC_SEQ_CONTROL_REG    (*(reg8 *) \
                                             myADC_SEQ_bSAR_SEQ_CtrlReg__CONTROL_REG)
#define myADC_SEQ_CONTROL_PTR    ( (reg8 *) \
                                             myADC_SEQ_bSAR_SEQ_CtrlReg__CONTROL_REG)
#define myADC_SEQ_COUNT_REG      (*(reg8 *) \
                                             myADC_SEQ_bSAR_SEQ_ChannelCounter__COUNT_REG)
#define myADC_SEQ_COUNT_PTR      ( (reg8 *) \
                                             myADC_SEQ_bSAR_SEQ_ChannelCounter__COUNT_REG)
#define myADC_SEQ_STATUS_REG     (*(reg8 *) myADC_SEQ_bSAR_SEQ_EOCSts__STATUS_REG)
#define myADC_SEQ_STATUS_PTR     ( (reg8 *) myADC_SEQ_bSAR_SEQ_EOCSts__STATUS_REG)

#define myADC_SEQ_SAR_DATA_ADDR_0 (myADC_SEQ_SAR_ADC_SAR__WRK0)
#define myADC_SEQ_SAR_DATA_ADDR_1 (myADC_SEQ_SAR_ADC_SAR__WRK1)
#define myADC_SEQ_SAR_DATA_ADDR_0_REG (*(reg8 *) \
                                              myADC_SEQ_SAR_ADC_SAR__WRK0)
#define myADC_SEQ_SAR_DATA_ADDR_1_REG (*(reg8 *) \
                                              myADC_SEQ_SAR_ADC_SAR__WRK1)


/**************************************
*       Register Constants
**************************************/

#if(myADC_SEQ_IRQ_REMOVE == 0u)

    /* Priority of the ADC_SAR_IRQ interrupt. */
    #define myADC_SEQ_INTC_PRIOR_NUMBER          (uint8)(myADC_SEQ_IRQ__INTC_PRIOR_NUM)

    /* ADC_SAR_IRQ interrupt number */
    #define myADC_SEQ_INTC_NUMBER                (uint8)(myADC_SEQ_IRQ__INTC_NUMBER)

#endif   /* End myADC_SEQ_IRQ_REMOVE */


/***************************************
*       API Constants
***************************************/

/* Constants for IsEndConversion() "retMode" parameter */
#define myADC_SEQ_RETURN_STATUS              (0x01u)
#define myADC_SEQ_WAIT_FOR_RESULT            (0x00u)

/* Defines for the Resolution parameter */
#define myADC_SEQ_BITS_12    myADC_SEQ_SAR__BITS_12
#define myADC_SEQ_BITS_10    myADC_SEQ_SAR__BITS_10
#define myADC_SEQ_BITS_8     myADC_SEQ_SAR__BITS_8

#define myADC_SEQ_CYCLE_COUNTER_ENABLE    (0x20u)
#define myADC_SEQ_BASE_COMPONENT_ENABLE   (0x01u)
#define myADC_SEQ_LOAD_COUNTER_PERIOD     (0x02u)
#define myADC_SEQ_SOFTWARE_SOC_PULSE      (0x04u)

/* Generic DMA Configuration parameters */
#define myADC_SEQ_TEMP_BYTES_PER_BURST     (uint8)(2u)
#define myADC_SEQ_TEMP_TRANSFER_COUNT      ((uint16)myADC_SEQ_NUMBER_OF_CHANNELS << 1u)
#define myADC_SEQ_FINAL_BYTES_PER_BURST    ((uint16)myADC_SEQ_NUMBER_OF_CHANNELS << 1u)
#define myADC_SEQ_REQUEST_PER_BURST        (uint8)(1u)

#define myADC_SEQ_GET_RESULT_INDEX_OFFSET    ((uint8)myADC_SEQ_NUMBER_OF_CHANNELS - 1u)

/* Define for Sample Mode  */
#define myADC_SEQ_SAMPLE_MODE_FREE_RUNNING    (0x00u)
#define myADC_SEQ_SAMPLE_MODE_SW_TRIGGERED    (0x01u)
#define myADC_SEQ_SAMPLE_MODE_HW_TRIGGERED    (0x02u)

/* Define for Clock Source  */
#define myADC_SEQ_CLOCK_INTERNAL              (0x00u)
#define myADC_SEQ_CLOCK_EXTERNAL              (0x01u)


/***************************************
*        Optional Function Prototypes
***************************************/
#if(myADC_SEQ_SAMPLE_MODE != myADC_SEQ_SAMPLE_MODE_HW_TRIGGERED)
    void myADC_SEQ_StartConvert(void);
    void myADC_SEQ_StopConvert(void);
#endif /* myADC_SEQ_SAMPLE_MODE != myADC_SEQ_SAMPLE_MODE_HW_TRIGGERED */

#endif  /* !defined(CY_myADC_SEQ_H) */

/* [] END OF FILE */
