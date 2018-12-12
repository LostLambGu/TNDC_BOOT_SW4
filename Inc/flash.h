#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32l4xx_hal.h"

#define MCU_FLASH_START_ADDR (FLASH_BASE)
#define JUMP_TO_BOOTLOADER_ADDRESS (MCU_FLASH_START_ADDR)
#define JUMP_TO_APPLICATION_ADDRESS (0x08008000)

#define IAP_BOOT_FLASH (uint32_t)(0xAA) // 170
#define IAP_BOOT_UART (uint32_t)(0xBB) // 187
#define IAP_BOOT_CHMODEM (uint32_t)(0x5C) // 92
#define IAP_APP_FLASH (uint32_t)(0xCC)  // 204
#define IAP_APP_UART (uint32_t)(0xDD)  // 221
#define IAP_APP_CHMODEM (uint32_t)(0xEE)  // 238
#define IAP_RESET (uint32_t)(0xFF)     // 255
#define IAP_FLAG (uint32_t)(JUMP_TO_APPLICATION_ADDRESS - 0x400)

#define ADDR_FLASH_PAGE_15 (15 * FLASH_PAGE_SIZE + MCU_FLASH_START_ADDR)
#define ADDR_FLASH_PAGE_16 (16 * FLASH_PAGE_SIZE + MCU_FLASH_START_ADDR)
#define FLASH_USER_END_ADDR (256 * FLASH_PAGE_SIZE + MCU_FLASH_START_ADDR)

/* Get the number of Sector from where the user program will be loaded */
#define FLASH_PAGE_NUMBER (uint32_t)((JUMP_TO_APPLICATION_ADDRESS - JUMP_TO_BOOTLOADER_ADDRESS) >> 12)

/* Compute the mask to test if the Flash memory, where the user program will be
   loaded, is write protected */
#define FLASH_PROTECTED_PAGES ((uint32_t) ~((1 << FLASH_PAGE_NUMBER) - 1))

#define USER_FLASH_END_ADDRESS FLASH_USER_END_ADDR

/* define the user application size */
#define USER_FLASH_SIZE (USER_FLASH_END_ADDRESS - JUMP_TO_APPLICATION_ADDRESS + 1)

#define FIRMWARE_VERSION_LEN_MAX (32)
#define FIRMWARE_CRC16_CAL_BUF_SIZE (512)
typedef struct FirmwareSizeCrc16
{
	uint32_t appsize;
	uint32_t crc16;
} FirmwareSizeCrc16TypeDef;
typedef struct FirmwareInfo
{
	uint32_t IAPFlag;
	uint32_t FirmwareSize;
	char Version[FIRMWARE_VERSION_LEN_MAX];
	FirmwareSizeCrc16TypeDef SizeCrc16;
} FirmwareInfoTypeDef;
#define MODEM_VERSION_HEAD "VER_" // Filename format: VER_00.00.00.000
#define APP_DEFAULT_VERSION "00.00.00.000"

extern void FLASH_If_Init(void);
extern void FLASH_BUSY_WAIT(void);
// extern uint32_t FLASH_If_GetWriteProtectionStatus(void);
extern HAL_StatusTypeDef FlashMemoryErasePage(uint32_t StartAddr, uint32_t EndAddr);
extern uint32_t FLASH_If_Write(__IO uint32_t *FlashAddress, uint64_t *Data, uint16_t DataLength);
extern void SerialDownload(void);

extern uint8_t CheckFirmwareInfo(FirmwareInfoTypeDef *pFirmwareInfo);
extern void UpdateFromEmmc(FirmwareInfoTypeDef *pFirmwareInfo);

#endif
