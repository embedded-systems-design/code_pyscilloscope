/*******************************************************************************
* File Name: myCounter_PM.c  
* Version 3.0
*
*  Description:
*    This file provides the power management source code to API for the
*    Counter.  
*
*   Note:
*     None
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "myCounter.h"

static myCounter_backupStruct myCounter_backup;


/*******************************************************************************
* Function Name: myCounter_SaveConfig
********************************************************************************
* Summary:
*     Save the current user configuration
*
* Parameters:  
*  void
*
* Return: 
*  void
*
* Global variables:
*  myCounter_backup:  Variables of this global structure are modified to 
*  store the values of non retention configuration registers when Sleep() API is 
*  called.
*
*******************************************************************************/
void myCounter_SaveConfig(void) 
{
    #if (!myCounter_UsingFixedFunction)

        myCounter_backup.CounterUdb = myCounter_ReadCounter();

        #if(!myCounter_ControlRegRemoved)
            myCounter_backup.CounterControlRegister = myCounter_ReadControlRegister();
        #endif /* (!myCounter_ControlRegRemoved) */

    #endif /* (!myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_RestoreConfig
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
* Global variables:
*  myCounter_backup:  Variables of this global structure are used to 
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void myCounter_RestoreConfig(void) 
{      
    #if (!myCounter_UsingFixedFunction)

       myCounter_WriteCounter(myCounter_backup.CounterUdb);

        #if(!myCounter_ControlRegRemoved)
            myCounter_WriteControlRegister(myCounter_backup.CounterControlRegister);
        #endif /* (!myCounter_ControlRegRemoved) */

    #endif /* (!myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_Sleep
********************************************************************************
* Summary:
*     Stop and Save the user configuration
*
* Parameters:  
*  void
*
* Return: 
*  void
*
* Global variables:
*  myCounter_backup.enableState:  Is modified depending on the enable 
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void myCounter_Sleep(void) 
{
    #if(!myCounter_ControlRegRemoved)
        /* Save Counter's enable state */
        if(myCounter_CTRL_ENABLE == (myCounter_CONTROL & myCounter_CTRL_ENABLE))
        {
            /* Counter is enabled */
            myCounter_backup.CounterEnableState = 1u;
        }
        else
        {
            /* Counter is disabled */
            myCounter_backup.CounterEnableState = 0u;
        }
    #else
        myCounter_backup.CounterEnableState = 1u;
        if(myCounter_backup.CounterEnableState != 0u)
        {
            myCounter_backup.CounterEnableState = 0u;
        }
    #endif /* (!myCounter_ControlRegRemoved) */
    
    myCounter_Stop();
    myCounter_SaveConfig();
}


/*******************************************************************************
* Function Name: myCounter_Wakeup
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
*  myCounter_backup.enableState:  Is used to restore the enable state of 
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void myCounter_Wakeup(void) 
{
    myCounter_RestoreConfig();
    #if(!myCounter_ControlRegRemoved)
        if(myCounter_backup.CounterEnableState == 1u)
        {
            /* Enable Counter's operation */
            myCounter_Enable();
        } /* Do nothing if Counter was disabled before */    
    #endif /* (!myCounter_ControlRegRemoved) */
    
}


/* [] END OF FILE */
