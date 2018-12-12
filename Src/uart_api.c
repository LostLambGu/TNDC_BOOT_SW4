#include <string.h>

#include "ymodem.h"

#include "usrtimer.h"
#include "uart_api.h"

uint16_t Uart4RxCount = 0;
uint8_t Uart4RxBuffer[UART_BUF_MAX_LENGTH] = {'\0'};

extern void CheckUARTRecTimerCallback(uint8_t Status);

/**
  * @brief  Print a string on the HyperTerminal
  * @param  s: The string to be printed
  * @retval None
  */
void Serial_PutString(uint8_t *s)
{
  HAL_UART_Transmit(&huart4, s, strlen((const char *)s), UART_TRANSMIT_TIMEOUT);
}

int32_t Receive_Byte(uint8_t *c, uint32_t timeout)
{
  SoftwareTimerStart(&UARTRecTimer);

  while (1)
  {
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE) != RESET)
    {
      *c = (uint8_t)(huart4.Instance->RDR & (uint8_t)0xff);
      SoftwareTimerReset(&UARTRecTimer, CheckUARTRecTimerCallback, CHECK_UART4_REC_TIMEOUT);
      SoftwareTimerStop(&UARTRecTimer);
      return 0;
    }

    SoftwareCheckTimerStatus();
    if (UARTRecTimer.IsTimeOverflow)
    {
      break;
    }
  }

  SoftwareTimerReset(&UARTRecTimer, CheckUARTRecTimerCallback, CHECK_UART4_REC_TIMEOUT);
  SoftwareTimerStop(&UARTRecTimer);
  return -1;
}

uint32_t Send_Byte(uint8_t c)
{
  huart4.Instance->TDR = c;
  while (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_TXE) == RESET)
  {
  }
  return 0;
}
