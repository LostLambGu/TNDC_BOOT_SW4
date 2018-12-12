/********************************************************************************
  * @file    STM32F0xx_IAP/inc/ymodem.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-July-2015
  * @brief   Header for main.c module
  ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _YMODEM_H_
#define _YMODEM_H_
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#include "iwdg.h"
/* Exported types ------------------------------------------------------------*/
#define CMD_STRING_SIZE 128
/* Exported constants --------------------------------------------------------*/
/* Common routines */
#define IS_AF(c) ((c >= 'A') && (c <= 'F'))
#define IS_af(c) ((c >= 'a') && (c <= 'f'))
#define IS_09(c) ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c) IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c) IS_09(c)
#define CONVERTDEC(c) (c - '0')

#define CONVERTHEX_alpha(c) (IS_AF(c) ? (c - 'A' + 10) : (c - 'a' + 10))
#define CONVERTHEX(c) (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

#define PACKET_SEQNO_INDEX (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER (3)
#define PACKET_TRAILER (2)
#define PACKET_OVERHEAD (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE (128)
#define PACKET_1K_SIZE (1024)

#define FILE_NAME_LENGTH (64)
#define FILE_SIZE_LENGTH (16)

#define SOH (0x01)   /* start of 128-byte data packet */
#define STX (0x02)   /* start of 1024-byte data packet */
#define EOT (0x04)   /* end of transmission */
#define ACK (0x06)   /* acknowledge */
#define NAK (0x15)   /* negative acknowledge */
#define CA (0x18)	/* two of these in succession aborts transfer */
#define CRC16 (0x43) /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1 (0x41) /* 'A' == 0x41, abort by user */
#define ABORT2 (0x61) /* 'a' == 0x61, abort by user */

#define B (0x42)
#define D (0x44)
#define E (0x45)
#define F (0x46)
#define G (0x47)
#define H (0x48)
#define I (0x49)
#define J (0x4A)
#define K (0x4B)
#define L (0x4C)
#define M (0x4D)
#define N (0x4E)
#define O (0x4F)
#define P (0x50)
#define Q (0x51)
#define R (0x52)
#define S (0x53)
#define T (0x54)
#define U (0x55)
#define V (0x56)
#define W (0x57)
#define X (0x58)
#define Y (0x59)
#define Z (0x5A)

#define cc (0x63)
#define dd (0x64)

//#define NAK_TIMEOUT             (0x100000)
#define NAK_TIMEOUT1 (0x100000)
#define NAK_TIMEOUT2 (0x200000)
#define NAK_TIMEOUT3 (0x300000)
#define NAK_TIMEOUT4 (0x400000)
#define NAK_TIMEOUT5 (0x500000)
#define NAK_TIMEOUT6 (0x600000)
#define NAK_TIMEOUT7 (0x799999)
#define NAK_TIMEOUT8 (0x1000000)
#define NAK_TIMEOUT9 (0x99999999)

#define MAX_ERRORS (5)

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
extern void Int2Str(uint8_t *str, int32_t intnum);
extern uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum);
extern int32_t Ymodem_Receive(uint8_t *);
extern uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte);
extern uint16_t Cal_CRC16(const uint8_t *data, uint32_t size);
extern uint8_t CalChecksum(const uint8_t *data, uint32_t size);
extern uint8_t Ymodem_Transmit(uint8_t *, const uint8_t *, uint32_t);
extern uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte);
extern uint16_t Cal_CRC16(const uint8_t *data, uint32_t size);
extern int32_t Ymodem_CheckResponse(uint8_t c);
extern void Ymodem_PrepareIntialPacket(uint8_t *data, const uint8_t *fileName, uint32_t *length);
extern void Ymodem_PreparePacket(uint8_t *SourceBuf, uint8_t *data, uint8_t pktNo, uint32_t sizeBlk);
extern void Ymodem_SendPacket(uint8_t *data, uint16_t length);
extern HAL_StatusTypeDef FLASH_If_Erase(uint32_t StartAddr, uint32_t EndAddr);

#endif /* _YMODEM_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
