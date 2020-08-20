/*******************************************************************************
* File Name: aout0.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_aout0_H) /* Pins aout0_H */
#define CY_PINS_aout0_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "aout0_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 aout0__PORT == 15 && ((aout0__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    aout0_Write(uint8 value);
void    aout0_SetDriveMode(uint8 mode);
uint8   aout0_ReadDataReg(void);
uint8   aout0_Read(void);
void    aout0_SetInterruptMode(uint16 position, uint16 mode);
uint8   aout0_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the aout0_SetDriveMode() function.
     *  @{
     */
        #define aout0_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define aout0_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define aout0_DM_RES_UP          PIN_DM_RES_UP
        #define aout0_DM_RES_DWN         PIN_DM_RES_DWN
        #define aout0_DM_OD_LO           PIN_DM_OD_LO
        #define aout0_DM_OD_HI           PIN_DM_OD_HI
        #define aout0_DM_STRONG          PIN_DM_STRONG
        #define aout0_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define aout0_MASK               aout0__MASK
#define aout0_SHIFT              aout0__SHIFT
#define aout0_WIDTH              1u

/* Interrupt constants */
#if defined(aout0__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in aout0_SetInterruptMode() function.
     *  @{
     */
        #define aout0_INTR_NONE      (uint16)(0x0000u)
        #define aout0_INTR_RISING    (uint16)(0x0001u)
        #define aout0_INTR_FALLING   (uint16)(0x0002u)
        #define aout0_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define aout0_INTR_MASK      (0x01u) 
#endif /* (aout0__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define aout0_PS                     (* (reg8 *) aout0__PS)
/* Data Register */
#define aout0_DR                     (* (reg8 *) aout0__DR)
/* Port Number */
#define aout0_PRT_NUM                (* (reg8 *) aout0__PRT) 
/* Connect to Analog Globals */                                                  
#define aout0_AG                     (* (reg8 *) aout0__AG)                       
/* Analog MUX bux enable */
#define aout0_AMUX                   (* (reg8 *) aout0__AMUX) 
/* Bidirectional Enable */                                                        
#define aout0_BIE                    (* (reg8 *) aout0__BIE)
/* Bit-mask for Aliased Register Access */
#define aout0_BIT_MASK               (* (reg8 *) aout0__BIT_MASK)
/* Bypass Enable */
#define aout0_BYP                    (* (reg8 *) aout0__BYP)
/* Port wide control signals */                                                   
#define aout0_CTL                    (* (reg8 *) aout0__CTL)
/* Drive Modes */
#define aout0_DM0                    (* (reg8 *) aout0__DM0) 
#define aout0_DM1                    (* (reg8 *) aout0__DM1)
#define aout0_DM2                    (* (reg8 *) aout0__DM2) 
/* Input Buffer Disable Override */
#define aout0_INP_DIS                (* (reg8 *) aout0__INP_DIS)
/* LCD Common or Segment Drive */
#define aout0_LCD_COM_SEG            (* (reg8 *) aout0__LCD_COM_SEG)
/* Enable Segment LCD */
#define aout0_LCD_EN                 (* (reg8 *) aout0__LCD_EN)
/* Slew Rate Control */
#define aout0_SLW                    (* (reg8 *) aout0__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define aout0_PRTDSI__CAPS_SEL       (* (reg8 *) aout0__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define aout0_PRTDSI__DBL_SYNC_IN    (* (reg8 *) aout0__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define aout0_PRTDSI__OE_SEL0        (* (reg8 *) aout0__PRTDSI__OE_SEL0) 
#define aout0_PRTDSI__OE_SEL1        (* (reg8 *) aout0__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define aout0_PRTDSI__OUT_SEL0       (* (reg8 *) aout0__PRTDSI__OUT_SEL0) 
#define aout0_PRTDSI__OUT_SEL1       (* (reg8 *) aout0__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define aout0_PRTDSI__SYNC_OUT       (* (reg8 *) aout0__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(aout0__SIO_CFG)
    #define aout0_SIO_HYST_EN        (* (reg8 *) aout0__SIO_HYST_EN)
    #define aout0_SIO_REG_HIFREQ     (* (reg8 *) aout0__SIO_REG_HIFREQ)
    #define aout0_SIO_CFG            (* (reg8 *) aout0__SIO_CFG)
    #define aout0_SIO_DIFF           (* (reg8 *) aout0__SIO_DIFF)
#endif /* (aout0__SIO_CFG) */

/* Interrupt Registers */
#if defined(aout0__INTSTAT)
    #define aout0_INTSTAT            (* (reg8 *) aout0__INTSTAT)
    #define aout0_SNAP               (* (reg8 *) aout0__SNAP)
    
	#define aout0_0_INTTYPE_REG 		(* (reg8 *) aout0__0__INTTYPE)
#endif /* (aout0__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_aout0_H */


/* [] END OF FILE */
