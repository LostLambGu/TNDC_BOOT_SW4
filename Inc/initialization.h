/*******************************************************************************
* File Name          : initialization.h
* Author             : Yanjie Gu
* Description        : This file provides all the initialization functions.

* History:
*  04/29/2015 : initialization V1.00
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _INITIALIZATION_SOFTWARE_H
#define _INITIALIZATION_SOFTWARE_H
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#define min(X, Y) ((X) < (Y) ? (X) : (Y))
#define max(X, Y) ((X) > (Y) ? (X) : (Y))

#define FunStates FunctionalState

typedef void (*pFunction)(void);
typedef void (*Function_Pointer)(void);

extern void SystemInitialization(void);
extern void SoftwareJumping(uint32_t Address);
extern void SoftwareReset(void);

#endif /* _INITIALIZATION_SOFTWARE_H */

/*******************************************************************************
End Of The File
*******************************************************************************/
