#include <string.h>

#include "flash.h"
#include "iwdg.h"
#include "ymodem.h"
#include "initialization.h"
#include "uart_api.h"

#include "SdioEmmcDrive.h"

uint8_t tab_1024[1024] = {0}; // __attribute__((at(0x20001400)));

uint8_t FileName[FILE_NAME_LENGTH];

// uint32_t FLASH_GetWRP(void)
// {
// 	/* Return the FLASH write protection Register value */
// 	return (uint32_t)(READ_REG(FLASH->WRPR));
// }

void FLASH_If_Init(void)
{
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	/* Clear OPTVERR bit set on virgin samples */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	/* Clear all FLASH flags */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_ALL_ERRORS);
}

void FLASH_BUSY_WAIT(void)
{
	while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY))
	{

	}
}

// uint32_t FLASH_If_GetWriteProtectionStatus(void)
// {
// 	/* Test if any page of Flash memory where program user will be loaded is write protected */
// 	if ((FLASH_GetWRP() & FLASH_PROTECTED_PAGES) != FLASH_PROTECTED_PAGES)
// 	{
// 		return 1;
// 	}
// 	else
// 	{
// 		return 0;
// 	}
// }

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;
  
  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
  
  return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr)
{
  uint32_t bank = 0;
  
  if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
  {
  	/* No Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
  	/* Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }
  
  return bank;
}
HAL_StatusTypeDef FlashMemoryErasePage(uint32_t StartAddr, uint32_t EndAddr)
{
	// #define ADDR_FLASH_PAGE_128   ((uint32_t)0x08040000) /* Base @ of Page 128, 2 Kbytes */
	FLASH_EraseInitTypeDef EraseInitStruct;
	HAL_StatusTypeDef Status = HAL_OK;
	uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0, another = 0;
	uint32_t PAGEError = 0, addr1 = 0, addr2 = 0;

	if (EndAddr >= ((uint32_t)0x08040000))
	{
		addr1 = StartAddr;
		addr2 = ((uint32_t)0x08040000) - 1;
		another = 1;
	}
	else
	{
		addr1 = StartAddr;
		addr2 = EndAddr;
	}

ERASE_ANOTHER_BANK:
	/* Get the 1st page to erase */
	FirstPage = GetPage(addr1);
	/* Get the number of pages to erase from 1st page */
	NbOfPages = GetPage(addr2) - FirstPage + 1;
	/* Get the bank */
	BankNumber = GetBank(addr1);
	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = BankNumber;
	EraseInitStruct.Page = FirstPage;
	EraseInitStruct.NbPages = NbOfPages;

	/* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
	Status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
	if (Status != HAL_OK)
	{
		/*
      Error occurred while page erase.
      User can add here some code to deal with this error.
      PAGEError will contain the faulty page and then to know the code error on this page,
      user can call function 'HAL_FLASH_GetError()'
    */
		return Status;
	}

	if (another)
	{
		another--;
		addr1 = (uint32_t)0x08040000;
		addr2 = EndAddr;
		goto ERASE_ANOTHER_BANK;
	}

	return Status;
}

uint32_t FLASH_If_Write(__IO uint32_t *FlashAddress, uint64_t *Data, uint16_t DataLength)
{
	uint32_t i = 0;

	for (i = 0; (i < DataLength) && (*FlashAddress <= (USER_FLASH_END_ADDRESS - 8)); i++)
	{
		/* the operation will be done by word */
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, *FlashAddress, *(Data + i)) == HAL_OK)
		{

			/* Check the written value */
			if (*(uint64_t *)*FlashAddress != *(Data + i))
			{
				return (2);
			}
			/* Increment FLASH destination address */
			*FlashAddress += 8;
		}
		else
		{
			return (1);
		}
	}
	return (0);
}

static void ymodem_init(void)
{
	UART_HandleTypeDef *huart = &huart4;

	__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
    __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);
}

static void ymodem_deinit(void)
{
	MX_UART4_Init();
}

/* USER CODE BEGIN PV */
void SerialDownload(void)
{
	int32_t Size = 0;
	uint32_t writeaddr = 0;
	uint8_t Number[10] = {0};
	char *ptmp = NULL;
	FirmwareInfoTypeDef FirmwareInfo = {0};
	
	TickHardwareWatchdog();
	
	ymodem_init();
	// Print Out
	Size = Ymodem_Receive(&tab_1024[0]);
	if (Size > 0)
	{
		// Send Successfully Info To UART
		Serial_PutString("\r\n\r\n\r\n");
		Serial_PutString("\r\n>>Programming Completed Successfully");
		Serial_PutString("\r\n>>App Name:");
		Serial_PutString(FileName);
		Serial_PutString("\r\n");
		Serial_PutString("\r\n>>App Size:");
		Int2Str(Number, Size);
		Serial_PutString(Number);
		Serial_PutString(" Bytes");

		FirmwareInfo.IAPFlag = IAP_APP_UART;
		FirmwareInfo.FirmwareSize = Size;

		ptmp = strstr((char *)FileName, MODEM_VERSION_HEAD);
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

		//DelayUsTime(100);// Wirte Flag
		//Check  the written value
		if ((((FirmwareInfoTypeDef *)IAP_FLAG)->IAPFlag == (uint32_t)IAP_APP_UART))
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

		ymodem_deinit();
	}
	else
	{
		TickHardwareWatchdog();
		// Show Fail Programming
		Serial_PutString("ProgramWord Fail\r\n");
		if (Size == -1)
			Serial_PutString("\n\n\rThe image size is higher than the allowed space memory!\r\n");
		else if (Size == -2)
			Serial_PutString("\n\n\rReadWord failed!\r\n");
		else if (Size == -3)
			Serial_PutString("\n\n\rProgramWord failed!\r\n");
		else if (Size == -4)
			Serial_PutString("\n\n\rVerification failed!\r\n");
		else if (Size == -5)
			Serial_PutString("\r\n\nAborted by user.\r\n");
		else
			Serial_PutString("\n\rFailed to receive the file\r\n");
		Serial_PutString("\r\nWaiting for the file to be sent ... \r\npress 'a' to abort\r\n");

		ymodem_deinit();
	}
}

uint8_t InitEmmc(void)
{
	if (EmmcInit())
	{
		Serial_PutString("\r\nEmmc Init error!");
		return 1;
	}
	else
	{
		uint16_t waitCount = 0;
		while (EMMC_CARD_TRANSFER != EmmcGetState());
		{
			waitCount++;
			if (waitCount == 0xfff)
			{
				return 2;
			}
		}
	}

	return 0;
}

uint8_t FirmwareSizeCrc16Read(uint32_t address, FirmwareSizeCrc16TypeDef *pSizeCrc16)
{
	if (address < EMMC_RESERVE_MEMORY_START_ADDR)
	{
		Serial_PutString("FirmwareSizeCrc16Read param error!");
		return 1;
	}

	if (Emmc_ReadData((uint8_t *)pSizeCrc16, sizeof(FirmwareSizeCrc16TypeDef), address))
	{
		Serial_PutString("FirmwareInfoSet error!");
		return 2;
	}

	return 0;
}

uint8_t CheckFirmwareInfo(FirmwareInfoTypeDef *pFirmwareInfo)
{
	char *ptmp = pFirmwareInfo->Version;
	uint8_t ret = 0;
	uint32_t size = 0;
	FirmwareSizeCrc16TypeDef SizeCrc16 = {0};
	// if ((ptmp[2] == '.') && (ptmp[2 + 3] == '.') && (ptmp[2 + 3 + 3] == '.'))
	// {
		
	// }
	// else
	// {
	// 	ret = 1;
	// }

	size = pFirmwareInfo->FirmwareSize;
	if ((size > 0) && (size < USER_FLASH_SIZE))
	{

	}
	else
	{
		ret = 2;
	}

	if (0 != FirmwareSizeCrc16Read(EMMC_FIRMWARE_START_ADDR, &SizeCrc16))
	{
		ret = 3;
	}

	if ((SizeCrc16.crc16 != pFirmwareInfo->SizeCrc16.crc16) || (SizeCrc16.appsize != pFirmwareInfo->SizeCrc16.appsize))
	{
		ret = 4;
	}

	return ret;
}

uint8_t FirmwareNeedUpdate(FirmwareInfoTypeDef *pold, FirmwareInfoTypeDef *pnew)
{
	char *ptmpold = pold->Version;
	char *ptmpnew = pnew->Version;
	uint8_t i = 0, ret = 0;

	for (i = 0; i < FIRMWARE_VERSION_LEN_MAX; i++)
	{
		if (ptmpold[i] != 0xff)
			break;
	}

	if (i == FIRMWARE_VERSION_LEN_MAX)
		return 1;

	for (i = 0; (i < FIRMWARE_VERSION_LEN_MAX) && (*ptmpold != 0) && (*ptmpnew != 0); i++)
	{
		if (*ptmpold < *ptmpnew)
		{
			ret = 1;
			break;
		}
		ptmpold++;
		ptmpnew++;
	}

	return ret;
}

uint8_t CopyFirmwareFromEmmc(FirmwareInfoTypeDef *pFirmwareInfo)
{
	#define COPY_FIRMWARE_BUF_SIZE (1024)
	uint32_t FirmwareSize = pFirmwareInfo->SizeCrc16.appsize + 8;
	uint8_t buf[COPY_FIRMWARE_BUF_SIZE] = {0};
	uint32_t count = 0, left = 0, i = 0;
	uint32_t readaddr = 0, writeaddr = 0;
	FirmwareInfoTypeDef FirmwareInfo = {0};

	Serial_PutString("\r\nFirmware Update From Emmc!");

	__disable_irq();

	FLASH_If_Init();

	FLASH_BUSY_WAIT();
	__ISB();
	__DSB();
	__DMB();
	if (HAL_OK != FlashMemoryErasePage(JUMP_TO_APPLICATION_ADDRESS, USER_FLASH_END_ADDRESS - 1))
	{
		__enable_irq();
		Serial_PutString("\r\nFlashMemoryErase error!");
		return 1;
	}
	__ISB();
	__DSB();
	__DMB();
	__enable_irq();

	count = FirmwareSize / COPY_FIRMWARE_BUF_SIZE;
	left = FirmwareSize % COPY_FIRMWARE_BUF_SIZE;

	readaddr = EMMC_FIRMWARE_START_ADDR + pFirmwareInfo->FirmwareSize - pFirmwareInfo->SizeCrc16.appsize;
	writeaddr = JUMP_TO_APPLICATION_ADDRESS;
	for (i = 0; i < count; i++)
	{
		TickHardwareWatchdog();
		
		memset(buf, 0, sizeof(buf));

		if (Emmc_ReadData(buf, COPY_FIRMWARE_BUF_SIZE, readaddr))
		{
			Serial_PutString("\r\nFirmware read error!");
			return 2;
		}
		FLASH_BUSY_WAIT();
		if (FLASH_If_Write(&writeaddr, (uint64_t *)buf, COPY_FIRMWARE_BUF_SIZE / 8))
		{
			Serial_PutString("\r\nFirmware write error!");
			return 3;
		}

		readaddr += (COPY_FIRMWARE_BUF_SIZE);
	}

	if (left)
	{
		if (Emmc_ReadData(buf, left, readaddr))
		{
			Serial_PutString("\r\nFirmware left read error!");
			return 4;
		}
		FLASH_BUSY_WAIT();
		if (FLASH_If_Write(&writeaddr, (uint64_t *)buf, left / 8))
		{
			Serial_PutString("\r\nFirmware left write error!");
			return 5;
		}
	}

	writeaddr = IAP_FLAG;
	FLASH_BUSY_WAIT();
	if (HAL_OK != FlashMemoryErasePage(ADDR_FLASH_PAGE_15, ADDR_FLASH_PAGE_15 + FLASH_PAGE_SIZE - 1))
	{
		Serial_PutString("\r\nFlashMemoryErase flag error!");
		return 6;
	}

	memset(&FirmwareInfo, 0, sizeof(FirmwareInfo));
	FirmwareInfo.IAPFlag = IAP_APP_FLASH;
	FirmwareInfo.FirmwareSize = pFirmwareInfo->FirmwareSize;
	strcpy(FirmwareInfo.Version, pFirmwareInfo->Version);
	FirmwareInfo.SizeCrc16 = pFirmwareInfo->SizeCrc16;

	FLASH_BUSY_WAIT();
	if (FLASH_If_Write(&writeaddr, (uint64_t *)&FirmwareInfo, sizeof(FirmwareInfo) / 8))
	{
		Serial_PutString("\r\nFirmwareInfo write error!");
		return 7;
	}
	FLASH_BUSY_WAIT();

	return 0;
}

void UpdateFromEmmc(FirmwareInfoTypeDef *pFirmwareInfo)
{
	FirmwareInfoTypeDef FirmwareInfo = {0};

	TickHardwareWatchdog();

	if (pFirmwareInfo == NULL)
	{
		Serial_PutString("\r\nUpdateFromEmmc param error!");
		return;
	}

	if (InitEmmc())
	{
		Serial_PutString("\r\nInitEmmc error!");
		return;
	}

	if (Emmc_ReadData((uint8_t *)&FirmwareInfo, sizeof(FirmwareInfo), EMMC_FIRMWARE_INFO_START_ADDR))
	{
		Serial_PutString("\r\nFirmwareInfo read error!");
		return;
	}

	if (CheckFirmwareInfo(&FirmwareInfo))
	{
		Serial_PutString("\r\nFirmwareInfo error!");
		return;
	}

	// if (FirmwareNeedUpdate(pFirmwareInfo, &FirmwareInfo) == 0)
	// {
	// 	Serial_PutString("\r\nFirmware version uptodate");
	// }
	// else
	{
		if (CopyFirmwareFromEmmc(&FirmwareInfo))
		{
			Serial_PutString("\r\nFirmware copy err");
			return;
		}
	}

	HAL_FLASH_Lock();
	/* Clear all FLASH flags */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_ALL_ERRORS);

	Serial_PutString("\r\n>>Jump to application...");
	Serial_PutString("\r\n\r\n\r\n");
	HAL_Delay(10);
	
	TickHardwareWatchdog();
	// Jump to user application
	SoftwareJumping(JUMP_TO_APPLICATION_ADDRESS);
}

// For Flash read and write and erase test
// #include "flash.h"
// __IO uint32_t FlashAddress = 0, i = 0;
// uint8_t Data[4096] = {0};
// int main(void)
// {
//   HAL_Init();

//   /* Configure the system clock to 80 MHz */
//   SystemClock_Config();

//   FLASH_If_Init();

//   FlashMemoryErasePage(JUMP_TO_APPLICATION_ADDRESS, USER_FLASH_END_ADDRESS - 1);

//   for (i = 0; i < 4096; i++)
//   {
//     Data[i] = i % 8;
//   }

//   FlashAddress = (0x0803F800);
//   if (FLASH_If_Write(&FlashAddress, (uint64_t *)Data, 4096 / 8))
//   {
//     while (1);
//   }

//   while (1);
// }

// #include "SdioEmmcDrive.h"
// uint8_t emmcWData[1024];
// uint8_t emmcRData[1024];
// int main(void)
// {
//   uint32_t i = 0, err = 0;

//   /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//   HAL_Init();
//   /* Configure the system clock */
//   SystemClock_Config();

//   /* Initialize all configured peripherals */
//   MX_GPIO_Init();
//   MX_DMA_Init();
//   MX_UART4_Init();

//   if (InitEmmc())
// 	{
// 		return 1;
// 	}

//   for (i = 0; i < 1024; i++)
//   {
//     emmcWData[i] = i % 16;
//   }
//   err = Emmc_WriteData(emmcWData, 127, EMMC_FIRMWARE_INFO_START_ADDR);
//   err = Emmc_ReadData(emmcRData, 788, EMMC_FIRMWARE_INFO_START_ADDR - 8);
//   return 0;
// }