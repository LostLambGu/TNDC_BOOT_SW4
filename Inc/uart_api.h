#ifndef __UART_API_H__
#define __UART_API_H__

#include "stm32l4xx_hal.h"

#include "usart.h"

#define UART_TRANSMIT_TIMEOUT (50)

#define UART_BUF_MAX_LENGTH (64)
extern uint16_t Uart4RxCount;
extern uint8_t Uart4RxBuffer[UART_BUF_MAX_LENGTH];

extern void Serial_PutString(uint8_t *s);
extern int32_t Receive_Byte(uint8_t *c, uint32_t timeout);
extern uint32_t Send_Byte(uint8_t c);

typedef struct
{
  uint32_t inIndex;
  uint32_t outIndex;
  uint32_t bufSize;
  uint8_t *pBuf;
} UARTChmodemRecTypeDef;

#endif
