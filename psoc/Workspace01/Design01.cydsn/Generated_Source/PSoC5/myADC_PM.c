/*******************************************************************************
* File Name: myADC_PM.c
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

#include "myADC.h"


/***************************************
* Local data allocation
***************************************/

static myADC_BACKUP_STRUCT  myADC_backup =
{
    myADC_DISABLED
};


/*******************************************************************************
* Function Name: myADC_SaveConfig
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
void myADC_SaveConfig(void)
{
    /* All configuration registers are marked as [reset_all_retention] */
}


/*******************************************************************************
* Function Name: myADC_RestoreConfig
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
void myADC_RestoreConfig(void)
{
    /* All congiguration registers are marked as [reset_all_retention] */
}


/*******************************************************************************
* Function Name: myADC_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred routine to prepare the component for sleep.
*  The myADC_Sleep() routine saves the current component state,
*  then it calls the ADC_Stop() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  myADC_backup - The structure field 'enableState' is modified
*  depending on the enable state of the block before entering to sleep mode.
*
*******************************************************************************/
void myADC_Sleep(void)
{
    if((myADC_PWRMGR_SAR_REG  & myADC_ACT_PWR_SAR_EN) != 0u)
    {
        if((myADC_SAR_CSR0_REG & myADC_SAR_SOF_START_CONV) != 0u)
        {
            myADC_backup.enableState = myADC_ENABLED | myADC_STARTED;
        }
        else
        {
            myADC_backup.enableState = myADC_ENABLED;
        }
        myADC_Stop();
    }
    else
    {
        myADC_backup.enableState = myADC_DISABLED;
    }
}


/*******************************************************************************
* Function Name: myADC_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred routine to restore the component to the state when
*  myADC_Sleep() was called. If the component was enabled before the
*  myADC_Sleep() function was called, the
*  myADC_Wakeup() function also re-enables the component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  myADC_backup - The structure field 'enableState' is used to
*  restore the enable state of block after wakeup from sleep mode.
*
*******************************************************************************/
void myADC_Wakeup(void)
{
    if(myADC_backup.enableState != myADC_DISABLED)
    {
        myADC_Enable();
        #if(myADC_DEFAULT_CONV_MODE != myADC__HARDWARE_TRIGGER)
            if((myADC_backup.enableState & myADC_STARTED) != 0u)
            {
                myADC_StartConvert();
            }
        #endif /* End myADC_DEFAULT_CONV_MODE != myADC__HARDWARE_TRIGGER */
    }
}


/* [] END OF FILE */
