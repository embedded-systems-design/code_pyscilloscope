/*******************************************************************************
* File Name: myClock.h
* Version 2.20
*
*  Description:
*   Provides the function and constant definitions for the clock component.
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CLOCK_myClock_H)
#define CY_CLOCK_myClock_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
* Conditional Compilation Parameters
***************************************/

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component cy_clock_v2_20 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */


/***************************************
*        Function Prototypes
***************************************/

void myClock_Start(void) ;
void myClock_Stop(void) ;

#if(CY_PSOC3 || CY_PSOC5LP)
void myClock_StopBlock(void) ;
#endif /* (CY_PSOC3 || CY_PSOC5LP) */

void myClock_StandbyPower(uint8 state) ;
void myClock_SetDividerRegister(uint16 clkDivider, uint8 restart) 
                                ;
uint16 myClock_GetDividerRegister(void) ;
void myClock_SetModeRegister(uint8 modeBitMask) ;
void myClock_ClearModeRegister(uint8 modeBitMask) ;
uint8 myClock_GetModeRegister(void) ;
void myClock_SetSourceRegister(uint8 clkSource) ;
uint8 myClock_GetSourceRegister(void) ;
#if defined(myClock__CFG3)
void myClock_SetPhaseRegister(uint8 clkPhase) ;
uint8 myClock_GetPhaseRegister(void) ;
#endif /* defined(myClock__CFG3) */

#define myClock_Enable()                       myClock_Start()
#define myClock_Disable()                      myClock_Stop()
#define myClock_SetDivider(clkDivider)         myClock_SetDividerRegister(clkDivider, 1u)
#define myClock_SetDividerValue(clkDivider)    myClock_SetDividerRegister((clkDivider) - 1u, 1u)
#define myClock_SetMode(clkMode)               myClock_SetModeRegister(clkMode)
#define myClock_SetSource(clkSource)           myClock_SetSourceRegister(clkSource)
#if defined(myClock__CFG3)
#define myClock_SetPhase(clkPhase)             myClock_SetPhaseRegister(clkPhase)
#define myClock_SetPhaseValue(clkPhase)        myClock_SetPhaseRegister((clkPhase) + 1u)
#endif /* defined(myClock__CFG3) */


/***************************************
*             Registers
***************************************/

/* Register to enable or disable the clock */
#define myClock_CLKEN              (* (reg8 *) myClock__PM_ACT_CFG)
#define myClock_CLKEN_PTR          ((reg8 *) myClock__PM_ACT_CFG)

/* Register to enable or disable the clock */
#define myClock_CLKSTBY            (* (reg8 *) myClock__PM_STBY_CFG)
#define myClock_CLKSTBY_PTR        ((reg8 *) myClock__PM_STBY_CFG)

/* Clock LSB divider configuration register. */
#define myClock_DIV_LSB            (* (reg8 *) myClock__CFG0)
#define myClock_DIV_LSB_PTR        ((reg8 *) myClock__CFG0)
#define myClock_DIV_PTR            ((reg16 *) myClock__CFG0)

/* Clock MSB divider configuration register. */
#define myClock_DIV_MSB            (* (reg8 *) myClock__CFG1)
#define myClock_DIV_MSB_PTR        ((reg8 *) myClock__CFG1)

/* Mode and source configuration register */
#define myClock_MOD_SRC            (* (reg8 *) myClock__CFG2)
#define myClock_MOD_SRC_PTR        ((reg8 *) myClock__CFG2)

#if defined(myClock__CFG3)
/* Analog clock phase configuration register */
#define myClock_PHASE              (* (reg8 *) myClock__CFG3)
#define myClock_PHASE_PTR          ((reg8 *) myClock__CFG3)
#endif /* defined(myClock__CFG3) */


/**************************************
*       Register Constants
**************************************/

/* Power manager register masks */
#define myClock_CLKEN_MASK         myClock__PM_ACT_MSK
#define myClock_CLKSTBY_MASK       myClock__PM_STBY_MSK

/* CFG2 field masks */
#define myClock_SRC_SEL_MSK        myClock__CFG2_SRC_SEL_MASK
#define myClock_MODE_MASK          (~(myClock_SRC_SEL_MSK))

#if defined(myClock__CFG3)
/* CFG3 phase mask */
#define myClock_PHASE_MASK         myClock__CFG3_PHASE_DLY_MASK
#endif /* defined(myClock__CFG3) */

#endif /* CY_CLOCK_myClock_H */


/* [] END OF FILE */
