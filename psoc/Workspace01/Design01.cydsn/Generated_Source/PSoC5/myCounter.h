/*******************************************************************************
* File Name: myCounter.h  
* Version 3.0
*
*  Description:
*   Contains the function prototypes and constants available to the counter
*   user module.
*
*   Note:
*    None
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/
    
#if !defined(CY_COUNTER_myCounter_H)
#define CY_COUNTER_myCounter_H

#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */

extern uint8 myCounter_initVar;

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component Counter_v3_0 requires cy_boot v3.0 or later
#endif /* (CY_ PSOC5LP) */

/* Error message for removed myCounter_CounterUDB_sCTRLReg_ctrlreg through optimization */
#ifdef myCounter_CounterUDB_sCTRLReg_ctrlreg__REMOVED
    #error Counter_v3_0 detected with a constant 0 for the enable, a \
                                constant 0 for the count or constant 1 for \
                                the reset. This will prevent the component from\
                                operating.
#endif /* myCounter_CounterUDB_sCTRLReg_ctrlreg__REMOVED */


/**************************************
*           Parameter Defaults        
**************************************/

#define myCounter_Resolution            16u
#define myCounter_UsingFixedFunction    1u
#define myCounter_ControlRegRemoved     0u
#define myCounter_COMPARE_MODE_SOFTWARE 0u
#define myCounter_CAPTURE_MODE_SOFTWARE 0u
#define myCounter_RunModeUsed           0u


/***************************************
*       Type defines
***************************************/


/**************************************************************************
 * Sleep Mode API Support
 * Backup structure for Sleep Wake up operations
 *************************************************************************/
typedef struct
{
    uint8 CounterEnableState; 
    uint16 CounterUdb;         /* Current Counter Value */

    #if (!myCounter_ControlRegRemoved)
        uint8 CounterControlRegister;               /* Counter Control Register */
    #endif /* (!myCounter_ControlRegRemoved) */

}myCounter_backupStruct;


/**************************************
 *  Function Prototypes
 *************************************/
void    myCounter_Start(void) ;
void    myCounter_Stop(void) ;
void    myCounter_SetInterruptMode(uint8 interruptsMask) ;
uint8   myCounter_ReadStatusRegister(void) ;
#define myCounter_GetInterruptSource() myCounter_ReadStatusRegister()
#if(!myCounter_ControlRegRemoved)
    uint8   myCounter_ReadControlRegister(void) ;
    void    myCounter_WriteControlRegister(uint8 control) \
        ;
#endif /* (!myCounter_ControlRegRemoved) */
#if (!(myCounter_UsingFixedFunction && (CY_PSOC5A)))
    void    myCounter_WriteCounter(uint16 counter) \
            ; 
#endif /* (!(myCounter_UsingFixedFunction && (CY_PSOC5A))) */
uint16  myCounter_ReadCounter(void) ;
uint16  myCounter_ReadCapture(void) ;
void    myCounter_WritePeriod(uint16 period) \
    ;
uint16  myCounter_ReadPeriod( void ) ;
#if (!myCounter_UsingFixedFunction)
    void    myCounter_WriteCompare(uint16 compare) \
        ;
    uint16  myCounter_ReadCompare( void ) \
        ;
#endif /* (!myCounter_UsingFixedFunction) */

#if (myCounter_COMPARE_MODE_SOFTWARE)
    void    myCounter_SetCompareMode(uint8 compareMode) ;
#endif /* (myCounter_COMPARE_MODE_SOFTWARE) */
#if (myCounter_CAPTURE_MODE_SOFTWARE)
    void    myCounter_SetCaptureMode(uint8 captureMode) ;
#endif /* (myCounter_CAPTURE_MODE_SOFTWARE) */
void myCounter_ClearFIFO(void)     ;
void myCounter_Init(void)          ;
void myCounter_Enable(void)        ;
void myCounter_SaveConfig(void)    ;
void myCounter_RestoreConfig(void) ;
void myCounter_Sleep(void)         ;
void myCounter_Wakeup(void)        ;


/***************************************
*   Enumerated Types and Parameters
***************************************/

/* Enumerated Type B_Counter__CompareModes, Used in Compare Mode retained for backward compatibility of tests*/
#define myCounter__B_COUNTER__LESS_THAN 1
#define myCounter__B_COUNTER__LESS_THAN_OR_EQUAL 2
#define myCounter__B_COUNTER__EQUAL 0
#define myCounter__B_COUNTER__GREATER_THAN 3
#define myCounter__B_COUNTER__GREATER_THAN_OR_EQUAL 4
#define myCounter__B_COUNTER__SOFTWARE 5

/* Enumerated Type Counter_CompareModes */
#define myCounter_CMP_MODE_LT 1u
#define myCounter_CMP_MODE_LTE 2u
#define myCounter_CMP_MODE_EQ 0u
#define myCounter_CMP_MODE_GT 3u
#define myCounter_CMP_MODE_GTE 4u
#define myCounter_CMP_MODE_SOFTWARE_CONTROLLED 5u

/* Enumerated Type B_Counter__CaptureModes, Used in Capture Mode retained for backward compatibility of tests*/
#define myCounter__B_COUNTER__NONE 0
#define myCounter__B_COUNTER__RISING_EDGE 1
#define myCounter__B_COUNTER__FALLING_EDGE 2
#define myCounter__B_COUNTER__EITHER_EDGE 3
#define myCounter__B_COUNTER__SOFTWARE_CONTROL 4

/* Enumerated Type Counter_CompareModes */
#define myCounter_CAP_MODE_NONE 0u
#define myCounter_CAP_MODE_RISE 1u
#define myCounter_CAP_MODE_FALL 2u
#define myCounter_CAP_MODE_BOTH 3u
#define myCounter_CAP_MODE_SOFTWARE_CONTROLLED 4u


/***************************************
 *  Initialization Values
 **************************************/
#define myCounter_CAPTURE_MODE_CONF       0u
#define myCounter_INIT_PERIOD_VALUE       60000u
#define myCounter_INIT_COUNTER_VALUE      60000u
#if (myCounter_UsingFixedFunction)
#define myCounter_INIT_INTERRUPTS_MASK    ((uint8)((uint8)0u << myCounter_STATUS_ZERO_INT_EN_MASK_SHIFT))
#else
#define myCounter_INIT_COMPARE_VALUE      60u
#define myCounter_INIT_INTERRUPTS_MASK ((uint8)((uint8)0u << myCounter_STATUS_ZERO_INT_EN_MASK_SHIFT) | \
        ((uint8)((uint8)0u << myCounter_STATUS_CAPTURE_INT_EN_MASK_SHIFT)) | \
        ((uint8)((uint8)0u << myCounter_STATUS_CMP_INT_EN_MASK_SHIFT)) | \
        ((uint8)((uint8)0u << myCounter_STATUS_OVERFLOW_INT_EN_MASK_SHIFT)) | \
        ((uint8)((uint8)0u << myCounter_STATUS_UNDERFLOW_INT_EN_MASK_SHIFT)))
#define myCounter_DEFAULT_COMPARE_MODE    1u

#if( 0 != myCounter_CAPTURE_MODE_CONF)
    #define myCounter_DEFAULT_CAPTURE_MODE    ((uint8)((uint8)0u << myCounter_CTRL_CAPMODE0_SHIFT))
#else    
    #define myCounter_DEFAULT_CAPTURE_MODE    (0u )
#endif /* ( 0 != myCounter_CAPTURE_MODE_CONF) */

#endif /* (myCounter_UsingFixedFunction) */


/**************************************
 *  Registers
 *************************************/
#if (myCounter_UsingFixedFunction)
    #define myCounter_STATICCOUNT_LSB     (*(reg16 *) myCounter_CounterHW__CAP0 )
    #define myCounter_STATICCOUNT_LSB_PTR ( (reg16 *) myCounter_CounterHW__CAP0 )
    #define myCounter_PERIOD_LSB          (*(reg16 *) myCounter_CounterHW__PER0 )
    #define myCounter_PERIOD_LSB_PTR      ( (reg16 *) myCounter_CounterHW__PER0 )
    /* MODE must be set to 1 to set the compare value */
    #define myCounter_COMPARE_LSB         (*(reg16 *) myCounter_CounterHW__CNT_CMP0 )
    #define myCounter_COMPARE_LSB_PTR     ( (reg16 *) myCounter_CounterHW__CNT_CMP0 )
    /* MODE must be set to 0 to get the count */
    #define myCounter_COUNTER_LSB         (*(reg16 *) myCounter_CounterHW__CNT_CMP0 )
    #define myCounter_COUNTER_LSB_PTR     ( (reg16 *) myCounter_CounterHW__CNT_CMP0 )
    #define myCounter_RT1                 (*(reg8 *) myCounter_CounterHW__RT1)
    #define myCounter_RT1_PTR             ( (reg8 *) myCounter_CounterHW__RT1)
#else
    
    #if (myCounter_Resolution <= 8u) /* 8-bit Counter */
    
        #define myCounter_STATICCOUNT_LSB     (*(reg8 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
        #define myCounter_STATICCOUNT_LSB_PTR ( (reg8 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
        #define myCounter_PERIOD_LSB          (*(reg8 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
        #define myCounter_PERIOD_LSB_PTR      ( (reg8 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
        #define myCounter_COMPARE_LSB         (*(reg8 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
        #define myCounter_COMPARE_LSB_PTR     ( (reg8 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
        #define myCounter_COUNTER_LSB         (*(reg8 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )  
        #define myCounter_COUNTER_LSB_PTR     ( (reg8 *)\
            myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )
    
    #elif(myCounter_Resolution <= 16u) /* 16-bit Counter */
        #if(CY_PSOC3) /* 8-bit address space */ 
            #define myCounter_STATICCOUNT_LSB     (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
            #define myCounter_STATICCOUNT_LSB_PTR ( (reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
            #define myCounter_PERIOD_LSB          (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
            #define myCounter_PERIOD_LSB_PTR      ( (reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
            #define myCounter_COMPARE_LSB         (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
            #define myCounter_COMPARE_LSB_PTR     ( (reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
            #define myCounter_COUNTER_LSB         (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )  
            #define myCounter_COUNTER_LSB_PTR     ( (reg16 *)\
                myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )
        #else /* 16-bit address space */
            #define myCounter_STATICCOUNT_LSB     (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_F0_REG )
            #define myCounter_STATICCOUNT_LSB_PTR ( (reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_F0_REG )
            #define myCounter_PERIOD_LSB          (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_D0_REG )
            #define myCounter_PERIOD_LSB_PTR      ( (reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_D0_REG )
            #define myCounter_COMPARE_LSB         (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_D1_REG )
            #define myCounter_COMPARE_LSB_PTR     ( (reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_D1_REG )
            #define myCounter_COUNTER_LSB         (*(reg16 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_A0_REG )  
            #define myCounter_COUNTER_LSB_PTR     ( (reg16 *)\
                myCounter_CounterUDB_sC16_counterdp_u0__16BIT_A0_REG )
        #endif /* CY_PSOC3 */   
    #elif(myCounter_Resolution <= 24u) /* 24-bit Counter */
        
        #define myCounter_STATICCOUNT_LSB     (*(reg32 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
        #define myCounter_STATICCOUNT_LSB_PTR ( (reg32 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
        #define myCounter_PERIOD_LSB          (*(reg32 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
        #define myCounter_PERIOD_LSB_PTR      ( (reg32 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
        #define myCounter_COMPARE_LSB         (*(reg32 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
        #define myCounter_COMPARE_LSB_PTR     ( (reg32 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
        #define myCounter_COUNTER_LSB         (*(reg32 *) \
            myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )  
        #define myCounter_COUNTER_LSB_PTR     ( (reg32 *)\
            myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )
    
    #else /* 32-bit Counter */
        #if(CY_PSOC3 || CY_PSOC5) /* 8-bit address space */
            #define myCounter_STATICCOUNT_LSB     (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
            #define myCounter_STATICCOUNT_LSB_PTR ( (reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__F0_REG )
            #define myCounter_PERIOD_LSB          (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
            #define myCounter_PERIOD_LSB_PTR      ( (reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D0_REG )
            #define myCounter_COMPARE_LSB         (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
            #define myCounter_COMPARE_LSB_PTR     ( (reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__D1_REG )
            #define myCounter_COUNTER_LSB         (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )  
            #define myCounter_COUNTER_LSB_PTR     ( (reg32 *)\
                myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )
        #else /* 32-bit address space */
            #define myCounter_STATICCOUNT_LSB     (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_F0_REG )
            #define myCounter_STATICCOUNT_LSB_PTR ( (reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_F0_REG )
            #define myCounter_PERIOD_LSB          (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_D0_REG )
            #define myCounter_PERIOD_LSB_PTR      ( (reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_D0_REG )
            #define myCounter_COMPARE_LSB         (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_D1_REG )
            #define myCounter_COMPARE_LSB_PTR     ( (reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_D1_REG )
            #define myCounter_COUNTER_LSB         (*(reg32 *) \
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_A0_REG )  
            #define myCounter_COUNTER_LSB_PTR     ( (reg32 *)\
                myCounter_CounterUDB_sC16_counterdp_u0__32BIT_A0_REG )
        #endif /* CY_PSOC3 || CY_PSOC5 */   
    #endif

	#define myCounter_COUNTER_LSB_PTR_8BIT     ( (reg8 *)\
                myCounter_CounterUDB_sC16_counterdp_u0__A0_REG )
				
    #define myCounter_AUX_CONTROLDP0 \
        (*(reg8 *) myCounter_CounterUDB_sC16_counterdp_u0__DP_AUX_CTL_REG)
    
    #define myCounter_AUX_CONTROLDP0_PTR \
        ( (reg8 *) myCounter_CounterUDB_sC16_counterdp_u0__DP_AUX_CTL_REG)
    
    #if (myCounter_Resolution == 16 || myCounter_Resolution == 24 || myCounter_Resolution == 32)
       #define myCounter_AUX_CONTROLDP1 \
           (*(reg8 *) myCounter_CounterUDB_sC16_counterdp_u1__DP_AUX_CTL_REG)
       #define myCounter_AUX_CONTROLDP1_PTR \
           ( (reg8 *) myCounter_CounterUDB_sC16_counterdp_u1__DP_AUX_CTL_REG)
    #endif /* (myCounter_Resolution == 16 || myCounter_Resolution == 24 || myCounter_Resolution == 32) */
    
    #if (myCounter_Resolution == 24 || myCounter_Resolution == 32)
       #define myCounter_AUX_CONTROLDP2 \
           (*(reg8 *) myCounter_CounterUDB_sC16_counterdp_u2__DP_AUX_CTL_REG)
       #define myCounter_AUX_CONTROLDP2_PTR \
           ( (reg8 *) myCounter_CounterUDB_sC16_counterdp_u2__DP_AUX_CTL_REG)
    #endif /* (myCounter_Resolution == 24 || myCounter_Resolution == 32) */
    
    #if (myCounter_Resolution == 32)
       #define myCounter_AUX_CONTROLDP3 \
           (*(reg8 *) myCounter_CounterUDB_sC16_counterdp_u3__DP_AUX_CTL_REG)
       #define myCounter_AUX_CONTROLDP3_PTR \
           ( (reg8 *) myCounter_CounterUDB_sC16_counterdp_u3__DP_AUX_CTL_REG)
    #endif /* (myCounter_Resolution == 32) */

#endif  /* (myCounter_UsingFixedFunction) */

#if (myCounter_UsingFixedFunction)
    #define myCounter_STATUS         (*(reg8 *) myCounter_CounterHW__SR0 )
    /* In Fixed Function Block Status and Mask are the same register */
    #define myCounter_STATUS_MASK             (*(reg8 *) myCounter_CounterHW__SR0 )
    #define myCounter_STATUS_MASK_PTR         ( (reg8 *) myCounter_CounterHW__SR0 )
    #define myCounter_CONTROL                 (*(reg8 *) myCounter_CounterHW__CFG0)
    #define myCounter_CONTROL_PTR             ( (reg8 *) myCounter_CounterHW__CFG0)
    #define myCounter_CONTROL2                (*(reg8 *) myCounter_CounterHW__CFG1)
    #define myCounter_CONTROL2_PTR            ( (reg8 *) myCounter_CounterHW__CFG1)
    #if (CY_PSOC3 || CY_PSOC5LP)
        #define myCounter_CONTROL3       (*(reg8 *) myCounter_CounterHW__CFG2)
        #define myCounter_CONTROL3_PTR   ( (reg8 *) myCounter_CounterHW__CFG2)
    #endif /* (CY_PSOC3 || CY_PSOC5LP) */
    #define myCounter_GLOBAL_ENABLE           (*(reg8 *) myCounter_CounterHW__PM_ACT_CFG)
    #define myCounter_GLOBAL_ENABLE_PTR       ( (reg8 *) myCounter_CounterHW__PM_ACT_CFG)
    #define myCounter_GLOBAL_STBY_ENABLE      (*(reg8 *) myCounter_CounterHW__PM_STBY_CFG)
    #define myCounter_GLOBAL_STBY_ENABLE_PTR  ( (reg8 *) myCounter_CounterHW__PM_STBY_CFG)
    

    /********************************
    *    Constants
    ********************************/

    /* Fixed Function Block Chosen */
    #define myCounter_BLOCK_EN_MASK          myCounter_CounterHW__PM_ACT_MSK
    #define myCounter_BLOCK_STBY_EN_MASK     myCounter_CounterHW__PM_STBY_MSK 
    
    /* Control Register Bit Locations */    
    /* As defined in Register Map, part of TMRX_CFG0 register */
    #define myCounter_CTRL_ENABLE_SHIFT      0x00u
    #define myCounter_ONESHOT_SHIFT          0x02u
    /* Control Register Bit Masks */
    #define myCounter_CTRL_ENABLE            ((uint8)((uint8)0x01u << myCounter_CTRL_ENABLE_SHIFT))         
    #define myCounter_ONESHOT                ((uint8)((uint8)0x01u << myCounter_ONESHOT_SHIFT))

    /* Control2 Register Bit Masks */
    /* Set the mask for run mode */
    #if (CY_PSOC5A)
        /* Use CFG1 Mode bits to set run mode */
        #define myCounter_CTRL_MODE_SHIFT        0x01u    
        #define myCounter_CTRL_MODE_MASK         ((uint8)((uint8)0x07u << myCounter_CTRL_MODE_SHIFT))
    #endif /* (CY_PSOC5A) */
    #if (CY_PSOC3 || CY_PSOC5LP)
        /* Use CFG2 Mode bits to set run mode */
        #define myCounter_CTRL_MODE_SHIFT        0x00u    
        #define myCounter_CTRL_MODE_MASK         ((uint8)((uint8)0x03u << myCounter_CTRL_MODE_SHIFT))
    #endif /* (CY_PSOC3 || CY_PSOC5LP) */
    /* Set the mask for interrupt (raw/status register) */
    #define myCounter_CTRL2_IRQ_SEL_SHIFT     0x00u
    #define myCounter_CTRL2_IRQ_SEL          ((uint8)((uint8)0x01u << myCounter_CTRL2_IRQ_SEL_SHIFT))     
    
    /* Status Register Bit Locations */
    #define myCounter_STATUS_ZERO_SHIFT      0x07u  /* As defined in Register Map, part of TMRX_SR0 register */ 

    /* Status Register Interrupt Enable Bit Locations */
    #define myCounter_STATUS_ZERO_INT_EN_MASK_SHIFT      (myCounter_STATUS_ZERO_SHIFT - 0x04u)

    /* Status Register Bit Masks */                           
    #define myCounter_STATUS_ZERO            ((uint8)((uint8)0x01u << myCounter_STATUS_ZERO_SHIFT))

    /* Status Register Interrupt Bit Masks*/
    #define myCounter_STATUS_ZERO_INT_EN_MASK       ((uint8)((uint8)myCounter_STATUS_ZERO >> 0x04u))
    
    /*RT1 Synch Constants: Applicable for PSoC3 and PSoC5LP */
    #define myCounter_RT1_SHIFT            0x04u
    #define myCounter_RT1_MASK             ((uint8)((uint8)0x03u << myCounter_RT1_SHIFT))  /* Sync TC and CMP bit masks */
    #define myCounter_SYNC                 ((uint8)((uint8)0x03u << myCounter_RT1_SHIFT))
    #define myCounter_SYNCDSI_SHIFT        0x00u
    #define myCounter_SYNCDSI_MASK         ((uint8)((uint8)0x0Fu << myCounter_SYNCDSI_SHIFT)) /* Sync all DSI inputs */
    #define myCounter_SYNCDSI_EN           ((uint8)((uint8)0x0Fu << myCounter_SYNCDSI_SHIFT)) /* Sync all DSI inputs */
    
#else /* !myCounter_UsingFixedFunction */
    #define myCounter_STATUS               (* (reg8 *) myCounter_CounterUDB_sSTSReg_stsreg__STATUS_REG )
    #define myCounter_STATUS_PTR           (  (reg8 *) myCounter_CounterUDB_sSTSReg_stsreg__STATUS_REG )
    #define myCounter_STATUS_MASK          (* (reg8 *) myCounter_CounterUDB_sSTSReg_stsreg__MASK_REG )
    #define myCounter_STATUS_MASK_PTR      (  (reg8 *) myCounter_CounterUDB_sSTSReg_stsreg__MASK_REG )
    #define myCounter_STATUS_AUX_CTRL      (*(reg8 *) myCounter_CounterUDB_sSTSReg_stsreg__STATUS_AUX_CTL_REG)
    #define myCounter_STATUS_AUX_CTRL_PTR  ( (reg8 *) myCounter_CounterUDB_sSTSReg_stsreg__STATUS_AUX_CTL_REG)
    #define myCounter_CONTROL              (* (reg8 *) myCounter_CounterUDB_sCTRLReg_ctrlreg__CONTROL_REG )
    #define myCounter_CONTROL_PTR          (  (reg8 *) myCounter_CounterUDB_sCTRLReg_ctrlreg__CONTROL_REG )


    /********************************
    *    Constants
    ********************************/
    /* Control Register Bit Locations */
    #define myCounter_CTRL_CAPMODE0_SHIFT    0x03u       /* As defined by Verilog Implementation */
    #define myCounter_CTRL_RESET_SHIFT       0x06u       /* As defined by Verilog Implementation */
    #define myCounter_CTRL_ENABLE_SHIFT      0x07u       /* As defined by Verilog Implementation */
    /* Control Register Bit Masks */
    #define myCounter_CTRL_CMPMODE_MASK      0x07u 
    #define myCounter_CTRL_CAPMODE_MASK      0x03u  
    #define myCounter_CTRL_RESET             ((uint8)((uint8)0x01u << myCounter_CTRL_RESET_SHIFT))  
    #define myCounter_CTRL_ENABLE            ((uint8)((uint8)0x01u << myCounter_CTRL_ENABLE_SHIFT)) 

    /* Status Register Bit Locations */
    #define myCounter_STATUS_CMP_SHIFT       0x00u       /* As defined by Verilog Implementation */
    #define myCounter_STATUS_ZERO_SHIFT      0x01u       /* As defined by Verilog Implementation */
    #define myCounter_STATUS_OVERFLOW_SHIFT  0x02u       /* As defined by Verilog Implementation */
    #define myCounter_STATUS_UNDERFLOW_SHIFT 0x03u       /* As defined by Verilog Implementation */
    #define myCounter_STATUS_CAPTURE_SHIFT   0x04u       /* As defined by Verilog Implementation */
    #define myCounter_STATUS_FIFOFULL_SHIFT  0x05u       /* As defined by Verilog Implementation */
    #define myCounter_STATUS_FIFONEMP_SHIFT  0x06u       /* As defined by Verilog Implementation */
    /* Status Register Interrupt Enable Bit Locations - UDB Status Interrupt Mask match Status Bit Locations*/
    #define myCounter_STATUS_CMP_INT_EN_MASK_SHIFT       myCounter_STATUS_CMP_SHIFT       
    #define myCounter_STATUS_ZERO_INT_EN_MASK_SHIFT      myCounter_STATUS_ZERO_SHIFT      
    #define myCounter_STATUS_OVERFLOW_INT_EN_MASK_SHIFT  myCounter_STATUS_OVERFLOW_SHIFT  
    #define myCounter_STATUS_UNDERFLOW_INT_EN_MASK_SHIFT myCounter_STATUS_UNDERFLOW_SHIFT 
    #define myCounter_STATUS_CAPTURE_INT_EN_MASK_SHIFT   myCounter_STATUS_CAPTURE_SHIFT   
    #define myCounter_STATUS_FIFOFULL_INT_EN_MASK_SHIFT  myCounter_STATUS_FIFOFULL_SHIFT  
    #define myCounter_STATUS_FIFONEMP_INT_EN_MASK_SHIFT  myCounter_STATUS_FIFONEMP_SHIFT  
    /* Status Register Bit Masks */                
    #define myCounter_STATUS_CMP             ((uint8)((uint8)0x01u << myCounter_STATUS_CMP_SHIFT))  
    #define myCounter_STATUS_ZERO            ((uint8)((uint8)0x01u << myCounter_STATUS_ZERO_SHIFT)) 
    #define myCounter_STATUS_OVERFLOW        ((uint8)((uint8)0x01u << myCounter_STATUS_OVERFLOW_SHIFT)) 
    #define myCounter_STATUS_UNDERFLOW       ((uint8)((uint8)0x01u << myCounter_STATUS_UNDERFLOW_SHIFT)) 
    #define myCounter_STATUS_CAPTURE         ((uint8)((uint8)0x01u << myCounter_STATUS_CAPTURE_SHIFT)) 
    #define myCounter_STATUS_FIFOFULL        ((uint8)((uint8)0x01u << myCounter_STATUS_FIFOFULL_SHIFT))
    #define myCounter_STATUS_FIFONEMP        ((uint8)((uint8)0x01u << myCounter_STATUS_FIFONEMP_SHIFT))
    /* Status Register Interrupt Bit Masks  - UDB Status Interrupt Mask match Status Bit Locations */
    #define myCounter_STATUS_CMP_INT_EN_MASK            myCounter_STATUS_CMP                    
    #define myCounter_STATUS_ZERO_INT_EN_MASK           myCounter_STATUS_ZERO            
    #define myCounter_STATUS_OVERFLOW_INT_EN_MASK       myCounter_STATUS_OVERFLOW        
    #define myCounter_STATUS_UNDERFLOW_INT_EN_MASK      myCounter_STATUS_UNDERFLOW       
    #define myCounter_STATUS_CAPTURE_INT_EN_MASK        myCounter_STATUS_CAPTURE         
    #define myCounter_STATUS_FIFOFULL_INT_EN_MASK       myCounter_STATUS_FIFOFULL        
    #define myCounter_STATUS_FIFONEMP_INT_EN_MASK       myCounter_STATUS_FIFONEMP         
    

    /* StatusI Interrupt Enable bit Location in the Auxilliary Control Register */
    #define myCounter_STATUS_ACTL_INT_EN     0x10u /* As defined for the ACTL Register */
    
    /* Datapath Auxillary Control Register definitions */
    #define myCounter_AUX_CTRL_FIFO0_CLR         0x01u   /* As defined by Register map */
    #define myCounter_AUX_CTRL_FIFO1_CLR         0x02u   /* As defined by Register map */
    #define myCounter_AUX_CTRL_FIFO0_LVL         0x04u   /* As defined by Register map */
    #define myCounter_AUX_CTRL_FIFO1_LVL         0x08u   /* As defined by Register map */
    #define myCounter_STATUS_ACTL_INT_EN_MASK    0x10u   /* As defined for the ACTL Register */
    
#endif /* myCounter_UsingFixedFunction */

#endif  /* CY_COUNTER_myCounter_H */


/* [] END OF FILE */

