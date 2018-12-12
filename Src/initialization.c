/*******************************************************************************
* File Name          : initialization.c
* Author               : Jevon
* Description        : This file provides all the initialization functions.

* History:
*  04/29/2015 : initialization V1.00
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#include "initialization.h"
#include "usrtimer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void SystemInitialization(void)
{
	InitSoftwareTimers();
}

void SoftwareJumping(uint32_t Address)
{
	uint32_t JumpAddress;
	pFunction Jump_To_Application;

	// Lock
	HAL_FLASH_Lock();
	// Jump to Address
	JumpAddress = *(__IO uint32_t *)(Address + 4);
	Jump_To_Application = (pFunction)JumpAddress;

	// Initialize user application's Stack Pointer
	__set_MSP(*(__IO uint32_t *)Address);

	// Jump to application
	Jump_To_Application();
}

void SoftwareReset(void)
{
#if 0
	NVIC_SystemReset();
#else
#define JUMP_ADDRESS 0x08000000
	uint32_t JumpAddress = 0;
	Function_Pointer Jump_Function;
	/* Jump to the illegal opcode address */
	JumpAddress = *(__IO uint32_t *)(JUMP_ADDRESS + 4);
	Jump_Function = (Function_Pointer)JumpAddress;

	/* Initialize user application's Stack Pointer */
	__set_MSP(*(__IO uint32_t *)JUMP_ADDRESS);

	/* Jump to Address */
	Jump_Function();
#endif
}

/*******************************************************************************
End Of The File
*******************************************************************************/
