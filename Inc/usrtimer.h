/*******************************************************************************
* File Name          : UserTimer.h
* Author               : Taotao Yan
* Description        : This file provides all the UserTimer functions.

* History:
*  1/13/2016 : UserTimer V1.00
*******************************************************************************/

#ifndef _USRTIM_H
#define _USRTIM_H

#include "stm32l4xx_hal.h"

#define CHECK_UART4_REC_TIMEOUT (1 * 5000)

// define softtimer structure.
typedef struct _TIMER
{
	__IO uint32_t TimeOutVal; //time out value
	uint8_t TimeId;			  // time ID
	uint32_t RecTickVal;	  //softtimer setting value
	uint8_t IsTimeOverflow;   //time out flag
	uint8_t TimerStartCounter;
	void (*Routine)(uint8_t);
} TIMER;

// Variable Declared
extern TIMER UARTRecTimer;

//Function Declare
extern void InitSoftwareTimers(void);
extern void SoftwareTimerCounter(void);
extern void SoftwareCheckTimerStatus(void);
extern void SoftwareTimerStart(TIMER *timer);
extern void SoftwareTimerStop(TIMER *timer);
extern void SoftwareTimerReset(TIMER *timer, void (*Routine)(uint8_t), uint32_t timeout);
extern void SoftwareTimerCreate(TIMER *timer, uint8_t TimeId, void (*Routine)(uint8_t), uint32_t timeout);
extern uint8_t IsSoftwareTimeOut(TIMER *timer);

#endif

/*******************************************************************************
    Copyrights (C) Asiatelco Technologies Co., 2003-2017. All rights reserved
                                  End Of The File
*******************************************************************************/
