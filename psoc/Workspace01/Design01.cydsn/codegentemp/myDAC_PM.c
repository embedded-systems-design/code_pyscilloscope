/*******************************************************************************
* File Name: myDAC_PM.c  
* Version 1.90
*
* Description:
*  This file provides the power management source code to API for the
*  VDAC8.  
*
* Note:
*  None
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "myDAC.h"

static myDAC_backupStruct myDAC_backup;


/*******************************************************************************
* Function Name: myDAC_SaveConfig
********************************************************************************
* Summary:
*  Save the current user configuration
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void myDAC_SaveConfig(void) 
{
    if (!((myDAC_CR1 & myDAC_SRC_MASK) == myDAC_SRC_UDB))
    {
        myDAC_backup.data_value = myDAC_Data;
    }
}


/*******************************************************************************
* Function Name: myDAC_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration.
*
* Parameters:  
*  void
*
* Return: 
*  void
*
*******************************************************************************/
void myDAC_RestoreConfig(void) 
{
    if (!((myDAC_CR1 & myDAC_SRC_MASK) == myDAC_SRC_UDB))
    {
        if((myDAC_Strobe & myDAC_STRB_MASK) == myDAC_STRB_EN)
        {
            myDAC_Strobe &= (uint8)(~myDAC_STRB_MASK);
            myDAC_Data = myDAC_backup.data_value;
            myDAC_Strobe |= myDAC_STRB_EN;
        }
        else
        {
            myDAC_Data = myDAC_backup.data_value;
        }
    }
}


/*******************************************************************************
* Function Name: myDAC_Sleep
********************************************************************************
* Summary:
*  Stop and Save the user configuration
*
* Parameters:  
*  void:  
*
* Return: 
*  void
*
* Global variables:
*  myDAC_backup.enableState:  Is modified depending on the enable 
*  state  of the block before entering sleep mode.
*
*******************************************************************************/
void myDAC_Sleep(void) 
{
    /* Save VDAC8's enable state */    
    if(myDAC_ACT_PWR_EN == (myDAC_PWRMGR & myDAC_ACT_PWR_EN))
    {
        /* VDAC8 is enabled */
        myDAC_backup.enableState = 1u;
    }
    else
    {
        /* VDAC8 is disabled */
        myDAC_backup.enableState = 0u;
    }
    
    myDAC_Stop();
    myDAC_SaveConfig();
}


/*******************************************************************************
* Function Name: myDAC_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration
*  
* Parameters:  
*  void
*
* Return: 
*  void
*
* Global variables:
*  myDAC_backup.enableState:  Is used to restore the enable state of 
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void myDAC_Wakeup(void) 
{
    myDAC_RestoreConfig();
    
    if(myDAC_backup.enableState == 1u)
    {
        /* Enable VDAC8's operation */
        myDAC_Enable();

        /* Restore the data register */
        myDAC_SetValue(myDAC_Data);
    } /* Do nothing if VDAC8 was disabled before */    
}


/* [] END OF FILE */
