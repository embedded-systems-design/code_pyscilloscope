/*******************************************************************************
* File Name: myADC1_AMux.h
* Version 1.80
*
*  Description:
*    This file contains the constants and function prototypes for the Analog
*    Multiplexer User Module AMux.
*
*   Note:
*
********************************************************************************
* Copyright 2008-2010, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
********************************************************************************/

#if !defined(CY_AMUX_myADC1_AMux_H)
#define CY_AMUX_myADC1_AMux_H

#include "cyfitter.h"
#include "cyfitter_cfg.h"

#if ((CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC3) || \
         (CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC4) || \
         (CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC5))    
    #include "cytypes.h"
#else
    #include "syslib/cy_syslib.h"
#endif /* ((CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC3) */


/***************************************
*        Function Prototypes
***************************************/

void myADC1_AMux_Start(void) ;
#define myADC1_AMux_Init() myADC1_AMux_Start()
void myADC1_AMux_FastSelect(uint8 channel) ;
/* The Stop, Select, Connect, Disconnect and DisconnectAll functions are declared elsewhere */
/* void myADC1_AMux_Stop(void); */
/* void myADC1_AMux_Select(uint8 channel); */
/* void myADC1_AMux_Connect(uint8 channel); */
/* void myADC1_AMux_Disconnect(uint8 channel); */
/* void myADC1_AMux_DisconnectAll(void) */


/***************************************
*         Parameter Constants
***************************************/

#define myADC1_AMux_CHANNELS  2u
#define myADC1_AMux_MUXTYPE   1
#define myADC1_AMux_ATMOSTONE 0

/***************************************
*             API Constants
***************************************/

#define myADC1_AMux_NULL_CHANNEL 0xFFu
#define myADC1_AMux_MUX_SINGLE   1
#define myADC1_AMux_MUX_DIFF     2


/***************************************
*        Conditional Functions
***************************************/

#if myADC1_AMux_MUXTYPE == myADC1_AMux_MUX_SINGLE
# if !myADC1_AMux_ATMOSTONE
#  define myADC1_AMux_Connect(channel) myADC1_AMux_Set(channel)
# endif
# define myADC1_AMux_Disconnect(channel) myADC1_AMux_Unset(channel)
#else
# if !myADC1_AMux_ATMOSTONE
void myADC1_AMux_Connect(uint8 channel) ;
# endif
void myADC1_AMux_Disconnect(uint8 channel) ;
#endif

#if myADC1_AMux_ATMOSTONE
# define myADC1_AMux_Stop() myADC1_AMux_DisconnectAll()
# define myADC1_AMux_Select(channel) myADC1_AMux_FastSelect(channel)
void myADC1_AMux_DisconnectAll(void) ;
#else
# define myADC1_AMux_Stop() myADC1_AMux_Start()
void myADC1_AMux_Select(uint8 channel) ;
# define myADC1_AMux_DisconnectAll() myADC1_AMux_Start()
#endif

#endif /* CY_AMUX_myADC1_AMux_H */


/* [] END OF FILE */
