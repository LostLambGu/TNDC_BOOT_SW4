/*******************************************************************************
* File Name          : UserTimer.c
* Author               : Taotao Yan
* Description        : This file provides all the UserTimer functions.

* History:
*  1/13/2016 : UserTimer V1.00
*******************************************************************************/
#include "stm32l4xx_hal.h"

#include "usrtimer.h"
#include "uart_api.h"

/*----------------------------------------------------------------------------*/
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
/*----------------------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
TIMER UARTRecTimer;

void CheckUARTRecTimerCallback(uint8_t Status)
{
}

void InitSoftwareTimers(void)
{
	// UARTRecTimer
	SoftwareTimerCreate(&UARTRecTimer, 1, CheckUARTRecTimerCallback, CHECK_UART4_REC_TIMEOUT);
	// SoftwareTimerStart(&UARTRecTimer);
}

void SoftwareTimerCounter(void)
{
	if (UARTRecTimer.TimerStartCounter == TRUE)
		UARTRecTimer.TimeOutVal++;
}

void SoftwareCheckTimerStatus(void)
{
	if (IsSoftwareTimeOut(&UARTRecTimer) == TRUE)
	{
	}
}

void SoftwareTimerStart(TIMER *timer)
{
	timer->TimerStartCounter = TRUE;
}

void SoftwareTimerStop(TIMER *timer)
{
	timer->TimerStartCounter = FALSE;
}

void SoftwareTimerReset(TIMER *timer, void (*Routine)(uint8_t), uint32_t timeout)
{
	timer->TimerStartCounter = FALSE; //timer stop
	timer->RecTickVal = timeout;	  //softtimer setting value
	timer->TimeOutVal = 0;			  //time out value
	timer->IsTimeOverflow = FALSE;	//time out flag
	timer->Routine = Routine;
}

void SoftwareTimerCreate(TIMER *timer, uint8_t TimeId, void (*Routine)(uint8_t), uint32_t timeout)
{
	timer->TimeId = TimeId;
	timer->RecTickVal = timeout;	  //softtimer setting value
	timer->TimeOutVal = 0;			  //time out value
	timer->IsTimeOverflow = FALSE;	//time out flag
	timer->TimerStartCounter = FALSE; //timer stop
	timer->Routine = Routine;
}

uint8_t IsSoftwareTimeOut(TIMER *timer)
{
	//ET0 = 0;
	if (timer->IsTimeOverflow == FALSE)
	{
		//After gSysTick and timer->TimeOutVal overflow,
		//the software timer function can still work well
		//the next statement is equivalent to:
		//(gSysTick - timer->TimeOutVal) < 0x80000000
		if (timer->TimeOutVal > timer->RecTickVal)
		{
			timer->TimerStartCounter = FALSE;
			timer->IsTimeOverflow = TRUE;
			timer->Routine(timer->TimeId);
		}
	}
	//ET0 = 1;

	return timer->IsTimeOverflow;
}

/*******************************************************************************
    Copyrights (C) Asiatelco Technologies Co., 2003-2018. All rights reserved
                                 End Of The File
*******************************************************************************/
