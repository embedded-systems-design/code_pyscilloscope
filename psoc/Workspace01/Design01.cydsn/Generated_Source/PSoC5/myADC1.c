/*******************************************************************************
* File Name: myADC1.c
* Version 3.30
*
* Description:
*  This file provides the source code to the API for the Delta-Sigma ADC
*  Component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myADC1.h"

#if(myADC1_DEFAULT_INTERNAL_CLK)
    #include "myADC1_theACLK.h"
#endif /* myADC1_DEFAULT_INTERNAL_CLK */

#include "myADC1_Ext_CP_Clk.h"

#if(myADC1_DEFAULT_INPUT_MODE)
    #include "myADC1_AMux.h"
#endif /* myADC1_DEFAULT_INPUT_MODE */


/***************************************
* Global data allocation
***************************************/

/* Software flag for checking conversion completed or not */
volatile uint8 myADC1_convDone = 0u;

/* Software flag to stop conversion for single sample conversion mode
*   with resolution above 16 bits 
*/
volatile uint8 myADC1_stopConversion = 0u;

/* To run the initialization block only at the start up */
uint8 myADC1_initVar = 0u;

/* To check whether ADC started or not before switching the configuration */
volatile uint8 myADC1_started = 0u;

/* Flag to hold ADC config number. By default active config is 1. */
volatile uint8 myADC1_Config = 1u;

volatile int32 myADC1_Offset = 0;
volatile int32 myADC1_CountsPerVolt;


/***************************************
* Local data allocation
***************************************/

/* The array with precalculated gain compensation coefficients */
static myADC1_GCOR_STRUCT myADC1_gcor[myADC1_DEFAULT_NUM_CONFIGS];



/***************************************
* Forward function references
***************************************/
static void myADC1_InitConfig(uint8 config) ;
static void myADC1_GainCompensation(uint8 inputRange, uint16 idealDecGain, uint16 idealOddDecGain, 
                            uint8 resolution, uint8 config) ;
static void myADC1_SetDSMRef0Reg(uint8 value) ;


/******************************************************************************
* Function Name: myADC1_Init
*******************************************************************************
*
* Summary:
*  Initialize component's parameters to the parameters set by user in the
*  customizer of the component placed onto schematic. Usually called in
* myADC1_Start().
*
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void myADC1_Init(void) 
{

    myADC1_Config = 1u;
    myADC1_convDone = 0u;

    myADC1_Ext_CP_Clk_SetMode(CYCLK_DUTY);

    /* This is only valid if there is an internal clock */
    #if(myADC1_DEFAULT_INTERNAL_CLK)
        myADC1_theACLK_SetMode(CYCLK_DUTY);
    #endif /* myADC1_DEFAULT_INTERNAL_CLK */

    #if(myADC1_IRQ_REMOVE == 0u)
        /* Set interrupt priority */
        CyIntSetPriority(myADC1_INTC_NUMBER, myADC1_INTC_PRIOR_NUMBER);
    #endif   /* End myADC1_IRQ_REMOVE */

    /* Init static registers with common configuration */
    myADC1_DSM_DEM0_REG    = myADC1_CFG1_DSM_DEM0;
    myADC1_DSM_DEM1_REG    = myADC1_CFG1_DSM_DEM1;
    myADC1_DSM_MISC_REG    = myADC1_CFG1_DSM_MISC;
    myADC1_DSM_CLK_REG    |= myADC1_CFG1_DSM_CLK;
    myADC1_DSM_REF1_REG    = myADC1_CFG1_DSM_REF1;

    myADC1_DSM_OUT0_REG    = myADC1_CFG1_DSM_OUT0;
    myADC1_DSM_OUT1_REG    = myADC1_CFG1_DSM_OUT1;

    myADC1_DSM_CR0_REG     = myADC1_CFG1_DSM_CR0;
    myADC1_DSM_CR1_REG     = myADC1_CFG1_DSM_CR1;
#if(myADC1_MI_ENABLE != 0u) /* Enable Modulator Input */
    myADC1_DSM_CR3_REG     |= myADC1_DSM_MODBIT_EN;
#else
    myADC1_DSM_CR3_REG     = myADC1_CFG1_DSM_CR3;
#endif /* myADC1_MI_ENABLE != 0u*/    
    myADC1_DSM_CR8_REG     = myADC1_CFG1_DSM_CR8;
    myADC1_DSM_CR9_REG     = myADC1_CFG1_DSM_CR9;
    myADC1_DSM_CR13_REG    = myADC1_CFG1_DSM_CR13;

    myADC1_DEC_SR_REG      = myADC1_CFG1_DEC_SR;

    /* Calculate Gain compensation coefficients for all configurations */
    myADC1_GainCompensation(myADC1_CFG1_INPUT_RANGE,
                                      myADC1_CFG1_IDEAL_DEC_GAIN,
                                      myADC1_CFG1_IDEAL_ODDDEC_GAIN,
                                      myADC1_CFG1_RESOLUTION,
                                      myADC1_CFG1);
    #if(myADC1_DEFAULT_NUM_CONFIGS > 1)
        myADC1_GainCompensation(myADC1_CFG2_INPUT_RANGE,
                                          myADC1_CFG2_IDEAL_DEC_GAIN,
                                          myADC1_CFG2_IDEAL_ODDDEC_GAIN,
                                          myADC1_CFG2_RESOLUTION,
                                          myADC1_CFG2);
    #endif /* myADC1_DEFAULT_NUM_CONFIGS > 1 */
    #if(myADC1_DEFAULT_NUM_CONFIGS > 2)
        myADC1_GainCompensation(myADC1_CFG3_INPUT_RANGE,
                                          myADC1_CFG3_IDEAL_DEC_GAIN,
                                          myADC1_CFG3_IDEAL_ODDDEC_GAIN,
                                          myADC1_CFG3_RESOLUTION,
                                          myADC1_CFG3);
    #endif /* myADC1_DEFAULT_NUM_CONFIGS > 2 */
    #if(myADC1_DEFAULT_NUM_CONFIGS > 3)
        myADC1_GainCompensation(myADC1_CFG4_INPUT_RANGE,
                                          myADC1_CFG4_IDEAL_DEC_GAIN,
                                          myADC1_CFG4_IDEAL_ODDDEC_GAIN,
                                          myADC1_CFG4_RESOLUTION,
                                          myADC1_CFG4);
    #endif /* myADC1_DEFAULT_NUM_CONFIGS > 3 */

    /* Set GCOR register for config1 */
    myADC1_DEC_GVAL_REG = myADC1_gcor[myADC1_Config - 1u].gval;
    CY_SET_REG16(myADC1_DEC_GCOR_16B_PTR, myADC1_gcor[myADC1_Config - 1u].gcor);

    /* Initialize the registers with default customizer settings for config1 */
    myADC1_InitConfig(myADC1_Config);
}


/******************************************************************************
* Function Name: myADC1_Enable
*******************************************************************************
*
* Summary:
*  Enables the ADC DelSig block operation.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects: 
*  Enables internal interrupt.
*
*******************************************************************************/
void myADC1_Enable(void) 
{
	uint8 config;
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Read volatile variable to the local variable */
    config = myADC1_Config;
    
    /* Enable active mode power for ADC */
    myADC1_PWRMGR_DEC_REG |= myADC1_ACT_PWR_DEC_EN;
    myADC1_PWRMGR_DSM_REG |= myADC1_ACT_PWR_DSM_EN;

     /* Enable alternative active mode power for ADC */
    myADC1_STBY_PWRMGR_DEC_REG |= myADC1_STBY_PWR_DEC_EN;
    myADC1_STBY_PWRMGR_DSM_REG |= myADC1_STBY_PWR_DSM_EN;

    /* Disable PRES, Enable power to VCMBUF0, REFBUF0 and REFBUF1, enable PRES */
    myADC1_RESET_CR4_REG |= myADC1_IGNORE_PRESA1;
    myADC1_RESET_CR5_REG |= myADC1_IGNORE_PRESA2;

    myADC1_DSM_CR17_REG |= (myADC1_DSM_EN_BUF_VREF | myADC1_DSM_EN_BUF_VCM);

    /* Code to disable the REFBUF0 if reference chosen is External ref */
    #if (((myADC1_CFG1_REFERENCE == myADC1_EXT_REF_ON_P03) || \
          (myADC1_CFG1_REFERENCE == myADC1_EXT_REF_ON_P32)) || \
         ((myADC1_DEFAULT_NUM_CONFIGS > 1) && \
          ((myADC1_CFG2_REFERENCE == myADC1_EXT_REF_ON_P03) ||  \
           (myADC1_CFG2_REFERENCE == myADC1_EXT_REF_ON_P32))) || \
         ((myADC1_DEFAULT_NUM_CONFIGS > 2) && \
          ((myADC1_CFG3_REFERENCE == myADC1_EXT_REF_ON_P03) ||  \
           (myADC1_CFG3_REFERENCE == myADC1_EXT_REF_ON_P32))) || \
         ((myADC1_DEFAULT_NUM_CONFIGS > 3) && \
          ((myADC1_CFG4_REFERENCE == myADC1_EXT_REF_ON_P03) || \
           (myADC1_CFG4_REFERENCE == myADC1_EXT_REF_ON_P32))))
        if (((config == 1u) &&
            ((myADC1_CFG1_REFERENCE == myADC1_EXT_REF_ON_P03) ||
             (myADC1_CFG1_REFERENCE == myADC1_EXT_REF_ON_P32))) ||
            ((config == 2u) &&
            ((myADC1_CFG2_REFERENCE == myADC1_EXT_REF_ON_P03) ||
             (myADC1_CFG2_REFERENCE == myADC1_EXT_REF_ON_P32))) ||
            ((config == 3u) &&
            ((myADC1_CFG3_REFERENCE == myADC1_EXT_REF_ON_P03) ||
             (myADC1_CFG3_REFERENCE == myADC1_EXT_REF_ON_P32))) ||
            ((config == 4u) &&
            ((myADC1_CFG4_REFERENCE == myADC1_EXT_REF_ON_P03) ||
             (myADC1_CFG4_REFERENCE == myADC1_EXT_REF_ON_P32))))
        {
            /* Disable the REFBUF0 */
            myADC1_DSM_CR17_REG &= (uint8)~myADC1_DSM_EN_BUF_VREF;
        }
    #endif /* External ref */

    #if (((myADC1_CFG1_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF) || \
         ((myADC1_DEFAULT_NUM_CONFIGS > 1) && \
          (myADC1_CFG2_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF)) || \
         ((myADC1_DEFAULT_NUM_CONFIGS > 2) && \
          (myADC1_CFG3_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF)) || \
         ((myADC1_DEFAULT_NUM_CONFIGS > 3) && \
          (myADC1_CFG4_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF))))
        if(((config == 1u) &&
            (myADC1_CFG1_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF) &&
            ((myADC1_CFG1_REFERENCE != myADC1_EXT_REF_ON_P03) &&
             (myADC1_CFG1_REFERENCE != myADC1_EXT_REF_ON_P32))) ||
             ((config == 2u) &&
              (myADC1_CFG2_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF) &&
             ((myADC1_CFG2_REFERENCE != myADC1_EXT_REF_ON_P03) &&
             (myADC1_CFG2_REFERENCE != myADC1_EXT_REF_ON_P32))) ||
             ((config == 3u) &&
              (myADC1_CFG3_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF) &&
             ((myADC1_CFG3_REFERENCE != myADC1_EXT_REF_ON_P03) &&
             (myADC1_CFG3_REFERENCE != myADC1_EXT_REF_ON_P32))) ||
             ((config == 4u) &&
              (myADC1_CFG4_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF) &&
             ((myADC1_CFG4_REFERENCE != myADC1_EXT_REF_ON_P03) &&
             (myADC1_CFG4_REFERENCE != myADC1_EXT_REF_ON_P32))))
        {
            /* Enable the REFBUF1 */
            myADC1_DSM_REF0_REG |= myADC1_DSM_EN_BUF_VREF_INN;
        }
    #endif /* VSSA_TO_2VREF */
    if(config != 0u)
    {
        /* Suppress compiler warning */
    }

    /* Wait for 3 microseconds */
    CyDelayUs(myADC1_PRES_DELAY_TIME);

    /* Enable the press circuit */
    myADC1_RESET_CR4_REG &= (uint8)~myADC1_IGNORE_PRESA1;
    myADC1_RESET_CR5_REG &= (uint8)~myADC1_IGNORE_PRESA2;

    /* Enable negative pumps for DSM  */
    myADC1_PUMP_CR1_REG  |= ( myADC1_PUMP_CR1_CLKSEL | myADC1_PUMP_CR1_FORCE );

    /* Enable Modulator Chopping if required */
    myADC1_DSM_CR2_REG = myADC1_CFG1_DSM_CR2;

    /* This is only valid if there is an internal clock */
    #if(myADC1_DEFAULT_INTERNAL_CLK)
        myADC1_PWRMGR_CLK_REG |= myADC1_ACT_PWR_CLK_EN;
        myADC1_STBY_PWRMGR_CLK_REG |= myADC1_STBY_PWR_CLK_EN;
    #endif /* myADC1_DEFAULT_INTERNAL_CLK */

    /* Enable the active and alternate active power for charge pump clock */
    myADC1_PWRMGR_CHARGE_PUMP_CLK_REG |= myADC1_ACT_PWR_CHARGE_PUMP_CLK_EN;
    myADC1_STBY_PWRMGR_CHARGE_PUMP_CLK_REG |= myADC1_STBY_PWR_CHARGE_PUMP_CLK_EN;

    #if(myADC1_IRQ_REMOVE == 0u)
        /* Clear a pending interrupt */
        CyIntClearPending(myADC1_INTC_NUMBER);
        /* Enable interrupt */
        CyIntEnable(myADC1_INTC_NUMBER);
    #endif   /* End myADC1_IRQ_REMOVE */

    CyExitCriticalSection(enableInterrupts);

}


/*******************************************************************************
* Function Name: myADC1_Start
********************************************************************************
*
* Summary:
*  Performs all required initialization for this component and enables
*  the power. It configure all the register the first time it is called.
*  Subsequent calls of the Start function only enable the ADC and turn
*  on the power. If multiple configurations are selected, it will
*  configure the ADC for configuration 1 by default, unless the
*  myADC1_SelectConfiguration( ) function has been called to change
*  the default setting.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  myADC1_initVar:  Used to check the initial configuration,
*  modified when this function is called for the first time.
*
* Side Effects: 
*  Enables internal interrupt.
*
*******************************************************************************/
void myADC1_Start(void) 
{
    if(myADC1_initVar == 0u)
    {
        if(myADC1_started == 0u)
        {
            myADC1_Init();
        }
        myADC1_initVar = 1u;
    }

    /* Enable the ADC */
    myADC1_Enable();
}


/*******************************************************************************
* Function Name: myADC1_Stop
********************************************************************************
*
* Summary:
*  This function stops and powers down the ADC component and the internal
*  clock if the external clock is not selected. If an external clock is
*  used, it is up to the designer to power down the external clock it
*  required.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void myADC1_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Stop conversions */
    myADC1_DEC_CR_REG &= (uint8)~myADC1_DEC_START_CONV;
    myADC1_DEC_SR_REG |=  myADC1_DEC_INTR_CLEAR;

    /* Disable PRES, Disable power to VCMBUF0, REFBUF0 and REFBUF1,
       enable PRES */
    myADC1_RESET_CR4_REG |= myADC1_IGNORE_PRESA1;
    myADC1_RESET_CR5_REG |= myADC1_IGNORE_PRESA2;

    myADC1_DSM_CR17_REG &= (uint8)~(myADC1_DSM_EN_BUF_VREF | myADC1_DSM_EN_BUF_VCM);
    myADC1_DSM_REF0_REG &= (uint8)~myADC1_DSM_EN_BUF_VREF_INN;

    /* Wait for 3 microseconds. */
    CyDelayUs(myADC1_PRES_DELAY_TIME);

    /* Enable the press circuit */
    myADC1_RESET_CR4_REG &= (uint8)~myADC1_IGNORE_PRESA1;
    myADC1_RESET_CR5_REG &= (uint8)~myADC1_IGNORE_PRESA2;

    /* Disable power to the ADC */
    myADC1_PWRMGR_DSM_REG &= (uint8)~myADC1_ACT_PWR_DSM_EN;

    /* Disable power to Decimator block */
    myADC1_PWRMGR_DEC_REG &= (uint8)~myADC1_ACT_PWR_DEC_EN;

    /* Disable alternative active power to the ADC */
    myADC1_STBY_PWRMGR_DEC_REG &= (uint8)~myADC1_STBY_PWR_DEC_EN;
    myADC1_STBY_PWRMGR_DSM_REG &= (uint8)~myADC1_STBY_PWR_DSM_EN;

   /* Disable negative pumps for DSM  */
    myADC1_PUMP_CR1_REG &= (uint8)~(myADC1_PUMP_CR1_CLKSEL | myADC1_PUMP_CR1_FORCE );

    /* This is only valid if there is an internal clock */
    #if(myADC1_DEFAULT_INTERNAL_CLK)
        myADC1_PWRMGR_CLK_REG &= (uint8)~myADC1_ACT_PWR_CLK_EN;
        myADC1_STBY_PWRMGR_CLK_REG &= (uint8)~myADC1_STBY_PWR_CLK_EN;
    #endif /* myADC1_DEFAULT_INTERNAL_CLK */

    /* Disable Modulator Chopping */
    myADC1_DSM_CR2_REG &= (uint8)~myADC1_DSM_MOD_CHOP_EN;
    /* Disable power to charge pump clock */
    myADC1_PWRMGR_CHARGE_PUMP_CLK_REG &= (uint8)~myADC1_ACT_PWR_CHARGE_PUMP_CLK_EN;
    myADC1_STBY_PWRMGR_CHARGE_PUMP_CLK_REG &= (uint8)~myADC1_STBY_PWR_CHARGE_PUMP_CLK_EN;
    
    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: myADC1_SetBufferGain
********************************************************************************
*
* Summary:
*  Sets input buffer gain.
*
* Parameters:
*  gain:  Two bit value to select a gain of 1, 2, 4, or 8.
*
* Return:
*  None
*
*******************************************************************************/
void myADC1_SetBufferGain(uint8 gain) 
{
    uint8 tmpReg;
    tmpReg = myADC1_DSM_BUF1_REG & (uint8)~myADC1_DSM_GAIN_MASK;
    tmpReg |= (uint8)(gain << myADC1_DSM_GAIN_SHIFT) & myADC1_DSM_GAIN_MASK;
    myADC1_DSM_BUF1_REG = tmpReg;
}


/*******************************************************************************
* Function Name: myADC1_SetCoherency
********************************************************************************
*
* Summary:
*  This function allows the user to change which of the ADC's 3 word
*  result will trigger a coherency unlock. The ADC's result will not be
*  updated until the set byte is read either by the ADC or DMA. 
*  By default the LSB is the coherency byte for right alignment data format. 
*  The middle or high byte is set automatically depend on left alignment 
*  configuration for DMA data transfer.
*  If DMA or if a custom API requires different byte to be read the last,
*  this API should be used to set the last byte of the ADC result that is read. 
*  If a multibyte read is performed either by DMA or the ARM processor, the
*  coherency can be set to any byte in the last word read.
*
* Parameters:
*  coherency:  Two bit value to set the coherency bit.
*           00-Coherency checking off
*           01-low byte is key byte
*           02-middle byte is the key byte
*           03-high byte is the key byte
*
* Return:
*  None
*
*******************************************************************************/
void myADC1_SetCoherency(uint8 coherency) 
{
    uint8 tmpReg;

    tmpReg = myADC1_DEC_COHER_REG & (uint8)~myADC1_DEC_SAMP_KEY_MASK;
    tmpReg |= coherency & myADC1_DEC_SAMP_KEY_MASK;
    myADC1_DEC_COHER_REG = tmpReg;
}


/*******************************************************************************
* Function Name: myADC1_SetGCOR
********************************************************************************
*
* Summary:
*  Calculates a new GCOR value and writes it into the GCOR register. 
*  The GCOR value is a 16-bit value that represents a gain of 0 to 2. 
*  The ADC result is multiplied by this value before it is placed in the ADC 
*  output registers. The numerical format for the GCOR value is:
*  0x0000 -> 0.000
*  0x8000 -> 1.000
*  0xFFFF -> 1.99997
*  When executing the function, the old GCOR value is multiplied by
*  gainAdjust and reloaded into the GCOR register.
*
* Parameters:
*  gainAdjust:  floating point value to set GCOR registers.
*
* Return:
*  uint8: 0 - if GCOR value is within the expected range.
*         1 - the correction value is outside GCOR value range of
*             0.00 to 1.9999.
*
* Side Effects:  The GVAL register is set to the amount of valid bits in the
*                GCOR  register minus one. If GVAL is 15 (0x0F), all 16 bits
*                of the GCOR registers will be valid. If for example GVAL is
*                11 (0x0B) only 12 bits will be valid. The least 4 bits will
*                be lost when the GCOR value is shifted 4 places to the right.
*
******************************************************************************/
uint8 myADC1_SetGCOR(float32 gainAdjust) 
{
    uint16 tmpReg;
    uint8 status;
    float32 tmpValue;

    tmpReg = myADC1_gcor[myADC1_Config - 1u].gcor;
    tmpValue = ((float32)tmpReg / (float32)myADC1_IDEAL_GAIN_CONST);
    tmpValue = tmpValue * gainAdjust;

    if (tmpValue > 1.9999)
    {
        status = 1u;
    }
    else
    {
        tmpValue *= (float32)myADC1_IDEAL_GAIN_CONST;
		tmpReg = (uint16)tmpValue;
        CY_SET_REG16(myADC1_DEC_GCOR_16B_PTR, tmpReg);
        /* Update gain array to be used by SelectConfiguration() API */
       myADC1_gcor[myADC1_Config - 1u].gcor = tmpReg;

        status = 0u;

    }
    return(status);
}


/******************************************************************************
* Function Name: myADC1_ReadGCOR
*******************************************************************************
*
* Summary:
*  This API returns the current GCOR register value, normalized based on the
*  GVAL register settings.
*  For example, if the GCOR value is 0x0812 and the GVAL register is set to 
*  11 (0x0B) then the returned value will be shifted by for bits to the left.
*  (Actual GCOR value = 0x0812, returned value = 0x8120)
*
* Parameters:
*  None
*
* Return:
*  uint16:  Normalized GCOR value.
*
*******************************************************************************/
uint16 myADC1_ReadGCOR(void) 
{
    uint8 gValue;
    uint16 gcorValue;

    gValue = myADC1_DEC_GVAL_REG;
    gcorValue = CY_GET_REG16(myADC1_DEC_GCOR_16B_PTR);

    if (gValue < myADC1_MAX_GVAL)
    {
        gcorValue <<= myADC1_MAX_GVAL - gValue;
    }

    return gcorValue;
}


/*******************************************************************************
* Function Name: myADC1_StartConvert
********************************************************************************
*
* Summary:
*  Forces the ADC to initiate a conversion. If in the "Single Sample"
*  mode, one conversion will be performed then the ADC will halt. If in
*  one of the other three conversion modes, the ADC will run
*  continuously until the ADC_Stop() or ADC_StopConvert() is called.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void myADC1_StartConvert(void) 
{
    /* Start the conversion */
    myADC1_DEC_CR_REG |= myADC1_DEC_START_CONV;
}


/*******************************************************************************
* Function Name: myADC1_StopConvert
********************************************************************************
*
* Summary:
*  Forces the ADC to stop all conversions. If the ADC is in the middle of a
*  conversion, the ADC will be reset and not provide a result for that partial
*  conversion.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void myADC1_StopConvert(void) 
{
    /* Stop all conversions */
    myADC1_DEC_CR_REG &= (uint8)~myADC1_DEC_START_CONV;
}


/*******************************************************************************
* Function Name: myADC1_IsEndConversion
********************************************************************************
*
* Summary:
*  Checks the status that the most recently started conversion has completed.
*  The status is cleared by any of ADC_GetResult8(), ADC_GetResult16() or 
*  ADC_GetResult32() API.
*  This function provides the programmer with two options. In one mode this 
*  function immediately returns with the conversion status. In the other mode,
*  the function does not return (blocking) until the conversion has completed.
*
* Parameters:
*  retMode: Check conversion return mode. See the following table for options.
*   myADC1_RETURN_STATUS -   Immediately returns conversion result
*                                      status.
*   myADC1_WAIT_FOR_RESULT - Does not return until ADC conversion
*                                      is complete.
*
* Return:
*  If a nonzero value is returned, the last conversion has completed.
*  If the returned value is zero, the ADC is still calculating the last result.
*
* Global variables:
*  myADC1_convDone:  Used to check whether conversion is complete
*  or not for single sample mode with resolution is above 16
*
*******************************************************************************/
uint8 myADC1_IsEndConversion(uint8 retMode) 
{
    uint8 status;

    do
    {
        /* Check for stop convert if conversion mode is Single Sample with
        *   resolution above 16 bit
        */
        if(myADC1_stopConversion != 0u)
        {
            status = myADC1_convDone;
        }
        else
        {
            status = myADC1_DEC_SR_REG & myADC1_DEC_CONV_DONE;
        }
    }while((status != myADC1_DEC_CONV_DONE) && (retMode == myADC1_WAIT_FOR_RESULT));

    return(status);
}


/*******************************************************************************
* Function Name: myADC1_GetResult8
********************************************************************************
*
* Summary:
*  This function returns the result of an 8-bit conversion. If the
*  resolution is set greater than 8-bits, the LSB of the result will be
*  returned. When the ADC is configured for 8-bit single ended mode,
*  the ADC_GetResult16() function should be used instead. This
*  function returns only signed 8-bit values. The maximum positive
*  signed 8-bit value is 127, but in singled ended 8-bit mode, the
*  maximum positive value is 255.
*
* Parameters:
*  None
*
* Return:
*  int8: The LSB of the last ADC conversion.
*
* Global variables:
*  myADC1_convDone:  Cleared in single sample mode with resolution
*                              above 16 bits
*
*******************************************************************************/
int8 myADC1_GetResult8( void ) 
{
    int8 result;
    uint8 coherency;

    /* Read active coherency configuration */
    coherency = myADC1_DEC_COHER_REG & myADC1_DEC_SAMP_KEY_MASK;

    result = (int8)myADC1_DEC_SAMP_REG;

    if(coherency == myADC1_DEC_SAMP_KEY_MID)
    {   /* Dummy read of the middle byte to unlock the coherency */
        (void)myADC1_DEC_SAMPM_REG;
    }
    else  if(coherency == myADC1_DEC_SAMP_KEY_HIGH)
    {   /* Dummy read of the MSB byte to unlock the coherency */
        (void)myADC1_DEC_SAMPH_REG;
    }
    else /*No action required for other coherency */
    {
    }
    /* Clear conversion complete status in Single Sample mode with resolution above 16 bit */
    if(myADC1_stopConversion != 0u)
    {
        myADC1_convDone = 0u;
    }
    return (result);
}


/*******************************************************************************
* Function Name: myADC1_GetResult16
********************************************************************************
*
* Summary:
*  Returns a 16-bit result for a conversion with a result that has a
*  resolution of 8 to 16 bits. If the resolution is set greater than 16-bits,
*  it will return the 16 least significant bits of the result. When the ADC
*  is configured for 16-bit single ended mode, the ADC_GetResult32()
*  function should be used instead. This function returns only signed
*  16-bit result, which allows a maximum positive value of 32767, not 65535.
*  This function supports different coherency settings.
*
* Parameters:
*   void
*
* Return:
*  int16:  ADC result.
*
* Global variables:
*  myADC1_convDone:  Cleared in single sample mode with resolution
*                              above 16 bits
*
*******************************************************************************/
int16 myADC1_GetResult16(void) 
{
    uint16 result;
    uint8 coherency;

    /* Read active coherency configuration */
    coherency = myADC1_DEC_COHER_REG & myADC1_DEC_SAMP_KEY_MASK;

    if(coherency <= myADC1_DEC_SAMP_KEY_LOW)
    {   /*  Use default method to read result registers i.e. LSB byte read at the end*/
        #if (CY_PSOC3)
            result = myADC1_DEC_SAMPM_REG;
            result = (result << 8u) | myADC1_DEC_SAMP_REG;
        #else
            result = (CY_GET_REG16(myADC1_DEC_SAMP_16B_PTR));
        #endif /* CY_PSOC3 */
    }
    else /* MID or HIGH */
    {   /* Read middle byte at the end */
        #if (CY_PSOC3)
            result = (CY_GET_REG16(myADC1_DEC_SAMP_16B_PTR));
        #else
            result = myADC1_DEC_SAMP_REG;
            result |=  (uint16)((uint16)myADC1_DEC_SAMPM_REG << 8u);
        #endif /* CY_PSOC3 */
        if(coherency == myADC1_DEC_SAMP_KEY_HIGH)
        {   /* Dummy read of the MSB byte to unlock the coherency */
            (void)myADC1_DEC_SAMPH_REG;
        }
    }
    /* Clear conversion complete status in Single Sample mode with resolution above 16 bit */
    if(myADC1_stopConversion != 0u)
    {
        myADC1_convDone = 0u;
    }

    return ((int16)result);
}


/*******************************************************************************
* Function Name: myADC1_GetResult32
********************************************************************************
*
* Summary:
*  Returns a 32-bit result for a conversion with a result that has a
*  resolution of 8 to 20 bits.
*  This function supports different coherency settings.
*
* Parameters:
*  None
*
* Return:
*  int32: Result of the last ADC conversion.
*
* Global variables:
*  myADC1_convDone:  Cleared in single sample mode with resolution
*                              above 16 bits
*
*******************************************************************************/
int32 myADC1_GetResult32(void) 
{
    uint32 result;
    uint8 coherency;
    #if (CY_PSOC3)
	    uint16 tmp;
    #endif /* CY_PSOC3 */

    /* Read active coherency configuration */
    coherency = myADC1_DEC_COHER_REG & myADC1_DEC_SAMP_KEY_MASK;

    if(coherency <= myADC1_DEC_SAMP_KEY_LOW)
    {   /*  Use default method to read result registers i.e. LSB byte read at the end*/
        #if (CY_PSOC3)
            result = myADC1_DEC_SAMPH_REG;
            if((result & 0x80u) != 0u)
            {   /* Sign extend */
                result |= 0xFF00u;
            }
            result = (result << 8u) | myADC1_DEC_SAMPM_REG;
            result = (result << 8u) | myADC1_DEC_SAMP_REG;
        #else
            result = CY_GET_REG16(myADC1_DEC_SAMPH_16B_PTR);
            result = (result << 16u) | (CY_GET_REG16(myADC1_DEC_SAMP_16B_PTR));
        #endif /* CY_PSOC3 */
    }
    else if(coherency == myADC1_DEC_SAMP_KEY_MID)
    {   /* Read middle byte at the end */
        #if (CY_PSOC3)
            result = myADC1_DEC_SAMPH_REG;
            if((result & 0x80u) != 0u)
            {   /* Sign extend */
                result |= 0xFF00u;
            }
            result = (result << 16u) | (CY_GET_REG16(myADC1_DEC_SAMP_16B_PTR));
        #else
            result = CY_GET_REG16(myADC1_DEC_SAMPH_16B_PTR);
            result = (result << 16u) | myADC1_DEC_SAMP_REG;
            result |=  (uint32)((uint32)myADC1_DEC_SAMPM_REG << 8u);
        #endif /* CY_PSOC3 */
    }
    else /*myADC1_DEC_SAMP_KEY_HIGH */
    {
        /* Read MSB byte at the end */
        #if (CY_PSOC3)
            result = CY_GET_REG16(myADC1_DEC_SAMP_16B_PTR);
			tmp = myADC1_DEC_SAMPH_REG;
            if((tmp & 0x80u) != 0u)
            {   /* Sign extend */
                tmp |= 0xFF00u;
            }
            result |= (uint32)tmp << 16u;
        #else
            result = CY_GET_REG16(myADC1_DEC_SAMP_16B_PTR);
            result |= (uint32)((uint32)CY_GET_REG16(myADC1_DEC_SAMPH_16B_PTR) << 16u);
        #endif /* CY_PSOC3 */
    }
    /* Clear conversion complete status in Single Sample mode with resolution above 16 bit */
    if(myADC1_stopConversion != 0u)
    {
        myADC1_convDone = 0u;
    }

    return ((int32)result);
}


/*******************************************************************************
* Function Name: myADC1_SetOffset
********************************************************************************
*
* Summary:
*  Sets the ADC offset which is used by the functions ADC_CountsTo_uVolts, 
*  ADC_CountsTo_mVolts, and ADC_CountsTo_Volts to subtract the offset from the 
*  given reading before calculating the voltage conversion.
*
* Parameters:
*  int32:  This value is a measured value when the inputs are shorted or 
*          connected to the same input voltage.
*
* Return:
*  None
*
* Global variables:
*  myADC1_Offset:  Modified to set the user provided offset. This
*  variable is used for offset calibration purpose.
*
* Side Effects:
*  Affects the myADC1_CountsTo_Volts,
*  myADC1_CountsTo_mVolts, myADC1_CountsTo_uVolts functions
*  by subtracting the given offset.
*
*******************************************************************************/
void myADC1_SetOffset(int32 offset) 
{

    myADC1_Offset = offset;
}


/*******************************************************************************
* Function Name: myADC1_SetGain
********************************************************************************
*
* Summary:
*  Sets the ADC gain in counts per volt for the voltage conversion
*  functions below. This value is set by default by the reference and
*  input range settings. It should only be used to further calibrate the
*  ADC with a known input or if an external reference is used. This
*  function may also be used to calibrate an entire signal chain, not
*  just the ADC.
*
* Parameters:
*  int32: ADC gain in counts per volt.
*
* Return:
*  None
*
* Global variables:
*  myADC1_CountsPerVolt:  modified to set the ADC gain in counts
*   per volt.
*
* Side Effects:
*  Affects the myADC1_CountsTo_Volts,
*  myADC1_CountsTo_mVolts, myADC1_CountsTo_uVolts functions
*  supplying the correct conversion between ADC counts and voltage.
*
*******************************************************************************/
void myADC1_SetGain(int32 adcGain) 
{
    myADC1_CountsPerVolt = adcGain;
}


/*******************************************************************************
* Function Name: myADC1_CountsTo_mVolts
********************************************************************************
*
* Summary:
*  Converts the ADC counts output to mVolts as a 16-bit integer. For
*  example, if the ADC measured 0.534 volts, the return value would
*  be 534 mVolts.
*
* Parameters:
*  int32: adcCounts Result from the ADC conversion.
*
* Return:
*  int16:  Result in mVolts
*
* Global variables:
*  myADC1_CountsPerVolt:  used to convert ADC counts to mVolts.
*  myADC1_Offset:  Used as the offset while converting ADC counts
*   to mVolts.
*
*******************************************************************************/
int16 myADC1_CountsTo_mVolts(int32 adcCounts) 
{

    int16 mVolts;

    /* Convert adcCounts to the right align if left option selected */
    #if(myADC1_CFG1_DEC_DIV != 0)
        if(myADC1_Config == myADC1_CFG1)
        {
            adcCounts /= myADC1_CFG1_DEC_DIV;
        }
    #endif /* myADC1_CFG1_DEC_DIV */
    #if((myADC1_CFG2_DEC_DIV != 0) && (myADC1_DEFAULT_NUM_CONFIGS > 1))
        if(myADC1_Config == myADC1_CFG2)
        {
            adcCounts /= myADC1_CFG2_DEC_DIV;
        }
    #endif /* myADC1_CFG2_DEC_DIV */
    #if((myADC1_CFG3_DEC_DIV != 0) && (myADC1_DEFAULT_NUM_CONFIGS > 2))
        if(myADC1_Config == myADC1_CFG3)
        {
            adcCounts /= myADC1_CFG3_DEC_DIV;
        }
    #endif /* myADC1_CFG2_DEC_DIV */
    #if((myADC1_CFG4_DEC_DIV != 0) && (myADC1_DEFAULT_NUM_CONFIGS > 3))
        if(myADC1_Config == myADC1_CFG4)
        {
            adcCounts /= myADC1_CFG4_DEC_DIV;
        }
    #endif /* myADC1_CFG2_DEC_DIV */

    /* Subtract ADC offset */
    adcCounts -= myADC1_Offset;

    mVolts = (int16)(( adcCounts * myADC1_1MV_COUNTS ) / myADC1_CountsPerVolt) ;

    return(mVolts);
}


/*******************************************************************************
* Function Name: myADC1_CountsTo_Volts
********************************************************************************
*
* Summary:
*  Converts the ADC output to Volts as a floating point number. For
*  example, if the ADC measure a voltage of 1.2345 Volts, the
*  returned result would be +1.2345 Volts.
*
* Parameters:
*  int32 adcCounts:  Result from the ADC conversion.
*
* Return:
*  float32: Result in Volts
*
* Global variables:
*  myADC1_CountsPerVolt:  used to convert to Volts.
*  myADC1_Offset:  Used as the offset while converting ADC counts
*   to Volts.
*
*******************************************************************************/
float32 myADC1_CountsTo_Volts(int32 adcCounts) 
{

    float32 Volts;

    /* Convert adcCounts to the right align if left option selected */
    #if(myADC1_CFG1_DEC_DIV != 0)
        if(myADC1_Config == myADC1_CFG1)
        {
            adcCounts /= myADC1_CFG1_DEC_DIV;
        }
    #endif /* myADC1_CFG1_DEC_DIV */
    #if((myADC1_CFG2_DEC_DIV != 0) && (myADC1_DEFAULT_NUM_CONFIGS > 1))
        if(myADC1_Config == myADC1_CFG2)
        {
            adcCounts /= myADC1_CFG2_DEC_DIV;
        }
    #endif /* myADC1_CFG2_DEC_DIV */
    #if((myADC1_CFG3_DEC_DIV != 0) && (myADC1_DEFAULT_NUM_CONFIGS > 2))
        if(myADC1_Config == myADC1_CFG3)
        {
            adcCounts /= myADC1_CFG3_DEC_DIV;
        }
    #endif /* myADC1_CFG2_DEC_DIV */
    #if((myADC1_CFG4_DEC_DIV != 0) && (myADC1_DEFAULT_NUM_CONFIGS > 3))
        if(myADC1_Config == myADC1_CFG4)
        {
            adcCounts /= myADC1_CFG4_DEC_DIV;
        }
    #endif /* myADC1_CFG2_DEC_DIV */

    /* Subtract ADC offset */
    adcCounts -= myADC1_Offset;

    Volts = (float32)adcCounts / (float32)myADC1_CountsPerVolt;

    return( Volts );
}


/*******************************************************************************
* Function Name: myADC1_CountsTo_uVolts
********************************************************************************
*
* Summary:
*  Converts the ADC output to uVolts as a 32-bit integer. For example,
*  if the ADC measured -0.02345 Volts, the return value would be -23450 uVolts.
*
* Parameters:
*  int32 adcCounts: Result from the ADC conversion.
*
* Return:
*  int32:  Result in uVolts
*
* Global variables:
*  myADC1_CountsPerVolt:  used to convert ADC counts to mVolts.
*  myADC1_Offset:  Used as the offset while converting ADC counts
*   to mVolts.
*
* Theory:
*  Care must be taken to not exceed the maximum value for a 31 bit signed
*  number in the conversion to uVolts and at the same time not lose resolution.
*
*  uVolts = ((A * adcCounts) / ((int32)myADC1_CountsPerVolt / B));
*
*******************************************************************************/
int32 myADC1_CountsTo_uVolts(int32 adcCounts) 
{

    int32 uVolts;
    int32 coefA;
    int32 coefB;
    uint8 resolution;

    /* Set the resolution based on the configuration */
    /* Convert adcCounts to the right align if left option selected */
    if (myADC1_Config == myADC1_CFG1)
    {
        resolution = myADC1_CFG1_RESOLUTION;
        #if(myADC1_CFG1_DEC_DIV != 0)
            adcCounts /= myADC1_CFG1_DEC_DIV;
        #endif /* myADC1_CFG1_DEC_DIV */
    }
    else if (myADC1_Config == myADC1_CFG2)
    {
        resolution = myADC1_CFG2_RESOLUTION;
        #if(myADC1_CFG2_DEC_DIV != 0)
            adcCounts /= myADC1_CFG2_DEC_DIV;
        #endif /* myADC1_CFG2_DEC_DIV */
    }
    else if (myADC1_Config == myADC1_CFG3)
    {
        resolution = myADC1_CFG3_RESOLUTION;
        #if(myADC1_CFG3_DEC_DIV != 0)
            adcCounts /= myADC1_CFG3_DEC_DIV;
        #endif /* myADC1_CFG3_DEC_DIV */
    }
    else
    {
        resolution = myADC1_CFG4_RESOLUTION;
        #if(myADC1_CFG4_DEC_DIV != 0)
            adcCounts /= myADC1_CFG4_DEC_DIV;
        #endif /* myADC1_CFG4_DEC_DIV */
    }

    switch (resolution)
    {
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_12) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_12) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_12) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_12) )
            case (uint8)myADC1__BITS_12:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_2;
                coefB = myADC1_DIVISOR_2;
                break;
        #endif /* myADC1__BITS_12 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_13) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_13) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_13) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_13) )
            case (uint8)myADC1__BITS_13:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_4;
                coefB = myADC1_DIVISOR_4;
                break;
        #endif /* myADC1__BITS_13 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_14) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_14) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_14) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_14) )
            case (uint8)myADC1__BITS_14:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_8;
                coefB = myADC1_DIVISOR_8;
                break;
        #endif /* myADC1__BITS_14 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_15) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_15) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_15) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_15) )
            case (uint8)myADC1__BITS_15:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_16;
                coefB = myADC1_DIVISOR_16;
                break;
        #endif /* myADC1__BITS_15 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_16) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_16) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_16) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_16) )
            case (uint8)myADC1__BITS_16:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_32;
                coefB = myADC1_DIVISOR_32;
                break;
        #endif /* myADC1__BITS_16 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_17) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_17) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_17) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_17) )
            case (uint8)myADC1__BITS_17:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_64;
                coefB = myADC1_DIVISOR_64;
                break;
        #endif /* myADC1__BITS_17 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_18) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_18) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_18) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_18) )
            case (uint8)myADC1__BITS_18:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_125;
                coefB = myADC1_DIVISOR_125;
                break;
        #endif /* myADC1__BITS_18 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_19) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_19) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_19) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_19) )
            case (uint8)myADC1__BITS_19:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_250;
                coefB = myADC1_DIVISOR_250;
                break;
        #endif /* myADC1__BITS_19 */    
        #if( (myADC1_CFG1_RESOLUTION == myADC1__BITS_20) || \
             (myADC1_CFG2_RESOLUTION == myADC1__BITS_20) || \
             (myADC1_CFG3_RESOLUTION == myADC1__BITS_20) || \
             (myADC1_CFG4_RESOLUTION == myADC1__BITS_20) )
            case (uint8)myADC1__BITS_20:
                coefA = myADC1_1UV_COUNTS / myADC1_DIVISOR_500;
                coefB = myADC1_DIVISOR_500;
                break;
        #endif /* myADC1__BITS_20 */    
        default:    /* resolution < 12 */
            /* 11 bits ADC + 2^20(1048576) = 31 bits */
            coefA = myADC1_1UV_COUNTS;
            coefB = myADC1_DIVISOR_1;
            break;
    }
    coefB = myADC1_CountsPerVolt / coefB;
    uVolts = ((coefA * adcCounts) / coefB) - ((coefA * myADC1_Offset) / coefB);

    return( uVolts );
}


/*******************************************************************************
* Function Name: myADC1_InitConfig(uint8 config)
********************************************************************************
*
* Summary:
*  Initializes all registers based on customizer settings
*
* Parameters:
*   void
*
* Return:
*  None
*
* Global variables:
*  myADC1_CountsPerVolt:  Used to set the default counts per volt.
*
* Side Effects: Rewrites the coherency set by myADC1_SetCoherency()
*   API to the default value.
*
*******************************************************************************/
static void myADC1_InitConfig(uint8 config) 
{
    myADC1_stopConversion = 0u;

    if (config == 1u)
    {
        /* Default Config */
        myADC1_DEC_CR_REG      = myADC1_CFG1_DEC_CR;
        myADC1_DEC_SHIFT1_REG  = myADC1_CFG1_DEC_SHIFT1;
        myADC1_DEC_SHIFT2_REG  = myADC1_CFG1_DEC_SHIFT2;
        myADC1_DEC_DR2_REG     = myADC1_CFG1_DEC_DR2;
        myADC1_DEC_DR2H_REG    = myADC1_CFG1_DEC_DR2H;
        myADC1_DEC_DR1_REG     = myADC1_CFG1_DEC_DR1;
        myADC1_DEC_OCOR_REG    = myADC1_CFG1_DEC_OCOR;
        myADC1_DEC_OCORM_REG   = myADC1_CFG1_DEC_OCORM;
        myADC1_DEC_OCORH_REG   = myADC1_CFG1_DEC_OCORH;
        myADC1_DEC_COHER_REG   = myADC1_CFG1_DEC_COHER;

        myADC1_DSM_CR4_REG     = myADC1_CFG1_DSM_CR4;
        myADC1_DSM_CR5_REG     = myADC1_CFG1_DSM_CR5;
        myADC1_DSM_CR6_REG     = myADC1_CFG1_DSM_CR6;
        myADC1_DSM_CR7_REG     = myADC1_CFG1_DSM_CR7;
        myADC1_DSM_CR10_REG    = myADC1_CFG1_DSM_CR10;
        myADC1_DSM_CR11_REG    = myADC1_CFG1_DSM_CR11;
        myADC1_DSM_CR12_REG    = myADC1_CFG1_DSM_CR12;
        myADC1_DSM_CR14_REG    = myADC1_CFG1_DSM_CR14;
        myADC1_DSM_CR15_REG    = myADC1_CFG1_DSM_CR15;
        myADC1_DSM_CR16_REG    = myADC1_CFG1_DSM_CR16;
        myADC1_DSM_CR17_REG    = myADC1_CFG1_DSM_CR17;
        /* Set DSM_REF0_REG by disabling and enabling the PRESS circuit */
        myADC1_SetDSMRef0Reg(myADC1_CFG1_DSM_REF0);
        myADC1_DSM_REF2_REG    = myADC1_CFG1_DSM_REF2;
        myADC1_DSM_REF3_REG    = myADC1_CFG1_DSM_REF3;

        myADC1_DSM_BUF0_REG    = myADC1_CFG1_DSM_BUF0;
        myADC1_DSM_BUF1_REG    = myADC1_CFG1_DSM_BUF1;
        myADC1_DSM_BUF2_REG    = myADC1_CFG1_DSM_BUF2;
        myADC1_DSM_BUF3_REG    = myADC1_CFG1_DSM_BUF3;

        /* To select either Vssa or Vref to -ve input of DSM depending on
        *  the input  range selected.
        */
        #if(myADC1_DEFAULT_INPUT_MODE)
            #if (myADC1_CFG1_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF)
                myADC1_AMux_Select(1u);
            #else
                myADC1_AMux_Select(0u);
            #endif /* myADC1_IR_VSSA_TO_2VREF) */
        #endif /* myADC1_DEFAULT_INPUT_MODE */

        /* Set the Conversion stop if resolution is above 16 bit and conversion
        *   mode is Single sample
        */
        #if(myADC1_CFG1_RESOLUTION > 16 && \
            myADC1_CFG1_CONV_MODE == myADC1_MODE_SINGLE_SAMPLE)
            myADC1_stopConversion = 1u;
        #endif /* Single sample with resolution above 16 bits. */

        myADC1_CountsPerVolt = (int32)myADC1_CFG1_COUNTS_PER_VOLT;

        myADC1_Ext_CP_Clk_SetDividerRegister(myADC1_CFG1_CP_CLK_DIVIDER, 1u);

        /* This is only valid if there is an internal clock */
        #if(myADC1_DEFAULT_INTERNAL_CLK)
            myADC1_theACLK_SetDividerRegister(myADC1_CFG1_ADC_CLK_DIVIDER, 1u);
        #endif /* myADC1_DEFAULT_INTERNAL_CLK */

        #if(myADC1_IRQ_REMOVE == 0u)
            /* Set interrupt vector */
            (void)CyIntSetVector(myADC1_INTC_NUMBER, &myADC1_ISR1);
        #endif   /* End myADC1_IRQ_REMOVE */
    }

    #if(myADC1_DEFAULT_NUM_CONFIGS > 1)
        if(config == 2u)
        {
            /* Second Config */
            myADC1_DEC_CR_REG      = myADC1_CFG2_DEC_CR;
            myADC1_DEC_SHIFT1_REG  = myADC1_CFG2_DEC_SHIFT1;
            myADC1_DEC_SHIFT2_REG  = myADC1_CFG2_DEC_SHIFT2;
            myADC1_DEC_DR2_REG     = myADC1_CFG2_DEC_DR2;
            myADC1_DEC_DR2H_REG    = myADC1_CFG2_DEC_DR2H;
            myADC1_DEC_DR1_REG     = myADC1_CFG2_DEC_DR1;
            myADC1_DEC_OCOR_REG    = myADC1_CFG2_DEC_OCOR;
            myADC1_DEC_OCORM_REG   = myADC1_CFG2_DEC_OCORM;
            myADC1_DEC_OCORH_REG   = myADC1_CFG2_DEC_OCORH;
            myADC1_DEC_COHER_REG   = myADC1_CFG2_DEC_COHER;

            myADC1_DSM_CR4_REG     = myADC1_CFG2_DSM_CR4;
            myADC1_DSM_CR5_REG     = myADC1_CFG2_DSM_CR5;
            myADC1_DSM_CR6_REG     = myADC1_CFG2_DSM_CR6;
            myADC1_DSM_CR7_REG     = myADC1_CFG2_DSM_CR7;
            myADC1_DSM_CR10_REG    = myADC1_CFG2_DSM_CR10;
            myADC1_DSM_CR11_REG    = myADC1_CFG2_DSM_CR11;
            myADC1_DSM_CR12_REG    = myADC1_CFG2_DSM_CR12;
            myADC1_DSM_CR14_REG    = myADC1_CFG2_DSM_CR14;
            myADC1_DSM_CR15_REG    = myADC1_CFG2_DSM_CR15;
            myADC1_DSM_CR16_REG    = myADC1_CFG2_DSM_CR16;
            myADC1_DSM_CR17_REG    = myADC1_CFG2_DSM_CR17;
            /* Set DSM_REF0_REG by disabling and enabling the PRESS cirucit */
            myADC1_SetDSMRef0Reg(myADC1_CFG2_DSM_REF0);
            myADC1_DSM_REF2_REG    = myADC1_CFG2_DSM_REF2;
            myADC1_DSM_REF3_REG    = myADC1_CFG2_DSM_REF3;

            myADC1_DSM_BUF0_REG    = myADC1_CFG2_DSM_BUF0;
            myADC1_DSM_BUF1_REG    = myADC1_CFG2_DSM_BUF1;
            myADC1_DSM_BUF2_REG    = myADC1_CFG2_DSM_BUF2;
            myADC1_DSM_BUF3_REG    = myADC1_CFG2_DSM_BUF3;

            /* To select either Vssa or Vref to -ve input of DSM depending on
            *  the input range selected.
            */

            #if(myADC1_DEFAULT_INPUT_MODE)
                #if (myADC1_CFG2_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF)
                    myADC1_AMux_Select(1u);
                #else
                    myADC1_AMux_Select(0u);
                #endif /* myADC1_IR_VSSA_TO_2VREF) */
            #endif /* myADC1_DEFAULT_INPUT_MODE */

            /* Set the Conversion stop if resolution is above 16 bit and
            *   conversion mode is Single sample
            */
            #if(myADC1_CFG2_RESOLUTION > 16 && \
                myADC1_CFG2_CONV_MODE == myADC1_MODE_SINGLE_SAMPLE)
                myADC1_stopConversion = 1u;
            #endif /* Single sample with resolution above 16 bits. */

            myADC1_CountsPerVolt = (int32)myADC1_CFG2_COUNTS_PER_VOLT;

            myADC1_Ext_CP_Clk_SetDividerRegister(myADC1_CFG2_CP_CLK_DIVIDER, 1u);

            /* This is only valid if there is an internal clock */
            #if(myADC1_DEFAULT_INTERNAL_CLK)
                myADC1_theACLK_SetDividerRegister(myADC1_CFG2_ADC_CLK_DIVIDER, 1u);
            #endif /* myADC1_DEFAULT_INTERNAL_CLK */

            #if(myADC1_IRQ_REMOVE == 0u)
                /* Set interrupt vector */
                (void)CyIntSetVector(myADC1_INTC_NUMBER, &myADC1_ISR2);
            #endif   /* End myADC1_IRQ_REMOVE */
        }
    #endif /* myADC1_DEFAULT_NUM_CONFIGS > 1 */

    #if(myADC1_DEFAULT_NUM_CONFIGS > 2)
        if(config == 3u)
        {
            /* Third Config */
            myADC1_DEC_CR_REG      = myADC1_CFG3_DEC_CR;
            myADC1_DEC_SHIFT1_REG  = myADC1_CFG3_DEC_SHIFT1;
            myADC1_DEC_SHIFT2_REG  = myADC1_CFG3_DEC_SHIFT2;
            myADC1_DEC_DR2_REG     = myADC1_CFG3_DEC_DR2;
            myADC1_DEC_DR2H_REG    = myADC1_CFG3_DEC_DR2H;
            myADC1_DEC_DR1_REG     = myADC1_CFG3_DEC_DR1;
            myADC1_DEC_OCOR_REG    = myADC1_CFG3_DEC_OCOR;
            myADC1_DEC_OCORM_REG   = myADC1_CFG3_DEC_OCORM;
            myADC1_DEC_OCORH_REG   = myADC1_CFG3_DEC_OCORH;
            myADC1_DEC_COHER_REG   = myADC1_CFG3_DEC_COHER;

            myADC1_DSM_CR4_REG     = myADC1_CFG3_DSM_CR4;
            myADC1_DSM_CR5_REG     = myADC1_CFG3_DSM_CR5;
            myADC1_DSM_CR6_REG     = myADC1_CFG3_DSM_CR6;
            myADC1_DSM_CR7_REG     = myADC1_CFG3_DSM_CR7;
            myADC1_DSM_CR10_REG    = myADC1_CFG3_DSM_CR10;
            myADC1_DSM_CR11_REG    = myADC1_CFG3_DSM_CR11;
            myADC1_DSM_CR12_REG    = myADC1_CFG3_DSM_CR12;
            myADC1_DSM_CR14_REG    = myADC1_CFG3_DSM_CR14;
            myADC1_DSM_CR15_REG    = myADC1_CFG3_DSM_CR15;
            myADC1_DSM_CR16_REG    = myADC1_CFG3_DSM_CR16;
            myADC1_DSM_CR17_REG    = myADC1_CFG3_DSM_CR17;
            /* Set DSM_REF0_REG by disabling and enabling the PRESS circuit */
            myADC1_SetDSMRef0Reg(myADC1_CFG3_DSM_REF0);
            myADC1_DSM_REF2_REG    = myADC1_CFG3_DSM_REF2;
            myADC1_DSM_REF3_REG    = myADC1_CFG3_DSM_REF3;

            myADC1_DSM_BUF0_REG    = myADC1_CFG3_DSM_BUF0;
            myADC1_DSM_BUF1_REG    = myADC1_CFG3_DSM_BUF1;
            myADC1_DSM_BUF2_REG    = myADC1_CFG3_DSM_BUF2;
            myADC1_DSM_BUF3_REG    = myADC1_CFG3_DSM_BUF3;

            /* To select either Vssa or Vref to -ve input of DSM depending on
            *  the input range selected.
            */
            #if(myADC1_DEFAULT_INPUT_MODE)
                #if (myADC1_CFG3_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF)
                    myADC1_AMux_Select(1u);
                #else
                    myADC1_AMux_Select(0u);
                #endif /* myADC1_IR_VSSA_TO_2VREF) */
            #endif /* myADC1_DEFAULT_INPUT_MODE */

            /* Set the Conversion stop if resolution is above 16 bit and
               conversion  mode is Single sample */
            #if(myADC1_CFG3_RESOLUTION > 16 && \
                myADC1_CFG3_CONV_MODE == myADC1_MODE_SINGLE_SAMPLE)
                myADC1_stopConversion = 1u;
            #endif /* Single sample with resolution above 16 bits */

            myADC1_CountsPerVolt = (int32)myADC1_CFG3_COUNTS_PER_VOLT;

            myADC1_Ext_CP_Clk_SetDividerRegister(myADC1_CFG3_CP_CLK_DIVIDER, 1u);

            /* This is only valid if there is an internal clock */
            #if(myADC1_DEFAULT_INTERNAL_CLK)
                myADC1_theACLK_SetDividerRegister(myADC1_CFG3_ADC_CLK_DIVIDER, 1u);
            #endif /* myADC1_DEFAULT_INTERNAL_CLK */

            #if(myADC1_IRQ_REMOVE == 0u)
                /* Set interrupt vector */
                (void)CyIntSetVector(myADC1_INTC_NUMBER, &myADC1_ISR3);
            #endif   /* End myADC1_IRQ_REMOVE */
        }
    #endif /* myADC1_DEFAULT_NUM_CONFIGS > 2 */

    #if(myADC1_DEFAULT_NUM_CONFIGS > 3)
        if (config == 4u)
        {
            /* Fourth Config */
            myADC1_DEC_CR_REG      = myADC1_CFG4_DEC_CR;
            myADC1_DEC_SHIFT1_REG  = myADC1_CFG4_DEC_SHIFT1;
            myADC1_DEC_SHIFT2_REG  = myADC1_CFG4_DEC_SHIFT2;
            myADC1_DEC_DR2_REG     = myADC1_CFG4_DEC_DR2;
            myADC1_DEC_DR2H_REG    = myADC1_CFG4_DEC_DR2H;
            myADC1_DEC_DR1_REG     = myADC1_CFG4_DEC_DR1;
            myADC1_DEC_OCOR_REG    = myADC1_CFG4_DEC_OCOR;
            myADC1_DEC_OCORM_REG   = myADC1_CFG4_DEC_OCORM;
            myADC1_DEC_OCORH_REG   = myADC1_CFG4_DEC_OCORH;
            myADC1_DEC_COHER_REG   = myADC1_CFG4_DEC_COHER;

            myADC1_DSM_CR4_REG     = myADC1_CFG4_DSM_CR4;
            myADC1_DSM_CR5_REG     = myADC1_CFG4_DSM_CR5;
            myADC1_DSM_CR6_REG     = myADC1_CFG4_DSM_CR6;
            myADC1_DSM_CR7_REG     = myADC1_CFG4_DSM_CR7;
            myADC1_DSM_CR10_REG    = myADC1_CFG4_DSM_CR10;
            myADC1_DSM_CR11_REG    = myADC1_CFG4_DSM_CR11;
            myADC1_DSM_CR12_REG    = myADC1_CFG4_DSM_CR12;
            myADC1_DSM_CR14_REG    = myADC1_CFG4_DSM_CR14;
            myADC1_DSM_CR15_REG    = myADC1_CFG4_DSM_CR15;
            myADC1_DSM_CR16_REG    = myADC1_CFG4_DSM_CR16;
            myADC1_DSM_CR17_REG    = myADC1_CFG4_DSM_CR17;
            /* Set DSM_REF0_REG by disabling and enabling the PRESS circuit */
            myADC1_SetDSMRef0Reg(myADC1_CFG4_DSM_REF0);
            myADC1_DSM_REF2_REG    = myADC1_CFG4_DSM_REF2;
            myADC1_DSM_REF3_REG    = myADC1_CFG4_DSM_REF3;

            myADC1_DSM_BUF0_REG    = myADC1_CFG4_DSM_BUF0;
            myADC1_DSM_BUF1_REG    = myADC1_CFG4_DSM_BUF1;
            myADC1_DSM_BUF2_REG    = myADC1_CFG4_DSM_BUF2;
            myADC1_DSM_BUF3_REG    = myADC1_CFG4_DSM_BUF3;

            /* To select either Vssa or Vref to -ve input of DSM depending on
            *  the input range selected.
            */
            #if(myADC1_DEFAULT_INPUT_MODE)
                #if (myADC1_CFG4_INPUT_RANGE == myADC1_IR_VSSA_TO_2VREF)
                    myADC1_AMux_Select(1u);
                #else
                    myADC1_AMux_Select(0u);
                #endif /* myADC1_IR_VSSA_TO_2VREF) */
            #endif /* myADC1_DEFAULT_INPUT_MODE */

            /* Set the Conversion stop if resolution is above 16 bit and
               conversion mode is Single sample */
            #if(myADC1_CFG4_RESOLUTION > 16 && \
                myADC1_CFG4_CONV_MODE == myADC1_MODE_SINGLE_SAMPLE)
                myADC1_stopConversion = 1u;
            #endif /* Single sample with resolution above 16 bits */

            myADC1_CountsPerVolt = (int32)myADC1_CFG4_COUNTS_PER_VOLT;

            myADC1_Ext_CP_Clk_SetDividerRegister(myADC1_CFG4_CP_CLK_DIVIDER, 1u);

            /* This is only valid if there is an internal clock */
            #if(myADC1_DEFAULT_INTERNAL_CLK)
                myADC1_theACLK_SetDividerRegister(myADC1_CFG4_ADC_CLK_DIVIDER, 1u);
            #endif /* myADC1_DEFAULT_INTERNAL_CLK */

            #if(myADC1_IRQ_REMOVE == 0u)
                /* Set interrupt vector */
                (void)CyIntSetVector(myADC1_INTC_NUMBER, &myADC1_ISR4);
            #endif   /* End myADC1_IRQ_REMOVE */
        }
    #endif /* myADC1_DEFAULT_NUM_CONFIGS > 3 */
}


/*******************************************************************************
* Function Name: myADC1_SelectCofiguration
********************************************************************************
*
* Summary:
*  Sets one of up to four ADC configurations. Before setting the new
*  configuration, the ADC is stopped and powered down. After setting
*  the new configuration, the ADC can be powered and conversion
*  can be restarted depending up on the value of second parameter
*  restart. If the value of this parameter is 1, then ADC will be
*  restarted. If this value is zero, then user must call myADC1_Start
*  and myADC1_StartConvert() to restart the conversion.
*
* Parameters:
*  config:  configuration user wants to select.
*           Valid range: 1..4
*  restart:  Restart option. 1 means start the ADC and restart the conversion.
*                            0 means do not start the ADC and conversion.
*
* Return:
*  None
*
* Side Effects: 
*  Enables internal interrupt.
*
*******************************************************************************/
void myADC1_SelectConfiguration(uint8 config, uint8 restart)
                                              
{
    /* Check whether the configuration number is valid or not */
    if((config > 0u) && (config <= myADC1_DEFAULT_NUM_CONFIGS))
    {
        /* Set the flag to ensure Start() API doesn't override the 
		*  selected configuration
		*/
        if(myADC1_initVar == 0u)
        {
            myADC1_started = 1u;
        }

        /* Update the config flag */
        myADC1_Config = config;

        /* Stop the ADC  */
        myADC1_Stop();

        /* Set the  ADC registers based on the configuration */
        myADC1_InitConfig(config);

        /* Compensate the gain */
        myADC1_DEC_GVAL_REG = myADC1_gcor[config - 1u].gval;
        CY_SET_REG16(myADC1_DEC_GCOR_16B_PTR, myADC1_gcor[config - 1u].gcor);

        if(restart == 1u)
        {
            /* Restart the ADC */
            myADC1_Start();

            /* Restart the ADC conversion */
            myADC1_StartConvert();
        }
    }
    else
    {
        /* Halt CPU in debug mode if config is out of valid range */
        CYASSERT(0u != 0u);
    }
}


/*******************************************************************************
* Function Name: myADC1_GainCompensation
********************************************************************************
*
* Summary:
*  This API calculates the trim value and then store this to gcor structure.
*
* Parameters:
*  inputRange:  input range for which trim value is to be calculated.
*  IdealDecGain:  Ideal Decimator gain for the selected resolution and
*                 conversion  mode.
*  IdealOddDecGain:  Ideal odd decimation gain for the selected resolution and
                     conversion mode.
*  resolution:  Resolution to select the proper flash location for trim value.
*  config:      Specifies the configuration number
*               Valid range: 1..4
*
* Return:
*  None
*
*******************************************************************************/
static void myADC1_GainCompensation(uint8 inputRange, uint16 idealDecGain, uint16 idealOddDecGain,
                              uint8 resolution, uint8 config) 
{
    int8 flash;
	int32 normalised;
	uint16 gcorValue;
    uint32 gcorTmp;

    if((config > 0u) && (config <= myADC1_DEFAULT_NUM_CONFIGS))
    {
        switch(inputRange)
        {
            case myADC1_IR_VNEG_VREF_DIFF:
            case myADC1_IR_VSSA_TO_2VREF:
                /* Normalize the flash Value */
                if(resolution > 15u)
                {
                    flash = myADC1_DEC_TRIM_VREF_DIFF_16_20;
                }
                else
                {
                    flash = myADC1_DEC_TRIM_VREF_DIFF_8_15;
                }
                break;

            case myADC1_IR_VNEG_VREF_2_DIFF:
                /* Normalize the flash Value */
                if(resolution > 15u)
                {
                    flash = myADC1_DEC_TRIM_VREF_2_DIFF_16_20;
                }
                else
                {
                    flash = myADC1_DEC_TRIM_VREF_2_DIFF_8_15;
                }
                break;

            case myADC1_IR_VNEG_VREF_4_DIFF:
                /* Normalize the flash Value */
                if(resolution > 15u)
                {
                    flash = myADC1_DEC_TRIM_VREF_4_DIFF_16_20;
                }
                else
                {
                    flash = myADC1_DEC_TRIM_VREF_4_DIFF_8_15;
                }
                break;

            case myADC1_IR_VNEG_VREF_16_DIFF:
                /* Normalize the flash Value */
                if(resolution > 15u)
                {
                    flash = myADC1_DEC_TRIM_VREF_16_DIFF_16_20;
                }
                else
                {
                    flash = myADC1_DEC_TRIM_VREF_16_DIFF_8_15;
                }
                break;

            default:
                flash = 0;
                break;
        }

        /* Add two values */
		normalised = (int32)idealDecGain + ((int32)flash * 32);
        gcorTmp = (uint32)normalised * (uint32)idealOddDecGain;
        gcorValue = (uint16)(gcorTmp / myADC1_IDEAL_GAIN_CONST);

        if (resolution < (myADC1_MAX_GVAL - 1u))
        {
            gcorValue = (gcorValue >> (myADC1_MAX_GVAL - (resolution + 1u)));
            myADC1_gcor[config - 1u].gval = (resolution + 1u);
        }
        else
        {
            /* Use all 16 bits */
            myADC1_gcor[config - 1u].gval = myADC1_MAX_GVAL;
        }

        /* Save the gain correction register value */
        myADC1_gcor[config - 1u].gcor = gcorValue;
    }
    else
    {
        /* Halt CPU in debug mode if config is out of valid range */
        CYASSERT(0u != 0u);
    }
}


/******************************************************************************
* Function Name: myADC1_SetDSMRef0Reg(uint8)
******************************************************************************
*
* Summary:
*  This API sets the DSM_REF0 register. This is written for internal use.
*
* Parameters:
*  value:  Value to be written to DSM_REF0 register.
*
* Return:
*  None
*
******************************************************************************/
static void myADC1_SetDSMRef0Reg(uint8 value) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Disable PRES, Enable power to VCMBUF0, REFBUF0 and REFBUF1, enable PRES */
    myADC1_RESET_CR4_REG |= (myADC1_IGNORE_PRESA1 | myADC1_IGNORE_PRESD1);
    myADC1_RESET_CR5_REG |= (myADC1_IGNORE_PRESA2 | myADC1_IGNORE_PRESD2);
    myADC1_DSM_REF0_REG = value;

    /* Wait for 3 microseconds */
    CyDelayUs(myADC1_PRES_DELAY_TIME);
    /* Enable the press circuit */
    myADC1_RESET_CR4_REG &= (uint8)~(myADC1_IGNORE_PRESA1 | myADC1_IGNORE_PRESD1);
    myADC1_RESET_CR5_REG &= (uint8)~(myADC1_IGNORE_PRESA2 | myADC1_IGNORE_PRESD2);

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: myADC1_Read8
********************************************************************************
*
* Summary:
*  This function simplifies getting results from the ADC when only a
*  single reading is required. When called, it will start ADC
*  conversions, wait for the conversion to be complete, stop ADC
*  conversion and return the result. This is a blocking function and will
*  not return until the result is ready.
*
* Parameters:
*  None
*
* Return:
*  int8:  ADC result.
*
*******************************************************************************/
int8 myADC1_Read8(void) 
{
    int8 result;

    /* Clear pending conversion done status */
    myADC1_DEC_SR_REG |= myADC1_DEC_INTR_CLEAR;
    myADC1_StartConvert();
    (void)myADC1_IsEndConversion(myADC1_WAIT_FOR_RESULT);
     result = myADC1_GetResult8();
    myADC1_StopConvert();
    
     return(result);
}


/*******************************************************************************
* Function Name: myADC1_Read16
********************************************************************************
*
* Summary:
*  This function simplifies getting results from the ADC when only a
*  single reading is required. When called, it will start ADC
*  conversions, wait for the conversion to be complete, stop ADC
*  conversion and return the result. This is a blocking function and will
*  not return until the result is ready.
*
* Parameters:
*   void
*
* Return:
*  int16:  ADC result.
*
*******************************************************************************/
int16 myADC1_Read16(void) 
{
    int16 result;

    /* Clear pending conversion done status */
    myADC1_DEC_SR_REG |= myADC1_DEC_INTR_CLEAR;
    myADC1_StartConvert();
    (void)myADC1_IsEndConversion(myADC1_WAIT_FOR_RESULT);
     result = myADC1_GetResult16();
    myADC1_StopConvert();
    
     return(result);
}


/*******************************************************************************
* Function Name: myADC1_Read32
********************************************************************************
*
* Summary:
*  This function simplifies getting results from the ADC when only a
*  single reading is required. When called, it will start ADC
*  conversions, wait for the conversion to be complete, stop ADC
*  conversion and return the result. This is a blocking function and will
*  not return until the result is ready.
*
* Parameters:
*  None
*
* Return:
*  int32: ADC result.
*
*******************************************************************************/
int32 myADC1_Read32(void) 
{
    int32 result;
    
    /* Clear pending conversion done status */
    myADC1_DEC_SR_REG |= myADC1_DEC_INTR_CLEAR;
    myADC1_StartConvert();
    (void)myADC1_IsEndConversion(myADC1_WAIT_FOR_RESULT);
     result = myADC1_GetResult32();
    myADC1_StopConvert();
    
     return(result);
}


/* [] END OF FILE */
