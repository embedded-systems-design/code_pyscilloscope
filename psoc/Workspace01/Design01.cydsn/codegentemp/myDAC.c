/*******************************************************************************
* File Name: myDAC.c  
* Version 1.90
*
* Description:
*  This file provides the source code to the API for the 8-bit Voltage DAC 
*  (VDAC8) User Module.
*
* Note:
*  Any unusual or non-standard behavior should be noted here. Other-
*  wise, this section should remain blank.
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "myDAC.h"

#if (CY_PSOC5A)
#include <CyLib.h>
#endif /* CY_PSOC5A */

uint8 myDAC_initVar = 0u;

#if (CY_PSOC5A)
    static uint8 myDAC_restoreVal = 0u;
#endif /* CY_PSOC5A */

#if (CY_PSOC5A)
    static myDAC_backupStruct myDAC_backup;
#endif /* CY_PSOC5A */


/*******************************************************************************
* Function Name: myDAC_Init
********************************************************************************
* Summary:
*  Initialize to the schematic state.
* 
* Parameters:
*  void:
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void myDAC_Init(void) 
{
    myDAC_CR0 = (myDAC_MODE_V );

    /* Set default data source */
    #if(myDAC_DEFAULT_DATA_SRC != 0 )
        myDAC_CR1 = (myDAC_DEFAULT_CNTL | myDAC_DACBUS_ENABLE) ;
    #else
        myDAC_CR1 = (myDAC_DEFAULT_CNTL | myDAC_DACBUS_DISABLE) ;
    #endif /* (myDAC_DEFAULT_DATA_SRC != 0 ) */

    /* Set default strobe mode */
    #if(myDAC_DEFAULT_STRB != 0)
        myDAC_Strobe |= myDAC_STRB_EN ;
    #endif/* (myDAC_DEFAULT_STRB != 0) */

    /* Set default range */
    myDAC_SetRange(myDAC_DEFAULT_RANGE); 

    /* Set default speed */
    myDAC_SetSpeed(myDAC_DEFAULT_SPEED);
}


/*******************************************************************************
* Function Name: myDAC_Enable
********************************************************************************
* Summary:
*  Enable the VDAC8
* 
* Parameters:
*  void
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void myDAC_Enable(void) 
{
    myDAC_PWRMGR |= myDAC_ACT_PWR_EN;
    myDAC_STBY_PWRMGR |= myDAC_STBY_PWR_EN;

    /*This is to restore the value of register CR0 ,
    which is modified  in Stop API , this prevents misbehaviour of VDAC */
    #if (CY_PSOC5A)
        if(myDAC_restoreVal == 1u) 
        {
             myDAC_CR0 = myDAC_backup.data_value;
             myDAC_restoreVal = 0u;
        }
    #endif /* CY_PSOC5A */
}


/*******************************************************************************
* Function Name: myDAC_Start
********************************************************************************
*
* Summary:
*  The start function initializes the voltage DAC with the default values, 
*  and sets the power to the given level.  A power level of 0, is the same as
*  executing the stop function.
*
* Parameters:
*  Power: Sets power level between off (0) and (3) high power
*
* Return:
*  void 
*
* Global variables:
*  myDAC_initVar: Is modified when this function is called for the 
*  first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void myDAC_Start(void)  
{
    /* Hardware initiazation only needs to occure the first time */
    if(myDAC_initVar == 0u)
    { 
        myDAC_Init();
        myDAC_initVar = 1u;
    }

    /* Enable power to DAC */
    myDAC_Enable();

    /* Set default value */
    myDAC_SetValue(myDAC_DEFAULT_DATA); 
}


/*******************************************************************************
* Function Name: myDAC_Stop
********************************************************************************
*
* Summary:
*  Powers down DAC to lowest power state.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void myDAC_Stop(void) 
{
    /* Disble power to DAC */
    myDAC_PWRMGR &= (uint8)(~myDAC_ACT_PWR_EN);
    myDAC_STBY_PWRMGR &= (uint8)(~myDAC_STBY_PWR_EN);

    /* This is a work around for PSoC5A  ,
    this sets VDAC to current mode with output off */
    #if (CY_PSOC5A)
        myDAC_backup.data_value = myDAC_CR0;
        myDAC_CR0 = myDAC_CUR_MODE_OUT_OFF;
        myDAC_restoreVal = 1u;
    #endif /* CY_PSOC5A */
}


/*******************************************************************************
* Function Name: myDAC_SetSpeed
********************************************************************************
*
* Summary:
*  Set DAC speed
*
* Parameters:
*  power: Sets speed value
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void myDAC_SetSpeed(uint8 speed) 
{
    /* Clear power mask then write in new value */
    myDAC_CR0 &= (uint8)(~myDAC_HS_MASK);
    myDAC_CR0 |=  (speed & myDAC_HS_MASK);
}


/*******************************************************************************
* Function Name: myDAC_SetRange
********************************************************************************
*
* Summary:
*  Set one of three current ranges.
*
* Parameters:
*  Range: Sets one of Three valid ranges.
*
* Return:
*  void 
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void myDAC_SetRange(uint8 range) 
{
    myDAC_CR0 &= (uint8)(~myDAC_RANGE_MASK);      /* Clear existing mode */
    myDAC_CR0 |= (range & myDAC_RANGE_MASK);      /*  Set Range  */
    myDAC_DacTrim();
}


/*******************************************************************************
* Function Name: myDAC_SetValue
********************************************************************************
*
* Summary:
*  Set 8-bit DAC value
*
* Parameters:  
*  value:  Sets DAC value between 0 and 255.
*
* Return: 
*  void 
*
* Theory: 
*
* Side Effects:
*
*******************************************************************************/
void myDAC_SetValue(uint8 value) 
{
    #if (CY_PSOC5A)
        uint8 myDAC_intrStatus = CyEnterCriticalSection();
    #endif /* CY_PSOC5A */

    myDAC_Data = value;                /*  Set Value  */

    /* PSOC5A requires a double write */
    /* Exit Critical Section */
    #if (CY_PSOC5A)
        myDAC_Data = value;
        CyExitCriticalSection(myDAC_intrStatus);
    #endif /* CY_PSOC5A */
}


/*******************************************************************************
* Function Name: myDAC_DacTrim
********************************************************************************
*
* Summary:
*  Set the trim value for the given range.
*
* Parameters:
*  range:  1V or 4V range.  See constants.
*
* Return:
*  void
*
* Theory: 
*
* Side Effects:
*
*******************************************************************************/
void myDAC_DacTrim(void) 
{
    uint8 mode;

    mode = (uint8)((myDAC_CR0 & myDAC_RANGE_MASK) >> 2) + myDAC_TRIM_M7_1V_RNG_OFFSET;
    myDAC_TR = CY_GET_XTND_REG8((uint8 *)(myDAC_DAC_TRIM_BASE + mode));
}


/* [] END OF FILE */
