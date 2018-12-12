#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <stdlib.h>

#include "uart_api.h"
#include "initialization.h"
#include "flash.h"
#include "ymodem.h"

#define DEFAULT_BL_SIZE 2048

#define NIB_MASK 0xF
#define NIB_SIZE 4
#define HEX_BASE 16
#define BYTE_MASK 0xFF
#define BYTE_SIZE 8

#define CHMODEM_RECEIVE_BUF_SIZE_BYTES (1024 * 3)
#define CHMODEM_MAX_NAME_LEN (64)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define CHMODEM_UARTP (&huart4)

__IO uint8_t UARTChmodemRecModeFlag = FALSE;
UARTChmodemRecTypeDef UARTChmodemRec = {0};
char chmodemFileName[CHMODEM_MAX_NAME_LEN] = {0};
uint8_t chmodemRecBuf[DEFAULT_BL_SIZE] = {0};
uint8_t chmodembuf[CHMODEM_RECEIVE_BUF_SIZE_BYTES] = {0};
static int file_size = 0;
static uint8_t crc8_write_flag = 0;
uint32_t dest = JUMP_TO_APPLICATION_ADDRESS;

static uint8_t chmodem_init(void)
{
  crc8_write_flag = 0;
	return 0;
}

static uint8_t chmodem_deinit(void)
{
	return 0;
}

static int16_t readuart(uint8_t* pBuf, uint16_t size)
{
  int16_t count = 0;

  while (size)
  {
    if (UARTChmodemRec.outIndex != UARTChmodemRec.inIndex)
    {
      pBuf[count] = (UARTChmodemRec.pBuf)[UARTChmodemRec.outIndex];
      UARTChmodemRec.outIndex++;
      UARTChmodemRec.outIndex %= UARTChmodemRec.bufSize;
      count++;
      size--;
    }
    else
    {
      return count;
    }
  }

  return count;
}

static int16_t writefile(uint32_t *pdest, uint8_t* pBuf, uint16_t size)
{
  uint32_t Reslut = 0;
  if (crc8_write_flag == 0) //the first 8 bytes was CRC and file length,not included in .bin
  {
    crc8_write_flag = 1;
    /* Write received data in Flash */
    Reslut = FLASH_If_Write(pdest, (uint64_t *)(pBuf + 8), ((uint16_t)size > 8 ? (uint16_t)(size - 8) / 8 : (uint16_t)(size / 8)));
  }
  else
    Reslut = FLASH_If_Write(pdest, (uint64_t *)pBuf, ((uint16_t)(size % 8) == 0 ? (uint16_t)(size / 8): (uint16_t)(size / 8) + 1));

  FLASH_BUSY_WAIT();
  if (Reslut == 0)
  {
    return 0;
  }
  else
  {
    /* End session */
    return 1;
  }
}

void sendOK(UART_HandleTypeDef *phuart)
{
  HAL_UART_Transmit(phuart, "OK", 2, UART_TRANSMIT_TIMEOUT);
}

void sendStr(UART_HandleTypeDef *phuart, char *msg)
{
  int bts = strlen(msg);

  HAL_UART_Transmit(phuart, (uint8_t *)msg, bts, UART_TRANSMIT_TIMEOUT);
}

// int verifySize(FIL *pf, int esize)
// {
//   if(f_size(pf) == esize)
//   {
//     return TRUE;
//   }
  
//   return FALSE;
// }

int waitPrologue(UART_HandleTypeDef *phuart)
{
  char pre[] = "Connected";
  int actual = strlen(pre);
  int i = 0;
  uint8_t buf[64] = {0};

  if (buf == NULL)
  {
    return 1;
  }
  
  while(i < actual)
  {
    if(readuart(&buf[i], 1) == 1)
    {
      i++;
    }
  }
  buf[i] = '\0';
  if(strncmp(&pre[0], (char *)buf, strlen(pre)))
  {
    return 2;
  }

  sendOK(phuart);
  
  return 0;
}

int decodeSize(char *buf, int len)
{
  int i;
  int num = 0;
  int shift = 0;
  int factor = 1;

  for(i = 0; i < len; i++)
  {
    num += ((buf[len -1 - i] & (NIB_MASK << shift)) >> shift) * factor;
    shift += NIB_SIZE;
    factor *= HEX_BASE;
    num += ((buf[len -1 - i] & (NIB_MASK << shift)) >> shift) * factor;
    shift = 0;
    factor *= HEX_BASE;
  }
  
  return num;
}

int waitSize(UART_HandleTypeDef *phuart)
{
  uint8_t buf[4];
  int i = 0;

  while(i < sizeof(buf))
  {
    if(readuart(&buf[i], 1) == 1)
    {
      i++;
    }
  }
  sendOK(phuart);
  return decodeSize((char *)&buf[0], sizeof(buf));
}

/*
 * Side effect: allocates memery; caller is responsible to release it.
 */
char *waitFileName(UART_HandleTypeDef *phuart, int len)
{
  char *buf;
  int i = 0;
  buf = chmodemFileName;

  if(!buf)
  {
    return NULL;
  }
  memset(chmodemFileName, 0, sizeof(chmodemFileName));
  
  while(i < len)
  {
    if(readuart((uint8_t *)&buf[i], 1) == 1)
    {
      i++;
    }
  }
  buf[i] = '\0';
  // sendOK(phuart); Defer until the file is opened successfully
  return buf;
}

int waitFileNameLen(UART_HandleTypeDef *phuart)
{
  uint8_t buf;
  int num = 0;
  int factor = 1;
  int shift = 0;
  int i = 0;

  while(i < 1)
  {
    if(readuart(&buf, 1) == 1)
    {
      i++;
    }
  }

  num = ((buf & (NIB_MASK << shift)) >> shift) * factor;
  shift += NIB_SIZE;
  factor *= HEX_BASE;
  num += ((buf & (NIB_MASK << shift)) >> shift) * factor;
  sendOK(phuart); 
  return num;
}

int receiveFile(int size, int block)
{
  int bread = 0;
  int left = size;
  // int consumed = 0;
  // int rc = 0;
  int i = 0, j = 0;

  uint8_t *bufp = chmodemRecBuf;
  if (!bufp)
  {
    return 1;
  }

  while (left > 0)
  {
    TickHardwareWatchdog();

    memset(chmodemRecBuf, 0, sizeof(chmodemRecBuf));
    
    if (left > block)
    {
      i = block;
    }
    else
    {
      i = left;
    }

    while (i > 0)
    {
      j = readuart(&bufp[bread], i);
      bread += j;
      i -= j;
    }

    if (bread > 0)
    {
      if (0 != writefile(&dest, &bufp[0], bread))
      {
        return 2;
      }

      left -= bread;
      // consumed += bread;
      // rc++;
      // if (rc == 3500)
      // {
      //   //break;
      //   rc = 0;
      //   //sendProgress(serP, consumed);
      // }
      
      bread = 0;
      // if (left < block)
      // {
      //   HAL_Delay(50);
      //   bread = readuart(&bufp[0], left);
      //   if (bread > 0)
      //   {
      //     writefile(&dest, &bufp[0], bread);
      //     left -= bread;
      //   }
      // }
    }
  }

  return 0;
}

int ChmodemReceive(void)
{
  int name_len;
  char *file_name;
  int bl_size = DEFAULT_BL_SIZE;

  TickHardwareWatchdog();
  if (waitPrologue(CHMODEM_UARTP) != 0)
  {
    return 1;
  }

  TickHardwareWatchdog();
  file_size = waitSize(CHMODEM_UARTP);
  if (file_size <= 0)
  {
    return 2;
  }

  TickHardwareWatchdog();
  name_len = waitFileNameLen(CHMODEM_UARTP);
  if (name_len <= 0)
  {
    return 3;
  }

  TickHardwareWatchdog();
  file_name = waitFileName(CHMODEM_UARTP, name_len);
  if (file_name == NULL)
  {
    return 4;
  }

  TickHardwareWatchdog();
  /* erase user application area */
  if (HAL_OK != FlashMemoryErasePage(ADDR_FLASH_PAGE_16, FLASH_USER_END_ADDR - 1))
  {
    return 5;
  }

  sendStr(CHMODEM_UARTP, "OK");
  HAL_Delay(100);

  if (0 != receiveFile(file_size, bl_size))
  {
    sendStr(CHMODEM_UARTP, "SM");
    return 6;
  }
  else
  {
    sendStr(CHMODEM_UARTP, "OK");
  }
  
  // if(verifySize(&file, file_size) == TRUE)
  // {
  //   sendStr(CHMODEM_UARTP, "OK");
  // }
  // else
  // {
  //   sendStr(CHMODEM_UARTP, "SM");
  //   return 7;
  // }

  return 0;
}

#define CHMODEM_RECEIVE_INTERVAL_MS (1000)
#define CHMODEM_RECEIVE_NODATA_TIMEOUT_COUNT_MAX (1)

int ChmodemReceiveLoadFile(void)
{
  int ret = 0;
  uint8_t Number[10] = {0};
	char *ptmp = NULL;
  uint32_t writeaddr = 0;
  FirmwareInfoTypeDef FirmwareInfo = {0};

  if (chmodembuf == NULL)
  {
    return 3;
  }

  memset(&UARTChmodemRec, 0, sizeof(UARTChmodemRec));
  memset(chmodembuf, 0, CHMODEM_RECEIVE_BUF_SIZE_BYTES);
  UARTChmodemRec.pBuf = chmodembuf;
  UARTChmodemRec.bufSize = CHMODEM_RECEIVE_BUF_SIZE_BYTES;
  UARTChmodemRecModeFlag = TRUE;

  if (0 != chmodem_init())
  {
    UARTChmodemRecModeFlag = FALSE;
    return 2;
  }

  TickHardwareWatchdog();

  ret = ChmodemReceive();

  TickHardwareWatchdog();
  if (ret == 0)
  {
    // Send Successfully Info To UART
		Serial_PutString("\r\n\r\n\r\n");
		Serial_PutString("\r\n>>Programming Completed Successfully");
		Serial_PutString("\r\n>>App Name:");
		Serial_PutString((uint8_t *)chmodemFileName);
		Serial_PutString("\r\n");
		Serial_PutString("\r\n>>App Size:");
		Int2Str(Number, file_size);
		Serial_PutString(Number);
		Serial_PutString(" Bytes");

		FirmwareInfo.IAPFlag = IAP_APP_CHMODEM;
		FirmwareInfo.FirmwareSize = file_size;

		ptmp = strstr((char *)chmodemFileName, MODEM_VERSION_HEAD);
		if (NULL != ptmp)
		{
			strncpy(FirmwareInfo.Version, (char *)(ptmp + strlen(MODEM_VERSION_HEAD)), strlen(APP_DEFAULT_VERSION));
		}
		else
		{
			strcpy(FirmwareInfo.Version, APP_DEFAULT_VERSION);
		}
		
		FlashMemoryErasePage(ADDR_FLASH_PAGE_15, ADDR_FLASH_PAGE_15 + FLASH_PAGE_SIZE - 1);
		FLASH_BUSY_WAIT();
		writeaddr = IAP_FLAG;
		FLASH_If_Write(&writeaddr, (uint64_t *)&FirmwareInfo, sizeof(FirmwareInfo) / 8);
		FLASH_BUSY_WAIT();

    // Wirte Flag
		//Check  the written value
		if ((((FirmwareInfoTypeDef *)IAP_FLAG)->IAPFlag == (uint32_t)IAP_APP_CHMODEM))
			Serial_PutString("\r\n>>Write Flag Success");
		else
		{
			Serial_PutString("\r\n>>Write Flag Fail");
		}

		HAL_FLASH_Lock();
		/* Clear all FLASH flags */
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_ALL_ERRORS);

		Serial_PutString("\r\n>>Jump to application...");
		Serial_PutString("\r\n\r\n\r\n");
		HAL_Delay(10);
		//
		TickHardwareWatchdog();
		// Jump to user application
		SoftwareJumping(JUMP_TO_APPLICATION_ADDRESS);
  }
  else
  {
    TickHardwareWatchdog();
		// Show Fail Programming
		Serial_PutString("ProgramWord Fail\r\n");
  }

  UARTChmodemRecModeFlag = FALSE;

  return chmodem_deinit();
}
