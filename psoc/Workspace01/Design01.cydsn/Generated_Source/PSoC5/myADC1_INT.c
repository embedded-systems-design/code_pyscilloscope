/*******************************************************************************
* File Name: myADC1_INT.c
* Version 3.30
*
* Description:
*  This file contains the code that operates during the ADC_DelSig interrupt
*  service routine.
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
#include "cyapicallbacks.h"


/*******************************************************************************
* Custom Declarations and Variables
* - add user include files, prototypes and variables between the following
*   #START and #END tags
*******************************************************************************/
/* `#START ADC_SYS_VAR`  */

/* `#END`  */

#if(myADC1_IRQ_REMOVE == 0u)


    /*****************************************************************************
    * Function Name: myADC1_ISR1
    ******************************************************************************
    *
    * Summary:
    *  Handle Interrupt Service Routine.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  None
    *
    * Reentrant:
    *  No
    *
    *****************************************************************************/
    CY_ISR( myADC1_ISR1)
    {
        #ifdef myADC1_ISR1_ENTRY_CALLBACK
            myADC1_ISR1_EntryCallback();
        #endif /* myADC1_ISR1_ENTRY_CALLBACK */
        

        /**************************************************************************
        *  Custom Code
        *  - add user ISR code between the following #START and #END tags
        **************************************************************************/
        /* `#START MAIN_ADC_ISR1`  */

        /* `#END`  */
        
        /* Stop the conversion if conversion mode is single sample and resolution
        *  is above 16 bits.
        */
        #if(myADC1_CFG1_RESOLUTION > 16 && \
            myADC1_CFG1_CONV_MODE == myADC1_MODE_SINGLE_SAMPLE)
            myADC1_StopConvert();
            /* Software flag for checking conversion complete or not. Will be used when
            *  resolution is above 16 bits and conversion mode is single sample 
			*/
            myADC1_convDone = myADC1_DEC_CONV_DONE;
        #endif /* Single sample conversion mode with resolution above 16 bits */

        #ifdef myADC1_ISR1_EXIT_CALLBACK
            myADC1_ISR1_ExitCallback();
        #endif /* myADC1_ISR1_EXIT_CALLBACK */
    }


    /*****************************************************************************
    * Function Name: myADC1_ISR2
    ******************************************************************************
    *
    * Summary:
    *  Handle Interrupt Service Routine.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  None
    *
    * Reentrant:
    *  No
    *
    *****************************************************************************/
    CY_ISR( myADC1_ISR2)
    {
        #ifdef myADC1_ISR2_ENTRY_CALLBACK
            myADC1_ISR2_EntryCallback();
        #endif /* myADC1_ISR2_ENTRY_CALLBACK */
        
        
        /***************************************************************************
        *  Custom Code
        *  - add user ISR code between the following #START and #END tags
        **************************************************************************/
        /* `#START MAIN_ADC_ISR2`  */

        /* `#END`  */

        /* Stop the conversion conversion mode is single sample and resolution
        *  is above 16 bits.
        */
        #if(myADC1_CFG2_RESOLUTION > 16 && \
            myADC1_CFG2_CONVMODE == myADC1_MODE_SINGLE_SAMPLE)
            myADC1_StopConvert();
            /* Software flag for checking conversion complete or not. Will be used when
            *   resolution is above 16 bits and conversion mode is single sample 
			*/
            myADC1_convDone = myADC1_DEC_CONV_DONE;
        #endif /* Single sample conversion mode with resolution above 16 bits */

        #ifdef myADC1_ISR2_EXIT_CALLBACK
            myADC1_ISR2_ExitCallback();
        #endif /* myADC1_ISR2_EXIT_CALLBACK */
    }


    /*****************************************************************************
    * Function Name: myADC1_ISR3
    ******************************************************************************
    *
    * Summary:
    *  Handle Interrupt Service Routine.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  None
    *
    * Reentrant:
    *  No
    *
    *****************************************************************************/
    CY_ISR( myADC1_ISR3)
    {
        #ifdef myADC1_ISR3_ENTRY_CALLBACK
            myADC1_ISR3_EntryCallback();
        #endif /* myADC1_ISR3_ENTRY_CALLBACK */        

        
        /***************************************************************************
        *  Custom Code
        *  - add user ISR code between the following #START and #END tags
        **************************************************************************/
        /* `#START MAIN_ADC_ISR3`  */

        /* `#END`  */
        
        /* Stop the conversion if conversion mode is set to single sample and
        *  resolution is above 16 bits.
        */
        #if(myADC1_CFG3_RESOLUTION > 16 && \
            myADC1_CFG3_CONVMODE == myADC1_MODE_SINGLE_SAMPLE)
            myADC1_StopConvert();
            /* Software flag for checking conversion complete or not. Will be used when
            *  resolution is above 16 bits and conversion mode is single sample 
			*/
            myADC1_convDone = myADC1_DEC_CONV_DONE;
        #endif /* Single sample conversion mode with resolution above 16 bits */

        #ifdef myADC1_ISR3_EXIT_CALLBACK
            myADC1_ISR3_ExitCallback();
        #endif /* myADC1_ISR3_EXIT_CALLBACK */        
    }


    /*****************************************************************************
    * Function Name: myADC1_ISR4
    ******************************************************************************
    *
    * Summary:
    *  Handle Interrupt Service Routine.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  None
    *
    * Reentrant:
    *  No
    *
    *****************************************************************************/
    CY_ISR( myADC1_ISR4)
    {
        #ifdef myADC1_ISR4_ENTRY_CALLBACK
            myADC1_ISR4_EntryCallback();
        #endif /* myADC1_ISR4_ENTRY_CALLBACK */            

        
        /***************************************************************************
        *  Custom Code
        *  - add user ISR code between the following #START and #END tags
        **************************************************************************/
        /* `#START MAIN_ADC_ISR4`  */

        /* `#END`  */

        /* Stop the conversion if conversion mode is set to single sample and
        *  resolution is above 16 bits.
        */
        #if(myADC1_CFG4_RESOLUTION > 16 && \
            myADC1_CFG4_CONVMODE == myADC1_MODE_SINGLE_SAMPLE)
            myADC1_StopConvert();
            /* Software flag for checking conversion complete or not. Will be used when
            *  resolution is above 16 bits and conversion mode is single sample 
			*/
            myADC1_convDone = myADC1_DEC_CONV_DONE;
        #endif /* Single sample conversion mode with resolution above 16 bits */
        
        #ifdef myADC1_ISR4_EXIT_CALLBACK
            myADC1_ISR4_ExitCallback();
        #endif /* myADC1_ISR4_EXIT_CALLBACK */            
    }

#endif   /* End myADC1_IRQ_REMOVE */


/* [] END OF FILE */
