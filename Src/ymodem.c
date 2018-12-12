/********************************************************************************
  * @file    STM32F0xx_IAP/src/ymodem.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-July-2015
  * @brief   THis file provides all ymodem functions.
  ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "stm32l4xx_hal.h"
#include "ymodem.h"
#include "main.h"

#include "flash.h"
#include "uart_api.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint8_t FileName[];
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//FLASH_ProcessTypeDef pFlash;

/**
  * @brief  Convert an Integer to a string
  * @param  str: The string
  * @param  intnum: The intger to be converted
  * @retval None
  */
void Int2Str(uint8_t *str, int32_t intnum)
{
  uint32_t i, Div = 1000000000, j = 0, Status = 0;

  for (i = 0; i < 10; i++)
  {
    str[j++] = (intnum / Div) + 48;
    intnum = intnum % Div;
    Div /= 10;
    if ((str[j - 1] == '0') & (Status == 0))
    {
      j = 0;
    }
    else
    {
      Status++;
    }
  }
}

/**
  * @brief  Convert a string to an integer
  * @param  inputstr: The string to be converted
  * @param  intnum: The intger value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
  {
    if (inputstr[2] == '\0')
    {
      return 0;
    }
    for (i = 2; i < 11; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1; */
        res = 1;
        break;
      }
      if (ISVALIDHEX(inputstr[i]))
      {
        val = (val << 4) + CONVERTHEX(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* over 8 digit hex --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }
  else /* max 10-digit decimal input */
  {
    for (i = 0; i < 11; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1 */
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
      {
        val = val << 10;
        *intnum = val;
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
      {
        val = val << 20;
        *intnum = val;
        res = 1;
        break;
      }
      else if (ISVALIDDEC(inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* Over 10 digit decimal --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }

  return res;
}

/**
  * @brief  Update CRC 16 for input byte
  * @param  CRC input value 
  * @param  input byte
  * @retval Updated CRC value
  */
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
  uint32_t crc = crcIn;
  uint32_t in = byte | 0x100;

  do
  {
    crc <<= 1;
    in <<= 1;

    if (in & 0x100)
    {
      ++crc;
    }

    if (crc & 0x10000)
    {
      crc ^= 0x1021;
    }
  } while (!(in & 0x10000));

  return (crc & 0xffffu);
}

/**
  * @brief  Cal CRC 16 for YModem Packet
  * @param  data
  * @param  length
  * @retval CRC value
  */
uint16_t Cal_CRC16(const uint8_t *data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t *dataEnd = data + size;

  while (data < dataEnd)
  {
    crc = UpdateCRC16(crc, *data++);
  }
  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);

  return (crc & 0xffffu);
}

/**
  * @brief  Cal Check sum for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint8_t CalChecksum(const uint8_t *data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *dataEnd = data + size;

  while (data < dataEnd)
  {
    sum += *data++;
  }

  return (sum & 0xffu);
}

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  * @param  timeout
  *          0: end of transmission
  *          -1: abort by sender
  *          >0: packet length
  * @retval 0: normally return
  *         -1: timeout or packet error
  *         1: abort by user
  */
static uint8_t Receive_Packet(uint8_t *data, int32_t *length, uint32_t timeout)
{
  uint16_t i, packet_size, computedcrc;
  uint8_t c;
  *length = 0;
  if (Receive_Byte(&c, timeout) != 0)
  {
    return 2;
  }
  switch (c)
  {
  case SOH:
    packet_size = PACKET_SIZE;
    break;
  case STX:
    packet_size = PACKET_1K_SIZE;
    break;
  case EOT:
    return 0;
  case CA:
    if ((Receive_Byte(&c, timeout) == 0) && (c == CA))
    {
      *length = -1;
      return 0;
    }
    else
    {
      return 3;
    }
  case ABORT1:
  case ABORT2:
    return 1;
  default:
    return 4;
  }
  *data = c;
  for (i = 1; i < (packet_size + PACKET_OVERHEAD); i++)
  {
    if (Receive_Byte(data + i, timeout) != 0)
    {
      return 5;
    }
  }
  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
  {
    return 6;
  }

  /* Compute the CRC */
  computedcrc = Cal_CRC16(&data[PACKET_HEADER], (uint32_t)packet_size);
  /* Check that received CRC match the already computed CRC value
     data[packet_size+3]<<8) | data[packet_size+4] contains the received CRC 
     computedcrc contains the computed CRC value */
  if (computedcrc != (uint16_t)((data[packet_size + 3] << 8) | data[packet_size + 4]))
  {
    /* CRC error */
    return 7;
  }

  *length = packet_size;
  return 0;
}

/**
  * @brief  Receive a file using the ymodem protocol
  * @param  buf: Address of the first byte
  * @retval The size of the file
  */
int32_t Ymodem_Receive(uint8_t *buf)
{
  uint8_t crc8_write_flag = 0;
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr, *buf_ptr;
  int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
  uint32_t flashdestination, ramsource;

  /* Initialize flashdestination variable */
  flashdestination = JUMP_TO_APPLICATION_ADDRESS;

  for (session_done = 0, errors = 0, session_begin = 0;;)
  {
    for (packets_received = 0, file_done = 0, buf_ptr = buf;;)
    {
      switch (Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT1))
      {
      case 0:
      {
        errors = 0;
        // Refresh IWDG: reload counter
        TickHardwareWatchdog();
        switch (packet_length)
        {
        /* Abort by sender */
        case -1:
          Send_Byte(ACK);
          return 0;
        /* End of transmission */
        case 0:
          Send_Byte(ACK);
          Send_Byte(CRC16);
          file_done = 1;
          break;
        /* Normal packet */
        default:
          if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))
          {
            Send_Byte(NAK);
          }
          else
          {
            if (packets_received == 0)
            {
              /* Filename packet */
              if (packet_data[PACKET_HEADER] != 0)
              {
                /* Filename packet has valid data */
                for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);)
                {
                  FileName[i++] = *file_ptr++;
                }
                FileName[i++] = '\0';
                for (i = 0, file_ptr++; (*file_ptr != ' ') && (i < (FILE_SIZE_LENGTH - 1));)
                {
                  file_size[i++] = *file_ptr++;
                }
                file_size[i++] = '\0';
                Str2Int(file_size, &size);

                /* Test the size of the image to be sent */
                /* Image size is greater than Flash size */
                if (size > (USER_FLASH_SIZE + 1))
                {
                  /* End session */
                  Send_Byte(CA);
                  Send_Byte(CA);
                  return -1;
                }
                /* erase user application area */
                FlashMemoryErasePage(ADDR_FLASH_PAGE_16, FLASH_USER_END_ADDR - 1);

                Send_Byte(ACK);
                Send_Byte(CRC16);
              }
              /* Filename packet is empty, end session */
              else
              {
                Send_Byte(ACK);
                file_done = 1;
                session_done = 1;
                break;
              }
            }
            /* Data packet */
            else
            {
              uint32_t Reslut = 0;
              memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length); //PACKET_OVERHEAD PACKET_HEADER
              ramsource = (uint32_t)buf;
              if ((packet_data[PACKET_SEQNO_INDEX] == 0x01) && (crc8_write_flag == 0)) //the first 8 bytes was CRC and file length,not included in .bin
              {
                crc8_write_flag = 1;
                /* Write received data in Flash */
                Reslut = FLASH_If_Write(&flashdestination, (uint64_t *)(ramsource + 8), ((uint16_t)packet_length > 8 ? (uint16_t)(packet_length - 8) / 8 : (uint16_t)(packet_length / 8)));
              }
              else
                Reslut = FLASH_If_Write(&flashdestination, (uint64_t *)ramsource, (uint16_t)(packet_length / 8));
              if (Reslut == 0)
              {
                Send_Byte(ACK);
              }
              else if (Reslut == 1)
              {
                /* End session */
                Send_Byte(CA);
                Send_Byte(CA);
                return -2;
              }
              else if (Reslut == 2)
              {
                /* End session */
                Send_Byte(CA);
                Send_Byte(CA);
                return -3;
              }
              else
              {
                /* End session */
                Send_Byte(CA);
                Send_Byte(CA);
                return -4;
              }
            }
            packets_received++;
            session_begin = 1;
          }
        }
      }
      break;
      case 1:
        // Refresh IWDG: reload counter
        TickHardwareWatchdog();
        Send_Byte(CA);
        Send_Byte(CA);
        return -5;
      default:
        // Refresh IWDG: reload counter
        TickHardwareWatchdog();
        if (session_begin > 0)
        {
          errors++;
        }
        if (errors > MAX_ERRORS)
        {
          Send_Byte(CA);
          Send_Byte(CA);
          return 0;
        }
        Send_Byte(CRC16);
        // Refresh IWDG: reload counter
        TickHardwareWatchdog();
        break;
      }
      if (file_done != 0)
      {
        break;
      }
    }
    if (session_done != 0)
    {
      break;
    }
  }
  return (int32_t)size;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
