/*******************************************************************************
* File Name: myADC1_PM.c
* Version 3.30
*
* Description:
*  This file provides the power manager source code to the API for the
*  Delta-Sigma ADC Component.
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

static myADC1_BACKUP_STRUCT myADC1_backup =
{
    myADC1_DISABLED,
    myADC1_CFG1_DEC_CR
};


/*******************************************************************************
* Function Name: myADC1_SaveConfig
********************************************************************************
*
* Summary:
*  Save the register configuration which are not retention.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  myADC1_backup:  This global structure variable is used to store
*  configuration registers which are non retention whenever user wants to go
*  sleep mode by calling Sleep() API.
*
*******************************************************************************/
void myADC1_SaveConfig(void) 
{
    myADC1_backup.deccr = myADC1_DEC_CR_REG;
}


/*******************************************************************************
* Function Name: myADC1_RestoreConfig
********************************************************************************
*
* Summary:
*  Restore the register configurations which are not retention.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  myADC1_backup:  This global structure variable is used to restore
*  configuration registers which are non retention whenever user wants to switch
*  to active power mode by calling Wakeup() API.
*
*******************************************************************************/
void myADC1_RestoreConfig(void) 
{
    myADC1_DEC_CR_REG = myADC1_backup.deccr;
}


/*******************************************************************************
* Function Name: myADC1_Sleep
********************************************************************************
*
* Summary:
*  Stops the operation of the block and saves the user configuration.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  myADC1_backup:  The structure field 'enableState' is modified
*  depending on the enable state of the block before entering to sleep mode.
*
*******************************************************************************/
void myADC1_Sleep(void) 
{
    /* Save ADC enable state */
    if((myADC1_ACT_PWR_DEC_EN == (myADC1_PWRMGR_DEC_REG & myADC1_ACT_PWR_DEC_EN)) &&
       (myADC1_ACT_PWR_DSM_EN == (myADC1_PWRMGR_DSM_REG & myADC1_ACT_PWR_DSM_EN)))
    {
        /* Component is enabled */
        myADC1_backup.enableState = myADC1_ENABLED;
        if((myADC1_DEC_CR_REG & myADC1_DEC_START_CONV) != 0u)
        {   
            /* Conversion is started */
            myADC1_backup.enableState |= myADC1_STARTED;
        }
		
        /* Stop the configuration */
        myADC1_Stop();
    }
    else
    {
        /* Component is disabled */
        myADC1_backup.enableState = myADC1_DISABLED;
    }

    /* Save the user configuration */
    myADC1_SaveConfig();
}


/*******************************************************************************
* Function Name: myADC1_Wakeup
********************************************************************************
*
* Summary:
*  Restores the user configuration and enables the power to the block.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  myADC1_backup:  The structure field 'enableState' is used to
*  restore the enable state of block after wakeup from sleep mode.
*
*******************************************************************************/
void myADC1_Wakeup(void) 
{
    /* Restore the configuration */
    myADC1_RestoreConfig();

    /* Enables the component operation */
    if(myADC1_backup.enableState != myADC1_DISABLED)
    {
        myADC1_Enable();
        if((myADC1_backup.enableState & myADC1_STARTED) != 0u)
        {
            myADC1_StartConvert();
        }
    } /* Do nothing if component was disable before */
}


/* [] END OF FILE */
