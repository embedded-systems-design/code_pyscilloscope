/*******************************************************************************
* File Name: myADC_SEQ_PM.c
* Version 2.10
*
* Description:
*  This file contains the setup, control and status commands to support
*  component operations in low power mode.
*
* Note:
*
********************************************************************************
* Copyright 2012-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myADC_SEQ.h"
#include "myADC_SEQ_SAR.h"
#if(myADC_SEQ_CLOCK_SOURCE == myADC_SEQ_CLOCK_INTERNAL)
    #include "myADC_SEQ_IntClock.h"
#endif   /* myADC_SEQ_CLOCK_SOURCE == myADC_SEQ_CLOCK_INTERNAL */


/*******************************************************************************
* Function Name: myADC_SEQ_Sleep
********************************************************************************
*
* Summary:
*  Stops the ADC operation and saves the configuration registers and component
*  enable state. Should be called just prior to entering sleep
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void myADC_SEQ_Sleep(void)
{
    myADC_SEQ_SAR_Stop();
    myADC_SEQ_SAR_Sleep();
    myADC_SEQ_Disable();

    #if(myADC_SEQ_CLOCK_SOURCE == myADC_SEQ_CLOCK_INTERNAL)
        myADC_SEQ_IntClock_Stop();
    #endif   /* myADC_SEQ_CLOCK_SOURCE == myADC_SEQ_CLOCK_INTERNAL */
}


/*******************************************************************************
* Function Name: myADC_SEQ_Wakeup
********************************************************************************
*
* Summary:
*  Restores the component enable state and configuration registers. This should
*  be called just after awaking from sleep mode
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void myADC_SEQ_Wakeup(void)
{
    myADC_SEQ_SAR_Wakeup();
    myADC_SEQ_SAR_Enable();

    #if(myADC_SEQ_CLOCK_SOURCE == myADC_SEQ_CLOCK_INTERNAL)
        myADC_SEQ_IntClock_Start();
    #endif   /* myADC_SEQ_CLOCK_SOURCE == myADC_SEQ_CLOCK_INTERNAL */

    /* The block is ready to use 10 us after the SAR enable signal is set high. */
    CyDelayUs(10u);
    
    myADC_SEQ_Enable();

    #if(myADC_SEQ_SAMPLE_MODE == myADC_SEQ_SAMPLE_MODE_FREE_RUNNING)
        myADC_SEQ_SAR_StartConvert();
    #endif /* (myADC_SEQ_SAMPLE_MODE == myADC_SEQ_SAMPLE_MODE_FREE_RUNNING) */

    (void) CY_GET_REG8(myADC_SEQ_STATUS_PTR);
}


/*******************************************************************************
* Function Name: myADC_SEQ_SaveConfig
********************************************************************************
*
* Summary:
*  Save the current configuration of ADC non-retention registers
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void myADC_SEQ_SaveConfig(void)
{

}


/*******************************************************************************
* Function Name: myADC_SEQ_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the configuration of ADC non-retention registers
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void myADC_SEQ_RestoreConfig(void)
{

}


/* [] END OF FILE */
