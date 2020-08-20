/*******************************************************************************
* File Name: myADC_SEQ_SAR_PM.c
* Version 3.10
*
* Description:
*  This file provides Sleep/WakeUp APIs functionality.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myADC_SEQ_SAR.h"


/***************************************
* Local data allocation
***************************************/

static myADC_SEQ_SAR_BACKUP_STRUCT  myADC_SEQ_SAR_backup =
{
    myADC_SEQ_SAR_DISABLED
};


/*******************************************************************************
* Function Name: myADC_SEQ_SAR_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void myADC_SEQ_SAR_SaveConfig(void)
{
    /* All configuration registers are marked as [reset_all_retention] */
}


/*******************************************************************************
* Function Name: myADC_SEQ_SAR_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void myADC_SEQ_SAR_RestoreConfig(void)
{
    /* All congiguration registers are marked as [reset_all_retention] */
}


/*******************************************************************************
* Function Name: myADC_SEQ_SAR_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred routine to prepare the component for sleep.
*  The myADC_SEQ_SAR_Sleep() routine saves the current component state,
*  then it calls the ADC_Stop() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  myADC_SEQ_SAR_backup - The structure field 'enableState' is modified
*  depending on the enable state of the block before entering to sleep mode.
*
*******************************************************************************/
void myADC_SEQ_SAR_Sleep(void)
{
    if((myADC_SEQ_SAR_PWRMGR_SAR_REG  & myADC_SEQ_SAR_ACT_PWR_SAR_EN) != 0u)
    {
        if((myADC_SEQ_SAR_SAR_CSR0_REG & myADC_SEQ_SAR_SAR_SOF_START_CONV) != 0u)
        {
            myADC_SEQ_SAR_backup.enableState = myADC_SEQ_SAR_ENABLED | myADC_SEQ_SAR_STARTED;
        }
        else
        {
            myADC_SEQ_SAR_backup.enableState = myADC_SEQ_SAR_ENABLED;
        }
        myADC_SEQ_SAR_Stop();
    }
    else
    {
        myADC_SEQ_SAR_backup.enableState = myADC_SEQ_SAR_DISABLED;
    }
}


/*******************************************************************************
* Function Name: myADC_SEQ_SAR_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred routine to restore the component to the state when
*  myADC_SEQ_SAR_Sleep() was called. If the component was enabled before the
*  myADC_SEQ_SAR_Sleep() function was called, the
*  myADC_SEQ_SAR_Wakeup() function also re-enables the component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  myADC_SEQ_SAR_backup - The structure field 'enableState' is used to
*  restore the enable state of block after wakeup from sleep mode.
*
*******************************************************************************/
void myADC_SEQ_SAR_Wakeup(void)
{
    if(myADC_SEQ_SAR_backup.enableState != myADC_SEQ_SAR_DISABLED)
    {
        myADC_SEQ_SAR_Enable();
        #if(myADC_SEQ_SAR_DEFAULT_CONV_MODE != myADC_SEQ_SAR__HARDWARE_TRIGGER)
            if((myADC_SEQ_SAR_backup.enableState & myADC_SEQ_SAR_STARTED) != 0u)
            {
                myADC_SEQ_SAR_StartConvert();
            }
        #endif /* End myADC_SEQ_SAR_DEFAULT_CONV_MODE != myADC_SEQ_SAR__HARDWARE_TRIGGER */
    }
}


/* [] END OF FILE */
