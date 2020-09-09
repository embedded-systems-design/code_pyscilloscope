/*******************************************************************************
* File Name: myCounter.c  
* Version 3.0
*
*  Description:
*     The Counter component consists of a 8, 16, 24 or 32-bit counter with
*     a selectable period between 2 and 2^Width - 1.  
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

uint8 myCounter_initVar = 0u;


/*******************************************************************************
* Function Name: myCounter_Init
********************************************************************************
* Summary:
*     Initialize to the schematic state
* 
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void myCounter_Init(void) 
{
        #if (!myCounter_UsingFixedFunction && !myCounter_ControlRegRemoved)
            uint8 ctrl;
        #endif /* (!myCounter_UsingFixedFunction && !myCounter_ControlRegRemoved) */
        
        #if(!myCounter_UsingFixedFunction) 
            /* Interrupt State Backup for Critical Region*/
            uint8 myCounter_interruptState;
        #endif /* (!myCounter_UsingFixedFunction) */
        
        #if (myCounter_UsingFixedFunction)
            /* Clear all bits but the enable bit (if it's already set for Timer operation */
            myCounter_CONTROL &= myCounter_CTRL_ENABLE;
            
            /* Clear the mode bits for continuous run mode */
            #if (CY_PSOC5A)
                myCounter_CONTROL2 &= ((uint8)(~myCounter_CTRL_MODE_MASK));
            #endif /* (CY_PSOC5A) */
            #if (CY_PSOC3 || CY_PSOC5LP)
                myCounter_CONTROL3 &= ((uint8)(~myCounter_CTRL_MODE_MASK));                
            #endif /* (CY_PSOC3 || CY_PSOC5LP) */
            /* Check if One Shot mode is enabled i.e. RunMode !=0*/
            #if (myCounter_RunModeUsed != 0x0u)
                /* Set 3rd bit of Control register to enable one shot mode */
                myCounter_CONTROL |= myCounter_ONESHOT;
            #endif /* (myCounter_RunModeUsed != 0x0u) */
            
            /* Set the IRQ to use the status register interrupts */
            myCounter_CONTROL2 |= myCounter_CTRL2_IRQ_SEL;
            
            /* Clear and Set SYNCTC and SYNCCMP bits of RT1 register */
            myCounter_RT1 &= ((uint8)(~myCounter_RT1_MASK));
            myCounter_RT1 |= myCounter_SYNC;     
                    
            /*Enable DSI Sync all all inputs of the Timer*/
            myCounter_RT1 &= ((uint8)(~myCounter_SYNCDSI_MASK));
            myCounter_RT1 |= myCounter_SYNCDSI_EN;

        #else
            #if(!myCounter_ControlRegRemoved)
            /* Set the default compare mode defined in the parameter */
            ctrl = myCounter_CONTROL & ((uint8)(~myCounter_CTRL_CMPMODE_MASK));
            myCounter_CONTROL = ctrl | myCounter_DEFAULT_COMPARE_MODE;
            
            /* Set the default capture mode defined in the parameter */
            ctrl = myCounter_CONTROL & ((uint8)(~myCounter_CTRL_CAPMODE_MASK));
            
            #if( 0 != myCounter_CAPTURE_MODE_CONF)
                myCounter_CONTROL = ctrl | myCounter_DEFAULT_CAPTURE_MODE;
            #else
                myCounter_CONTROL = ctrl;
            #endif /* 0 != myCounter_CAPTURE_MODE */ 
            
            #endif /* (!myCounter_ControlRegRemoved) */
        #endif /* (myCounter_UsingFixedFunction) */
        
        /* Clear all data in the FIFO's */
        #if (!myCounter_UsingFixedFunction)
            myCounter_ClearFIFO();
        #endif /* (!myCounter_UsingFixedFunction) */
        
        /* Set Initial values from Configuration */
        myCounter_WritePeriod(myCounter_INIT_PERIOD_VALUE);
        #if (!(myCounter_UsingFixedFunction && (CY_PSOC5A)))
            myCounter_WriteCounter(myCounter_INIT_COUNTER_VALUE);
        #endif /* (!(myCounter_UsingFixedFunction && (CY_PSOC5A))) */
        myCounter_SetInterruptMode(myCounter_INIT_INTERRUPTS_MASK);
        
        #if (!myCounter_UsingFixedFunction)
            /* Read the status register to clear the unwanted interrupts */
            (void)myCounter_ReadStatusRegister();
            /* Set the compare value (only available to non-fixed function implementation */
            myCounter_WriteCompare(myCounter_INIT_COMPARE_VALUE);
            /* Use the interrupt output of the status register for IRQ output */
            
            /* CyEnterCriticalRegion and CyExitCriticalRegion are used to mark following region critical*/
            /* Enter Critical Region*/
            myCounter_interruptState = CyEnterCriticalSection();
            
            myCounter_STATUS_AUX_CTRL |= myCounter_STATUS_ACTL_INT_EN_MASK;
            
            /* Exit Critical Region*/
            CyExitCriticalSection(myCounter_interruptState);
            
        #endif /* (!myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_Enable
********************************************************************************
* Summary:
*     Enable the Counter
* 
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Side Effects: 
*   If the Enable mode is set to Hardware only then this function has no effect 
*   on the operation of the counter.
*
*******************************************************************************/
void myCounter_Enable(void) 
{
    /* Globally Enable the Fixed Function Block chosen */
    #if (myCounter_UsingFixedFunction)
        myCounter_GLOBAL_ENABLE |= myCounter_BLOCK_EN_MASK;
        myCounter_GLOBAL_STBY_ENABLE |= myCounter_BLOCK_STBY_EN_MASK;
    #endif /* (myCounter_UsingFixedFunction) */  
        
    /* Enable the counter from the control register  */
    /* If Fixed Function then make sure Mode is set correctly */
    /* else make sure reset is clear */
    #if(!myCounter_ControlRegRemoved || myCounter_UsingFixedFunction)
        myCounter_CONTROL |= myCounter_CTRL_ENABLE;                
    #endif /* (!myCounter_ControlRegRemoved || myCounter_UsingFixedFunction) */
    
}


/*******************************************************************************
* Function Name: myCounter_Start
********************************************************************************
* Summary:
*  Enables the counter for operation 
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Global variables:
*  myCounter_initVar: Is modified when this function is called for the  
*   first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void myCounter_Start(void) 
{
    if(myCounter_initVar == 0u)
    {
        myCounter_Init();
        
        myCounter_initVar = 1u; /* Clear this bit for Initialization */        
    }
    
    /* Enable the Counter */
    myCounter_Enable();        
}


/*******************************************************************************
* Function Name: myCounter_Stop
********************************************************************************
* Summary:
* Halts the counter, but does not change any modes or disable interrupts.
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Side Effects: If the Enable mode is set to Hardware only then this function
*               has no effect on the operation of the counter.
*
*******************************************************************************/
void myCounter_Stop(void) 
{
    /* Disable Counter */
    #if(!myCounter_ControlRegRemoved || myCounter_UsingFixedFunction)
        myCounter_CONTROL &= ((uint8)(~myCounter_CTRL_ENABLE));        
    #endif /* (!myCounter_ControlRegRemoved || myCounter_UsingFixedFunction) */
    
    /* Globally disable the Fixed Function Block chosen */
    #if (myCounter_UsingFixedFunction)
        myCounter_GLOBAL_ENABLE &= ((uint8)(~myCounter_BLOCK_EN_MASK));
        myCounter_GLOBAL_STBY_ENABLE &= ((uint8)(~myCounter_BLOCK_STBY_EN_MASK));
    #endif /* (myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_SetInterruptMode
********************************************************************************
* Summary:
* Configures which interrupt sources are enabled to generate the final interrupt
*
* Parameters:  
*  InterruptsMask: This parameter is an or'd collection of the status bits
*                   which will be allowed to generate the counters interrupt.   
*
* Return: 
*  void
*
*******************************************************************************/
void myCounter_SetInterruptMode(uint8 interruptsMask) 
{
    myCounter_STATUS_MASK = interruptsMask;
}


/*******************************************************************************
* Function Name: myCounter_ReadStatusRegister
********************************************************************************
* Summary:
*   Reads the status register and returns it's state. This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the status register
*
* Side Effects:
*   Status register bits may be clear on read. 
*
*******************************************************************************/
uint8   myCounter_ReadStatusRegister(void) 
{
    return myCounter_STATUS;
}


#if(!myCounter_ControlRegRemoved)
/*******************************************************************************
* Function Name: myCounter_ReadControlRegister
********************************************************************************
* Summary:
*   Reads the control register and returns it's state. This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the control register
*
*******************************************************************************/
uint8   myCounter_ReadControlRegister(void) 
{
    return myCounter_CONTROL;
}


/*******************************************************************************
* Function Name: myCounter_WriteControlRegister
********************************************************************************
* Summary:
*   Sets the bit-field of the control register.  This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the control register
*
*******************************************************************************/
void    myCounter_WriteControlRegister(uint8 control) 
{
    myCounter_CONTROL = control;
}

#endif  /* (!myCounter_ControlRegRemoved) */


#if (!(myCounter_UsingFixedFunction && (CY_PSOC5A)))
/*******************************************************************************
* Function Name: myCounter_WriteCounter
********************************************************************************
* Summary:
*   This funtion is used to set the counter to a specific value
*
* Parameters:  
*  counter:  New counter value. 
*
* Return: 
*  void 
*
*******************************************************************************/
void myCounter_WriteCounter(uint16 counter) \
                                   
{
    #if(myCounter_UsingFixedFunction)
        /* assert if block is already enabled */
        CYASSERT (0u == (myCounter_GLOBAL_ENABLE & myCounter_BLOCK_EN_MASK));
        /* If block is disabled, enable it and then write the counter */
        myCounter_GLOBAL_ENABLE |= myCounter_BLOCK_EN_MASK;
        CY_SET_REG16(myCounter_COUNTER_LSB_PTR, (uint16)counter);
        myCounter_GLOBAL_ENABLE &= ((uint8)(~myCounter_BLOCK_EN_MASK));
    #else
        CY_SET_REG16(myCounter_COUNTER_LSB_PTR, counter);
    #endif /* (myCounter_UsingFixedFunction) */
}
#endif /* (!(myCounter_UsingFixedFunction && (CY_PSOC5A))) */


/*******************************************************************************
* Function Name: myCounter_ReadCounter
********************************************************************************
* Summary:
* Returns the current value of the counter.  It doesn't matter
* if the counter is enabled or running.
*
* Parameters:  
*  void:  
*
* Return: 
*  (uint16) The present value of the counter.
*
*******************************************************************************/
uint16 myCounter_ReadCounter(void) 
{
    /* Force capture by reading Accumulator */
    /* Must first do a software capture to be able to read the counter */
    /* It is up to the user code to make sure there isn't already captured data in the FIFO */
    #if(myCounter_UsingFixedFunction)
		(void)CY_GET_REG16(myCounter_COUNTER_LSB_PTR);
	#else
		(void)CY_GET_REG8(myCounter_COUNTER_LSB_PTR_8BIT);
	#endif/* (myCounter_UsingFixedFunction) */
    
    /* Read the data from the FIFO (or capture register for Fixed Function)*/
    #if(myCounter_UsingFixedFunction)
        return ((uint16)CY_GET_REG16(myCounter_STATICCOUNT_LSB_PTR));
    #else
        return (CY_GET_REG16(myCounter_STATICCOUNT_LSB_PTR));
    #endif /* (myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_ReadCapture
********************************************************************************
* Summary:
*   This function returns the last value captured.
*
* Parameters:  
*  void
*
* Return: 
*  (uint16) Present Capture value.
*
*******************************************************************************/
uint16 myCounter_ReadCapture(void) 
{
    #if(myCounter_UsingFixedFunction)
        return ((uint16)CY_GET_REG16(myCounter_STATICCOUNT_LSB_PTR));
    #else
        return (CY_GET_REG16(myCounter_STATICCOUNT_LSB_PTR));
    #endif /* (myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_WritePeriod
********************************************************************************
* Summary:
* Changes the period of the counter.  The new period 
* will be loaded the next time terminal count is detected.
*
* Parameters:  
*  period: (uint16) A value of 0 will result in
*         the counter remaining at zero.  
*
* Return: 
*  void
*
*******************************************************************************/
void myCounter_WritePeriod(uint16 period) 
{
    #if(myCounter_UsingFixedFunction)
        CY_SET_REG16(myCounter_PERIOD_LSB_PTR,(uint16)period);
    #else
        CY_SET_REG16(myCounter_PERIOD_LSB_PTR, period);
    #endif /* (myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_ReadPeriod
********************************************************************************
* Summary:
* Reads the current period value without affecting counter operation.
*
* Parameters:  
*  void:  
*
* Return: 
*  (uint16) Present period value.
*
*******************************************************************************/
uint16 myCounter_ReadPeriod(void) 
{
    #if(myCounter_UsingFixedFunction)
        return ((uint16)CY_GET_REG16(myCounter_PERIOD_LSB_PTR));
    #else
        return (CY_GET_REG16(myCounter_PERIOD_LSB_PTR));
    #endif /* (myCounter_UsingFixedFunction) */
}


#if (!myCounter_UsingFixedFunction)
/*******************************************************************************
* Function Name: myCounter_WriteCompare
********************************************************************************
* Summary:
* Changes the compare value.  The compare output will 
* reflect the new value on the next UDB clock.  The compare output will be 
* driven high when the present counter value compares true based on the 
* configured compare mode setting. 
*
* Parameters:  
*  Compare:  New compare value. 
*
* Return: 
*  void
*
*******************************************************************************/
void myCounter_WriteCompare(uint16 compare) \
                                   
{
    #if(myCounter_UsingFixedFunction)
        CY_SET_REG16(myCounter_COMPARE_LSB_PTR, (uint16)compare);
    #else
        CY_SET_REG16(myCounter_COMPARE_LSB_PTR, compare);
    #endif /* (myCounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: myCounter_ReadCompare
********************************************************************************
* Summary:
* Returns the compare value.
*
* Parameters:  
*  void:
*
* Return: 
*  (uint16) Present compare value.
*
*******************************************************************************/
uint16 myCounter_ReadCompare(void) 
{
    return (CY_GET_REG16(myCounter_COMPARE_LSB_PTR));
}


#if (myCounter_COMPARE_MODE_SOFTWARE)
/*******************************************************************************
* Function Name: myCounter_SetCompareMode
********************************************************************************
* Summary:
*  Sets the software controlled Compare Mode.
*
* Parameters:
*  compareMode:  Compare Mode Enumerated Type.
*
* Return:
*  void
*
*******************************************************************************/
void myCounter_SetCompareMode(uint8 compareMode) 
{
    /* Clear the compare mode bits in the control register */
    myCounter_CONTROL &= ((uint8)(~myCounter_CTRL_CMPMODE_MASK));
    
    /* Write the new setting */
    myCounter_CONTROL |= compareMode;
}
#endif  /* (myCounter_COMPARE_MODE_SOFTWARE) */


#if (myCounter_CAPTURE_MODE_SOFTWARE)
/*******************************************************************************
* Function Name: myCounter_SetCaptureMode
********************************************************************************
* Summary:
*  Sets the software controlled Capture Mode.
*
* Parameters:
*  captureMode:  Capture Mode Enumerated Type.
*
* Return:
*  void
*
*******************************************************************************/
void myCounter_SetCaptureMode(uint8 captureMode) 
{
    /* Clear the capture mode bits in the control register */
    myCounter_CONTROL &= ((uint8)(~myCounter_CTRL_CAPMODE_MASK));
    
    /* Write the new setting */
    myCounter_CONTROL |= ((uint8)((uint8)captureMode << myCounter_CTRL_CAPMODE0_SHIFT));
}
#endif  /* (myCounter_CAPTURE_MODE_SOFTWARE) */


/*******************************************************************************
* Function Name: myCounter_ClearFIFO
********************************************************************************
* Summary:
*   This function clears all capture data from the capture FIFO
*
* Parameters:  
*  void:
*
* Return: 
*  None
*
*******************************************************************************/
void myCounter_ClearFIFO(void) 
{

    while(0u != (myCounter_ReadStatusRegister() & myCounter_STATUS_FIFONEMP))
    {
        (void)myCounter_ReadCapture();
    }

}
#endif  /* (!myCounter_UsingFixedFunction) */


/* [] END OF FILE */

