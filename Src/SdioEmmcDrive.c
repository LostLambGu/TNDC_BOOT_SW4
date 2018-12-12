/*******************************************************************************
* File Name          : SdioEmmcDrive.c
* Author             : Yangjie Gu
* Description        : This file provides all the SdioEmmcDrive functions.

* History:
*  02/27/2018 : SdioEmmcDrive V1.00
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "SdioEmmcDrive.h"
#include "sdmmc.h"
#include "gpio.h"
#include "dma.h"

#include "string.h"

#ifndef EMMC_NO_DMA_SUPPORT_OS
#define EMMC_NO_DMA_SUPPORT_OS (0)
#endif /* EMMC_NO_DMA_SUPPORT_OS */

#if EMMC_NO_DMA_SUPPORT_OS
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#endif /* EMMC_NO_DMA_SUPPORT_OS */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SDMMC_TypeDef *pSDMMC1 = SDMMC1;
#define SDMMC_DIRECTION_READ (1)
#define SDMMC_DIRECTION_WRITE (2)
static __IO uint8_t sdmmcDirect = 0;
DMA_HandleTypeDef *pHdma_sdmmc1_rx = &hdma_sdmmc1;
DMA_HandleTypeDef *pHdma_sdmmc1_tx = &hdma_sdmmc1;
SDMMC_InitTypeDef SDMMC_InitStructure;
SDMMC_CmdInitTypeDef SDMMC_CmdInitStructure = {0};
SDMMC_DataInitTypeDef SDMMC_DataInitStructure = {0};
EmmcCardInfo MyEmmcCardInfo;
// static uint32_t CardType =  SDMMC_MULTIMEDIA_CARD;

uint32_t EmmcInitializeCards(EmmcCardInfo *E);

#define EMMC_POWER_EN_PORT PA3_eMMC_PWR_EN_GPIO_Port
#define EMMC_POWER_EN_PIN PA3_eMMC_PWR_EN_Pin
#define EMMC_RESET_PORT PA8_eMMC_RST_GPIO_Port
#define EMMC_RESET_PIN PA8_eMMC_RST_Pin

void EmmcChipEnControl(FunctionalState State)
{
  if (State == ENABLE)
  {
    HAL_GPIO_WritePin(EMMC_RESET_PORT, EMMC_RESET_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EMMC_POWER_EN_PORT, EMMC_POWER_EN_PIN, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(EMMC_RESET_PORT, EMMC_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
  }
  else
  {
    HAL_GPIO_WritePin(EMMC_POWER_EN_PORT, EMMC_POWER_EN_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(EMMC_RESET_PORT, EMMC_RESET_PIN, GPIO_PIN_RESET);
  }
  
}

/**
  * @brief  Initializes the EMMC Card and put it into StandBy State (Ready for data 
  *         transfer).
  * @param  None
  * @retval EmmcError: EMMC Card Error code.
  */
uint32_t EmmcInit(void)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;

  SDMMC1_DeInit();
  EmmcChipEnControl(DISABLE);

  if (SdmmcPowerON() != HAL_OK)
  {
    return SDMMC_ERROR_INVALID_VOLTRANGE;
  }

  errorstate = EmmcGoReadyState();
  if (errorstate != SDMMC_ERROR_NONE)
  {
    return errorstate;
  }

  errorstate = EmmcInitializeCards(&MyEmmcCardInfo);
  if (errorstate != SDMMC_ERROR_NONE)
  {
    return (errorstate);
  }

  errorstate = EmmcEnableWideBusOperation(SDMMC_BUS_WIDE_8B);
  if (errorstate != SDMMC_ERROR_NONE)
  {
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return (errorstate);
  }

  // while (EMMC_CARD_TRANSFER != EmmcGetState());

  return (errorstate);
}

/**
  * @brief  Enquires cards about their operating voltage and configures 
  *   clock controls.
  * @param  None
  * @retval EmmcError: EMMC Card Error code.
  */
HAL_StatusTypeDef SdmmcPowerON(void)
{
  /*!< Power ON Sequence -----------------------------------------------------*/
  /*!< Configure the SDMMC peripheral */
  /*!< SDMMC_CK = SDMMCCLK / (SDMMC_INIT_CLK_DIV + 2) */
  /*!< SDMMC_CK for initialization should not exceed 400 KHz */
  HAL_StatusTypeDef errorstate = HAL_OK;

  // HAL_Delay(2U);
  SDMMC1_MspInit();

  EmmcChipEnControl(ENABLE);

  /* Default SDMMC peripheral configuration for SD card initialization */
  SDMMC_InitStructure.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  SDMMC_InitStructure.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  SDMMC_InitStructure.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  SDMMC_InitStructure.BusWide = SDMMC_BUS_WIDE_1B;
  SDMMC_InitStructure.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  SDMMC_InitStructure.ClockDiv = 158;

  /* Initialize SDMMC peripheral interface with default configuration */
  // HAL_Delay(2U);
  errorstate = SDMMC_Init(pSDMMC1, SDMMC_InitStructure);
  if (errorstate != HAL_OK)
  {
    return errorstate;
  }

  /* Disable SDMMC Clock */
  // HAL_Delay(2U);
  __SDMMC_DISABLE(pSDMMC1);

  /* Set Power State to ON */
  HAL_Delay(2U);
  errorstate = SDMMC_PowerState_ON(pSDMMC1);
  if (errorstate != HAL_OK)
  {
    return errorstate;
  }

  /* Enable SDMMC Clock */
  __SDMMC_ENABLE(pSDMMC1);

  return errorstate;
}

uint32_t EmmcGoReadyState(void)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;
  uint32_t Cnt = 0;
  uint32_t Rev = 0;

  /* Required power up waiting time before starting the EMMC initialization sequence */
  // HAL_Delay(2U);

  /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
  /*!< No CMD response required */
  errorstate = SDMMC_CmdGoIdleState(pSDMMC1);
  if (errorstate != SDMMC_ERROR_NONE)
  {
    return errorstate;
  }

  // HAL_Delay(2U);
  // CardType = SDMMC_HIGH_CAPACITY_MMC_CARD;
  do
  {
    // HAL_Delay(5);
    /*!< SEND CMD1*/
    errorstate = SDMMC_CmdOpCondition(pSDMMC1, 0xC0FF8000U);
    Rev = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP1);
    Cnt++;
  } while ((Cnt < SDMMC_MAX_VOLT_TRIAL) && ((0 == (Rev & 0x80000000))));

  // HAL_Delay(2U);
  // /*!< SEND CMD1*/
  // errorstate = SDMMC_CmdOpCondition(pSDMMC1, EMMC_OCR_REG);

  if (Cnt == SDMMC_MAX_VOLT_TRIAL)
  {
    return SDMMC_ERROR_UNSUPPORTED_FEATURE;
  }

  return errorstate;
}

/**
  * @brief  Turns the SDMMC output signals off.
  * @param  None
  * @retval EmmcError: EMMC Card Error code.
  */
uint32_t EmmcPowerOFF(void)
{
  uint32_t errorstatus = SDMMC_ERROR_NONE;

  /*!< Set Power State to OFF */
  errorstatus = SDMMC_PowerState_OFF(pSDMMC1);

  return (errorstatus);
}

/**
  * @brief  Intialises all cards or single card as the case may be Card(s) come 
  *         into standby state.
  * @param  None
  * @retval EmmcError: EMMC Card Error code.
  */
uint32_t EmmcInitializeCards(EmmcCardInfo *E)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;
  uint32_t CSD_Tab[4] = {0};
  uint32_t CID_Tab[4] = {0};
  uint16_t Rca = 0;

  if (SDMMC_GetPowerState(pSDMMC1) == 0)
  {
    /* Power off */
    return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
  }
  /*!< Send CMD2 ALL_SEND_CID */
  errorstate = SDMMC_CmdSendCID(pSDMMC1);
  if (errorstate != SDMMC_ERROR_NONE)
  {
    return errorstate;
  }

  CID_Tab[0] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP1);
  CID_Tab[1] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP2);
  CID_Tab[2] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP3);
  CID_Tab[3] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP4);

  /*!< Send CMD3 SET_REL_ADDR with argument 0 */
  /*!< EMMC Card publishes its RCA. */
  errorstate = SDMMC_CmdSetRelAdd(pSDMMC1, &Rca);
  if (errorstate != SDMMC_ERROR_NONE)
  {
    return errorstate;
  }

  E->RCA = Rca;

  /*!< Send CMD9 SEND_CSD with argument as card's RCA */
  errorstate = SDMMC_CmdSendCSD(pSDMMC1, (uint32_t)(Rca << 16U));
  if (errorstate != SDMMC_ERROR_NONE)
  {
    return errorstate;
  }

  CSD_Tab[0] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP1);
  CSD_Tab[1] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP2);
  CSD_Tab[2] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP3);
  CSD_Tab[3] = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP4);

  errorstate = EmmcSelectDeselect((uint32_t)(Rca << 16));
  if (SDMMC_ERROR_NONE != errorstate)
  {
    return errorstate;
  }

  // errorstate = EmmcReadExtCsd(&MyEmmcCardInfo);
  // if (EmmcReadExtCsd(&MyEmmcCardInfo) != EMMC_OK)
  // {
  //   return (SDMMC_ERROR_CID_CSD_OVERWRITE);
  // }

  errorstate = EMMC_GetCardCID(CID_Tab, &(E->EmmcCid));
  if (SDMMC_ERROR_NONE != errorstate)
  {
    return (errorstate);
  }

  errorstate = EMMC_GetCardCSD(CSD_Tab, &(E->EmmcCsd));
  if (SDMMC_ERROR_NONE != errorstate)
  {
    return (errorstate);
  }

  errorstate = SDMMC_ERROR_NONE; /*!< All cards get intialized */
  return (errorstate);
}

/**
  * @brief  Returns information the information of the card which are stored on
  *         the CID register.
  * @param  hmmc: Pointer to MMC handle
  * @param  pCID: Pointer to a EMMC_CardCIDTypeDef structure that  
  *         contains all CID register parameters 
  * @retval HAL status
  */
uint32_t EMMC_GetCardCID(uint32_t *CID_Tab, EMMC_CardCIDTypeDef *pCID)
{
  uint32_t tmp = 0U;
  
  /* Byte 0 */
  tmp = (uint8_t)((CID_Tab[0U] & 0xFF000000U) >> 24U);
  pCID->ManufacturerID = tmp;
  
  /* Byte 1 */
  tmp = (uint8_t)((CID_Tab[0U] & 0x00FF0000U) >> 16U);
  pCID->OEM_AppliID = tmp << 8U;
  
  /* Byte 2 */
  tmp = (uint8_t)((CID_Tab[0U] & 0x000000FF00U) >> 8U);
  pCID->OEM_AppliID |= tmp;
  
  /* Byte 3 */
  tmp = (uint8_t)(CID_Tab[0U] & 0x000000FFU);
  pCID->ProdName1 = tmp << 24U;
  
  /* Byte 4 */
  tmp = (uint8_t)((CID_Tab[1U] & 0xFF000000U) >> 24U);
  pCID->ProdName1 |= tmp << 16U;
  
  /* Byte 5 */
  tmp = (uint8_t)((CID_Tab[1U] & 0x00FF0000U) >> 16U);
  pCID->ProdName1 |= tmp << 8U;
  
  /* Byte 6 */
  tmp = (uint8_t)((CID_Tab[1U] & 0x0000FF00U) >> 8U);
  pCID->ProdName1 |= tmp;
  
  /* Byte 7 */
  tmp = (uint8_t)(CID_Tab[1U] & 0x000000FFU);
  pCID->ProdName2 = tmp;
  
  /* Byte 8 */
  tmp = (uint8_t)((CID_Tab[2U] & 0xFF000000U) >> 24U);
  pCID->ProdRev = tmp;
  
  /* Byte 9 */
  tmp = (uint8_t)((CID_Tab[2U] & 0x00FF0000U) >> 16U);
  pCID->ProdSN = tmp << 24U;
  
  /* Byte 10 */
  tmp = (uint8_t)((CID_Tab[2U] & 0x0000FF00U) >> 8U);
  pCID->ProdSN |= tmp << 16U;
  
  /* Byte 11 */
  tmp = (uint8_t)(CID_Tab[2U] & 0x000000FFU);
  pCID->ProdSN |= tmp << 8U;
  
  /* Byte 12 */
  tmp = (uint8_t)((CID_Tab[3U] & 0xFF000000U) >> 24U);
  pCID->ProdSN |= tmp;
  
  /* Byte 13 */
  tmp = (uint8_t)((CID_Tab[3U] & 0x00FF0000U) >> 16U);
  pCID->Reserved1   |= (tmp & 0xF0U) >> 4U;
  pCID->ManufactDate = (tmp & 0x0FU) << 8U;
  
  /* Byte 14 */
  tmp = (uint8_t)((CID_Tab[3U] & 0x0000FF00U) >> 8U);
  pCID->ManufactDate |= tmp;
  
  /* Byte 15 */
  tmp = (uint8_t)(CID_Tab[3U] & 0x000000FFU);
  pCID->CID_CRC   = (tmp & 0xFEU) >> 1U;
  pCID->Reserved2 = 1U;

  return SDMMC_ERROR_NONE;
}

/**
  * @brief  Returns information the information of the card which are stored on
  *         the CSD register.
  * @param  pCSD: Pointer to a EMMC_CardCSDTypeDef structure that  
  *         contains all CSD register parameters  
  * @retval HAL status
  */
uint32_t EMMC_GetCardCSD(uint32_t *CSD_Tab, EMMC_CardCSDTypeDef *pCSD)
{
  uint32_t tmp = 0U;
  
  /* Byte 0 */
  tmp = (CSD_Tab[0U] & 0xFF000000U) >> 24U;
  pCSD->CSDStruct      = (uint8_t)((tmp & 0xC0U) >> 6U);
  pCSD->SysSpecVersion = (uint8_t)((tmp & 0x3CU) >> 2U);
  pCSD->Reserved1      = tmp & 0x03U;
  
  /* Byte 1 */
  tmp = (CSD_Tab[0U] & 0x00FF0000U) >> 16U;
  pCSD->TAAC = (uint8_t)tmp;
  
  /* Byte 2 */
  tmp = (CSD_Tab[0U] & 0x0000FF00U) >> 8U;
  pCSD->NSAC = (uint8_t)tmp;
  
  /* Byte 3 */
  tmp = CSD_Tab[0U] & 0x000000FFU;
  pCSD->MaxBusClkFrec = (uint8_t)tmp;
  
  /* Byte 4 */
  tmp = (CSD_Tab[1U] & 0xFF000000U) >> 24U;
  pCSD->CardComdClasses = (uint16_t)(tmp << 4U);
  
  /* Byte 5 */
  tmp = (CSD_Tab[1U] & 0x00FF0000U) >> 16U;
  pCSD->CardComdClasses |= (uint16_t)((tmp & 0xF0U) >> 4U);
  pCSD->RdBlockLen       = (uint8_t)(tmp & 0x0FU);
  
  /* Byte 6 */
  tmp = (CSD_Tab[1U] & 0x0000FF00U) >> 8U;
  pCSD->PartBlockRead   = (uint8_t)((tmp & 0x80U) >> 7U);
  pCSD->WrBlockMisalign = (uint8_t)((tmp & 0x40U) >> 6U);
  pCSD->RdBlockMisalign = (uint8_t)((tmp & 0x20U) >> 5U);
  pCSD->DSRImpl         = (uint8_t)((tmp & 0x10U) >> 4U);
  pCSD->Reserved2       = 0; /*!< Reserved */
       
  pCSD->DeviceSize = (tmp & 0x03U) << 10U;
  
  /* Byte 7 */
  tmp = (uint8_t)(CSD_Tab[1U] & 0x000000FFU);
  pCSD->DeviceSize |= (tmp) << 2U;
  
  /* Byte 8 */
  tmp = (uint8_t)((CSD_Tab[2U] & 0xFF000000U) >> 24U);
  pCSD->DeviceSize |= (tmp & 0xC0U) >> 6U;
  
  pCSD->MaxRdCurrentVDDMin = (tmp & 0x38U) >> 3U;
  pCSD->MaxRdCurrentVDDMax = (tmp & 0x07U);
  
  /* Byte 9 */
  tmp = (uint8_t)((CSD_Tab[2U] & 0x00FF0000U) >> 16U);
  pCSD->MaxWrCurrentVDDMin = (tmp & 0xE0U) >> 5U;
  pCSD->MaxWrCurrentVDDMax = (tmp & 0x1CU) >> 2U;
  pCSD->DeviceSizeMul      = (tmp & 0x03U) << 1U;
  /* Byte 10 */
  tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00U) >> 8U);
  pCSD->DeviceSizeMul |= (tmp & 0x80U) >> 7U;
  
  // hmmc->MmcCard.BlockNbr  = (pCSD->DeviceSize + 1U) ;
  // hmmc->MmcCard.BlockNbr *= (1U << (pCSD->DeviceSizeMul + 2U));
  // hmmc->MmcCard.BlockSize = 1U << (pCSD->RdBlockLen);
  
  // hmmc->MmcCard.LogBlockNbr =  (hmmc->MmcCard.BlockNbr) * ((hmmc->MmcCard.BlockSize) / 512U); 
  // hmmc->MmcCard.LogBlockSize = 512U;
  
  pCSD->EraseGrSize = (tmp & 0x40U) >> 6U;
  pCSD->EraseGrMul  = (tmp & 0x3FU) << 1U;
  
  /* Byte 11 */
  tmp = (uint8_t)(CSD_Tab[2U] & 0x000000FFU);
  pCSD->EraseGrMul     |= (tmp & 0x80U) >> 7U;
  pCSD->WrProtectGrSize = (tmp & 0x7FU);
  
  /* Byte 12 */
  tmp = (uint8_t)((CSD_Tab[3U] & 0xFF000000U) >> 24U);
  pCSD->WrProtectGrEnable = (tmp & 0x80U) >> 7U;
  pCSD->ManDeflECC        = (tmp & 0x60U) >> 5U;
  pCSD->WrSpeedFact       = (tmp & 0x1CU) >> 2U;
  pCSD->MaxWrBlockLen     = (tmp & 0x03U) << 2U;
  
  /* Byte 13 */
  tmp = (uint8_t)((CSD_Tab[3U] & 0x00FF0000U) >> 16U);
  pCSD->MaxWrBlockLen      |= (tmp & 0xC0U) >> 6U;
  pCSD->WriteBlockPaPartial = (tmp & 0x20U) >> 5U;
  pCSD->Reserved3           = 0U;
  pCSD->ContentProtectAppli = (tmp & 0x01U);
  
  /* Byte 14 */
  tmp = (uint8_t)((CSD_Tab[3U] & 0x0000FF00U) >> 8U);
  pCSD->FileFormatGrouop = (tmp & 0x80U) >> 7U;
  pCSD->CopyFlag         = (tmp & 0x40U) >> 6U;
  pCSD->PermWrProtect    = (tmp & 0x20U) >> 5U;
  pCSD->TempWrProtect    = (tmp & 0x10U) >> 4U;
  pCSD->FileFormat       = (tmp & 0x0CU) >> 2U;
  pCSD->ECC              = (tmp & 0x03U);
  
  /* Byte 15 */
  tmp = (uint8_t)(CSD_Tab[3U] & 0x000000FFU);
  pCSD->CSD_CRC   = (tmp & 0xFEU) >> 1U;
  pCSD->Reserved4 = 1U;
  
  return SDMMC_ERROR_NONE;
}

// /**
//   * @brief  Returns information about specific card.
//   * @param  E: pointer to a EmmcCardInfo structure that contains all EMMC card 
//   *         information.
//   * @retval EmmcError: EMMC Card Error code.
//   */
// uint32_t EmmcGetCardInfo(EmmcCardInfo *E, uint32_t *CSD_Tab, uint32_t *CID_Tab, uint16_t Rca)
// {
	
// 		uint32_t errorstatus = SDMMC_ERROR_NONE;
// 		uint8_t tmp = 0;

// 		E->CardType = (uint8_t)CardType;
// 		E->RCA = (uint16_t)Rca;

// 		/*!< Byte 0 */
// 		tmp = (uint8_t)((CSD_Tab[0] & 0xFF000000) >> 24);
// 		E->EmmcCsd.CSDStruct = (tmp & 0xC0) >> 6;
// 		E->EmmcCsd.SysSpecVersion = (tmp & 0x3C) >> 2;
// 		E->EmmcCsd.Reserved1 = tmp & 0x03;

// 		/*!< Byte 1 */
// 		tmp = (uint8_t)((CSD_Tab[0] & 0x00FF0000) >> 16);
// 		E->EmmcCsd.TAAC = tmp;

// 		/*!< Byte 2 */
// 		tmp = (uint8_t)((CSD_Tab[0] & 0x0000FF00) >> 8);
// 		E->EmmcCsd.NSAC = tmp;

// 		/*!< Byte 3 */
// 		tmp = (uint8_t)(CSD_Tab[0] & 0x000000FF);
// 		E->EmmcCsd.MaxBusClkFrec = tmp;

// 		/*!< Byte 4 */
// 		tmp = (uint8_t)((CSD_Tab[1] & 0xFF000000) >> 24);
// 		E->EmmcCsd.CardComdClasses = tmp << 4;

// 		/*!< Byte 5 */
// 		tmp = (uint8_t)((CSD_Tab[1] & 0x00FF0000) >> 16);
// 		E->EmmcCsd.CardComdClasses |= (tmp & 0xF0) >> 4;
// 		E->EmmcCsd.RdBlockLen = tmp & 0x0F;

// 		/*!< Byte 6 */
// 		tmp = (uint8_t)((CSD_Tab[1] & 0x0000FF00) >> 8);
// 		E->EmmcCsd.PartBlockRead = (tmp & 0x80) >> 7;
// 		E->EmmcCsd.WrBlockMisalign = (tmp & 0x40) >> 6;
// 		E->EmmcCsd.RdBlockMisalign = (tmp & 0x20) >> 5;
// 		E->EmmcCsd.DSRImpl = (tmp & 0x10) >> 4;
// 		E->EmmcCsd.Reserved2 = 0; /*!< Reserved */


// 		E->EmmcCsd.DeviceSize = (tmp & 0x03) << 10;

// 		/*!< Byte 7 */
// 		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
// 		E->EmmcCsd.DeviceSize |= (tmp) << 2;

// 		/*!< Byte 8 */
// 		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
// 		E->EmmcCsd.DeviceSize |= (tmp & 0xC0) >> 6;

// 		E->EmmcCsd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
// 		E->EmmcCsd.MaxRdCurrentVDDMax = (tmp & 0x07);

// 		/*!< Byte 9 */
// 		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
// 		E->EmmcCsd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
// 		E->EmmcCsd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
// 		E->EmmcCsd.DeviceSizeMul = (tmp & 0x03) << 1;
// 		/*!< Byte 10 */
// 		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
// 		E->EmmcCsd.DeviceSizeMul |= (tmp & 0x80) >> 7;


// 		E->CardBlockSize = 1 << (E->EmmcCsd.RdBlockLen);


// 		E->CardCapacity = (uint64_t)((uint64_t)((E->EmmcExtCsd.EXT_CSD.SEC_COUNT[3] << 24) | (E->EmmcExtCsd.EXT_CSD.SEC_COUNT[2] << 16) | 
// 		(E->EmmcExtCsd.EXT_CSD.SEC_COUNT[1] << 8) | (E->EmmcExtCsd.EXT_CSD.SEC_COUNT[0])) * E->CardBlockSize);


// 		E->EmmcCsd.EraseGrSize = (tmp & 0x40) >> 6;
// 		E->EmmcCsd.EraseGrMul = (tmp & 0x3F) << 1;

// 		/*!< Byte 11 */
// 		tmp = (uint8_t)(CSD_Tab[2] & 0x000000FF);
// 		E->EmmcCsd.EraseGrMul |= (tmp & 0x80) >> 7;
// 		E->EmmcCsd.WrProtectGrSize = (tmp & 0x7F);

// 		/*!< Byte 12 */
// 		tmp = (uint8_t)((CSD_Tab[3] & 0xFF000000) >> 24);
// 		E->EmmcCsd.WrProtectGrEnable = (tmp & 0x80) >> 7;
// 		E->EmmcCsd.ManDeflECC = (tmp & 0x60) >> 5;
// 		E->EmmcCsd.WrSpeedFact = (tmp & 0x1C) >> 2;
// 		E->EmmcCsd.MaxWrBlockLen = (tmp & 0x03) << 2;

// 		/*!< Byte 13 */
// 		tmp = (uint8_t)((CSD_Tab[3] & 0x00FF0000) >> 16);
// 		E->EmmcCsd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
// 		E->EmmcCsd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
// 		E->EmmcCsd.Reserved3 = 0;
// 		E->EmmcCsd.ContentProtectAppli = (tmp & 0x01);

// 		/*!< Byte 14 */
// 		tmp = (uint8_t)((CSD_Tab[3] & 0x0000FF00) >> 8);
// 		E->EmmcCsd.FileFormatGrouop = (tmp & 0x80) >> 7;
// 		E->EmmcCsd.CopyFlag = (tmp & 0x40) >> 6;
// 		E->EmmcCsd.PermWrProtect = (tmp & 0x20) >> 5;
// 		E->EmmcCsd.TempWrProtect = (tmp & 0x10) >> 4;
// 		E->EmmcCsd.FileFormat = (tmp & 0x0C) >> 2;
// 		E->EmmcCsd.ECC = (tmp & 0x03);

// 		/*!< Byte 15 */
// 		tmp = (uint8_t)(CSD_Tab[3] & 0x000000FF);
// 		E->EmmcCsd.CSD_CRC = (tmp & 0xFE) >> 1;
// 		E->EmmcCsd.Reserved4 = 1;


// 		/*!< Byte 0 */
// 		tmp = (uint8_t)((CID_Tab[0] & 0xFF000000) >> 24);
// 		E->EmmcCid.ManufacturerID = tmp;

// 		/*!< Byte 1 */
// 		tmp = (uint8_t)((CID_Tab[0] & 0x00FF0000) >> 16);
// 		E->EmmcCid.OEM_AppliID = tmp << 8;

// 		/*!< Byte 2 */
// 		tmp = (uint8_t)((CID_Tab[0] & 0x000000FF00) >> 8);
// 		E->EmmcCid.OEM_AppliID |= tmp;

// 		/*!< Byte 3 */
// 		tmp = (uint8_t)(CID_Tab[0] & 0x000000FF);
// 		E->EmmcCid.ProdName1 = tmp << 24;

// 		/*!< Byte 4 */
// 		tmp = (uint8_t)((CID_Tab[1] & 0xFF000000) >> 24);
// 		E->EmmcCid.ProdName1 |= tmp << 16;

// 		/*!< Byte 5 */
// 		tmp = (uint8_t)((CID_Tab[1] & 0x00FF0000) >> 16);
// 		E->EmmcCid.ProdName1 |= tmp << 8;

// 		/*!< Byte 6 */
// 		tmp = (uint8_t)((CID_Tab[1] & 0x0000FF00) >> 8);
// 		E->EmmcCid.ProdName1 |= tmp;

// 		/*!< Byte 7 */
// 		tmp = (uint8_t)(CID_Tab[1] & 0x000000FF);
// 		E->EmmcCid.ProdName2 = tmp;

// 		/*!< Byte 8 */
// 		tmp = (uint8_t)((CID_Tab[2] & 0xFF000000) >> 24);
// 		E->EmmcCid.ProdRev = tmp;

// 		/*!< Byte 9 */
// 		tmp = (uint8_t)((CID_Tab[2] & 0x00FF0000) >> 16);
// 		E->EmmcCid.ProdSN = tmp << 24;

// 		/*!< Byte 10 */
// 		tmp = (uint8_t)((CID_Tab[2] & 0x0000FF00) >> 8);
// 		E->EmmcCid.ProdSN |= tmp << 16;

// 		/*!< Byte 11 */
// 		tmp = (uint8_t)(CID_Tab[2] & 0x000000FF);
// 		E->EmmcCid.ProdSN |= tmp << 8;

// 		/*!< Byte 12 */
// 		tmp = (uint8_t)((CID_Tab[3] & 0xFF000000) >> 24);
// 		E->EmmcCid.ProdSN |= tmp;

// 		/*!< Byte 13 */
// 		tmp = (uint8_t)((CID_Tab[3] & 0x00FF0000) >> 16);
// 		E->EmmcCid.Reserved1 |= (tmp & 0xF0) >> 4;
// 		E->EmmcCid.ManufactDate = (tmp & 0x0F) << 8;

// 		/*!< Byte 14 */
// 		tmp = (uint8_t)((CID_Tab[3] & 0x0000FF00) >> 8);
// 		E->EmmcCid.ManufactDate |= tmp;

// 		/*!< Byte 15 */
// 		tmp = (uint8_t)(CID_Tab[3] & 0x000000FF);
// 		E->EmmcCid.CID_CRC = (tmp & 0xFE) >> 1;
// 		E->EmmcCid.Reserved2 = 1;
  
//   return(errorstatus);
// }

// /**
//   * @brief  Enables or disables the SDMMC wide bus mode.
//   * @param  NewState: new state of the SDMMC wide bus mode.
//   *   This parameter can be: ENABLE or DISABLE.
//   * @retval EmmcError: EMMC Card Error code.
//   */
// uint32_t EmmcEnWideBus(FunctionalState NewState)
// {
//   uint32_t errorstate = SDMMC_ERROR_NONE;
  
//   if((SDMMC_GetResponse(pSDMMC1, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
//   {
//     return SDMMC_ERROR_LOCK_UNLOCK_FAILED;
//   }

//   /*!< If wide bus operation to be enabled */
//   if (NewState == ENABLE)
//   {
//     /* CMD6 */
//     // errorstate = SDMMC_CmdSwitch(pSDMMC1, EMMC_POWER_REG);
//     // if (errorstate != SDMMC_ERROR_NONE)
//     // {
//     //   return errorstate;
//     // }

//     /* CMD6 */
//     // errorstate = SDMMC_CmdSwitch(pSDMMC1, EMMC_HIGHSPEED_REG);
//     // if (errorstate != SDMMC_ERROR_NONE)
//     // {
//     //   return errorstate;
//     // }

//     /* CMD6 */
//     errorstate = SDMMC_CmdSwitch(pSDMMC1, EMMC_4BIT_REG);
//     if (errorstate != SDMMC_ERROR_NONE)
//     {
//       return errorstate;
//     }

//     return (errorstate);
//   }   /*!< If wide bus operation to be disabled */
//   else
//   {
//     /* Not implemented */
//     return SDMMC_ERROR_NONE;
//   }
// }

/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by 
  *         card.
  * @param  WideMode: Specifies the EMMC card wide bus mode. 
  *   This parameter can be one of the following values:
  *     @arg SDMMC_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDMMC_BusWide_4b: 4-bit data transfer
  *     @arg SDMMC_BusWide_1b: 1-bit data transfer
  * @retval EmmcError: EMMC Card Error code.
  */

uint32_t EmmcEnableWideBusOperation(uint32_t WideMode)
{
  __IO uint32_t count = 0U;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  uint32_t response = 0U, busy = 0U;
  
  /* Update Clock for Bus mode update */
  SDMMC_InitStructure.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  SDMMC_InitStructure.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  SDMMC_InitStructure.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  SDMMC_InitStructure.BusWide = WideMode;
  SDMMC_InitStructure.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  SDMMC_InitStructure.ClockDiv = 158;

  /* Initialize SDMMC peripheral interface with default configuration */
  errorstate = SDMMC_Init(pSDMMC1, SDMMC_InitStructure);
  if (errorstate != HAL_OK)
  {
    return EMMC_SDIO_FUNCTION_FAILED;
  }

  if(WideMode == SDMMC_BUS_WIDE_8B)
  {
    errorstate = SDMMC_CmdSwitch(pSDMMC1, 0x03B70200U);
    if(errorstate != SDMMC_ERROR_NONE)
    {
      return errorstate;
    }
  }
  else if(WideMode == SDMMC_BUS_WIDE_4B)
  {
    errorstate = SDMMC_CmdSwitch(pSDMMC1, 0x03B70100U);
    if(errorstate != SDMMC_ERROR_NONE)
    {
      return errorstate;
    }
  }
  else if(WideMode == SDMMC_BUS_WIDE_1B)
  {
    errorstate = SDMMC_CmdSwitch(pSDMMC1, 0x03B70000U);
    if(errorstate != SDMMC_ERROR_NONE)
    {
      return errorstate;
    }
  }
  else
  {
    /* WideMode is not a valid argument*/
    return SDMMC_ERROR_INVALID_PARAMETER;
  }
  
  /* Check for switch error and violation of the trial number of sending CMD 13 */
  while(busy == 0U)
  {
    if(count++ == SDMMC_MAX_TRIAL)
    {
      return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
    }
    
    /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
    errorstate = SDMMC_CmdSendStatus(pSDMMC1, (uint32_t)(MyEmmcCardInfo.RCA << 16));
    if(errorstate != SDMMC_ERROR_NONE)
    {
      return errorstate;
    }
    
    /* Get command response */
    response = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP1);
    
    /* Get operating voltage*/
    busy = (((response >> 7U) == 1U) ? 0U : 1U);
  }
      
  /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
  count = SDMMC_DATATIMEOUT;
  while((response & 0x00000100U) == 0U)
  {
    if(count-- == 0U)
    {
      return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
    }
    
    /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
    errorstate = SDMMC_CmdSendStatus(pSDMMC1, (uint32_t)(MyEmmcCardInfo.RCA << 16));
    if(errorstate != SDMMC_ERROR_NONE)
    {
      return errorstate;
    }
    
    /* Get command response */
    response = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP1);
  }
  
  SDMMC_InitStructure.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  SDMMC_InitStructure.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  SDMMC_InitStructure.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  SDMMC_InitStructure.BusWide = WideMode;
  SDMMC_InitStructure.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  SDMMC_InitStructure.ClockDiv = 2;

  /* Initialize SDMMC peripheral interface with default configuration */
  errorstate = SDMMC_Init(pSDMMC1, SDMMC_InitStructure);
  if (errorstate != HAL_OK)
  {
    return EMMC_SDIO_FUNCTION_FAILED;
  }

  return errorstate;
}
// uint32_t EmmcEnableWideBusOperation(uint32_t WideMode)
// {
//   uint32_t errorstate = SDMMC_ERROR_NONE;

//   if (SDMMC_BUS_WIDE_8B == WideMode)
//   {
//     // errorstate = EmmcEnWideBus(ENABLE);

//     if (SDMMC_ERROR_NONE == errorstate)
//     {
//       /* Default SDMMC peripheral configuration for SD card initialization */
//       SDMMC_InitStructure.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
//       SDMMC_InitStructure.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
//       SDMMC_InitStructure.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
//       SDMMC_InitStructure.BusWide = SDMMC_BUS_WIDE_1B;
//       SDMMC_InitStructure.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
//       SDMMC_InitStructure.ClockDiv = 2;

//       /* Initialize SDMMC peripheral interface with default configuration */
//       errorstate = SDMMC_Init(pSDMMC1, SDMMC_InitStructure);
//       if (errorstate != HAL_OK)
//       {
//         return EMMC_SDIO_FUNCTION_FAILED;
//       }
//     }
//   }

//   return (errorstate);
// }

/**
  * @brief  Selects od Deselects the corresponding card.
  * @param  addr: Address of the Card to be selected.
  * @retval Error code.
  */
uint32_t EmmcSelectDeselect(uint32_t addr)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;
  /* Select the Card */
  errorstate = SDMMC_CmdSelDesel(pSDMMC1, (uint32_t)(addr));
  
  return errorstate;
}

// EmmcError EmmcReadExtCsd(EmmcCardInfo *E)
// {
//   EmmcError Result = EMMC_OK;
//   uint32_t errorstate = SDMMC_ERROR_NONE;
//   uint32_t count = 0;
//   uint32_t *ExtCsdBuf;
//   ExtCsdBuf = (uint32_t *)(&(E->EmmcExtCsd.CsdBuf[0]));

//   /* Configure the SDMMC DPSM (Data Path State Machine) */
//   SDMMC_DataInitStructure.DataTimeOut = EMMC_DATATIMEOUT;
//   SDMMC_DataInitStructure.DataLength = (uint32_t)512;
//   SDMMC_DataInitStructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
//   SDMMC_DataInitStructure.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC;
//   SDMMC_DataInitStructure.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
//   SDMMC_DataInitStructure.DPSM = SDMMC_DPSM_ENABLE;
//   SDMMC_ConfigData(pSDMMC1, &SDMMC_DataInitStructure);

//   errorstate = SDMMC_CmdOperCond(pSDMMC1);
//   if (errorstate != SDMMC_ERROR_NONE)
//   {
//     return EMMC_ERROR;
//   }
//   // /* CMD8 */
//   // SDIO_CmdInitStructure.SDIO_Argument = 0;
//   // SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_HS_SEND_EXT_CSD;
//   // SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//   // SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//   // SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//   // SDIO_SendCommand(&SDIO_CmdInitStructure);

//   // Result = CmdResp1Error(EMMC_CMD_HS_SEND_EXT_CSD);

//   // if (EMMC_OK != Result)
//   // {
//   //   return (Result);
//   // }

//   while (!(pSDMMC1->STA & (SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND)))
//   {
//     if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXFIFOHF) != RESET)
//     {
//       for (count = 0; count < 8; count++)
//       {
//         *(ExtCsdBuf + count) = SDMMC_ReadFIFO(pSDMMC1);
//       }
//       ExtCsdBuf += 8;
//     }
//   }

//   if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DTIMEOUT) != RESET)
//   {
//     __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_FLAG_DTIMEOUT);
//     Result = EMMC_DATA_TIMEOUT;
//     return (Result);
//   }
//   else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DCRCFAIL) != RESET)
//   {
//     __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_FLAG_DCRCFAIL);
//     Result = EMMC_DATA_CRC_FAIL;
//     return (Result);
//   }
//   else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXOVERR) != RESET)
//   {
//     __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_FLAG_RXOVERR);
//     Result = EMMC_RX_OVERRUN;
//     return (Result);
//   }

//   count = EMMC_DATATIMEOUT;

//   while ((__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXDAVL) != RESET) && (count > 0))
//   {
//     *ExtCsdBuf = SDMMC_ReadFIFO(pSDMMC1);
//     ExtCsdBuf++;
//     count--;
//   }

//   /*!< Clear all the static flags */
//   __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);

//   return Result;
// }

/**
  * @brief  Gets the cuurent data transfer state.
  * @param  None
  * @retval EmmcTransferState: Data Transfer state.
  *   This value can be: 
  *        - EMMC_TRANSFER_STATE_OK: No data transfer is acting
  *        - EMMC_TRANSFER_STATE_BUSY: Data transfer is acting
  */
EmmcTransferState EmmcGetTransferState(void)
{
  if (pSDMMC1->STA & (SDMMC_FLAG_TXACT | SDMMC_FLAG_RXACT))
  {
    return(EMMC_TRANSFER_STATE_BUSY);
  }
  else
  {
    return(EMMC_TRANSFER_STATE_OK);
  }
}

/**
  * @brief  Aborts an ongoing data transfer.
  * @param  None
  * @retval EmmcError: EMMC Card Error code.
  */
uint32_t EmmcStopTransfer(void)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;

  errorstate = SDMMC_CmdStopTransfer(pSDMMC1);

  return errorstate;
}

/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the EMMC card 
  *         status (Card Status register).
  * @retval EmmcError: EMMC Card Error code.
  */
uint32_t EmmcSendStatus(uint32_t *pcardstatus)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;

  if (pcardstatus == NULL)
  {
    return SDMMC_ERROR_INVALID_PARAMETER;
  }

   /* Send Status command */
  errorstate = SDMMC_CmdSendStatus(pSDMMC1, (uint32_t)(MyEmmcCardInfo.RCA << 16));
  if(errorstate != HAL_OK)
  {
    return errorstate;
  }
  
  /* Get EMMC card status */
  *pcardstatus = SDMMC_GetResponse(pSDMMC1, SDMMC_RESP1);

  return errorstate;
}

/**
  * @brief  Returns the current card's state.
  * @param  None
  * @retval EmmcCardState: EMMC Card Error or EMMC Card Current State.
  */
EmmcCardState EmmcGetState(void)
{
    uint32_t Resp = 0;
	
    if (EmmcSendStatus(&Resp) != EMMC_OK)
    {
      return EMMC_CARD_ERROR;
    }
    else
    {
      return (EmmcCardState)((Resp >> 9) & 0x0F);
    }
}

/**
  * @brief  Gets the cuurent EMMC card data transfer status.
  * @param  None
  * @retval EmmcTransferState: Data Transfer state.
  *   This value can be: 
  *        - EMMC_TRANSFER_STATE_OK: No data transfer is acting
  *        - EMMC_TRANSFER_STATE_BUSY: Data transfer is acting
  */
EmmcTransferState EmmcGetStatus(void)
{
		EmmcCardState CardState =  EMMC_CARD_TRANSFER;

		CardState = EmmcGetState();
		
		if (CardState == EMMC_CARD_TRANSFER)
		{
			return(EMMC_TRANSFER_STATE_OK);
		}
		else if (CardState == EMMC_CARD_RECEIVING)
		{
			return(EMMC_TRANSFER_STATE_OK);
		}
		
		else if(CardState == EMMC_CARD_ERROR)
		{
			return (EMMC_TRANSFER_STATE_ERROR);
		}
		else
		{
			return(EMMC_TRANSFER_STATE_BUSY);
		}
}

/**
  * @brief  Reads block(s) from a specified address in a card. The Data transfer 
  *         is managed by polling mode.  
  * @note   This API should be followed by a check on the card state through
  *         EmmcGetState().
  * @param  pData: pointer to the buffer that will contain the received data
  * @param  BlockAdd: Block Address from where data is to be read 
  * @param  NumberOfBlocks: Number of EMMC blocks to read   
  * @param  Timeout: Specify timeout value
  * @retval status
  */
uint32_t EMMC_ReadBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;
  uint32_t tickstart = HAL_GetTick();
  uint32_t count = 0, *tempbuff = (uint32_t *)pData;

  if (pData == NULL)
  {
    return SDMMC_ERROR_INVALID_PARAMETER;
  }

  while (EMMC_CARD_TRANSFER != EmmcGetState())
  {
    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_BUSY;
    }
  }

  pSDMMC1->DCTRL = 0x0U;

  // /* Set Block Size for Card */
  // errorstate = SDMMC_CmdBlockLength(pSDMMC1, BLOCKSIZE);
  // if (errorstate != SDMMC_ERROR_NONE)
  // {
  //   /* Clear all the static flags */
  //   __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
  //   return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
  // }

  /* Configure the SD DPSM (Data Path State Machine) */

  SDMMC_DataInitStructure.DataTimeOut = SDMMC_DATATIMEOUT;
  SDMMC_DataInitStructure.DataLength = NumberOfBlocks * BLOCKSIZE;
  SDMMC_DataInitStructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
  SDMMC_DataInitStructure.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC;
  SDMMC_DataInitStructure.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
  SDMMC_DataInitStructure.DPSM = SDMMC_DPSM_ENABLE;
  SDMMC_ConfigData(pSDMMC1, &SDMMC_DataInitStructure);

  /* Read block(s) in polling mode */
  if (NumberOfBlocks > 1)
  {
    errorstate = SDMMC_CmdReadMultiBlock(pSDMMC1, BlockAdd);
  }
  else
  {
    /* Read Single Block command */
    errorstate = SDMMC_CmdReadSingleBlock(pSDMMC1, BlockAdd);
  }
  
  if (errorstate != SDMMC_ERROR_NONE)
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
  }

  #if EMMC_NO_DMA_SUPPORT_OS
    taskENTER_CRITICAL();
  #endif /* EMMC_NO_DMA_SUPPORT_OS */
  /* Poll on SDMMC flags */
  while (!__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
  {
    if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXFIFOHF))
    {
      /* Read data from SDMMC Rx FIFO */
      for (count = 0U; count < 8U; count++)
      {
        *(tempbuff + count) = SDMMC_ReadFIFO(pSDMMC1);
      }
      tempbuff += 8U;
    }

    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_TIMEOUT;
    }
  }

  #if EMMC_NO_DMA_SUPPORT_OS
    taskEXIT_CRITICAL();
  #endif /* EMMC_NO_DMA_SUPPORT_OS */

  /* Send stop transmission command in case of multiblock read */
  if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
  {
    /* Send stop transmission command */
    errorstate = SDMMC_CmdStopTransfer(pSDMMC1);
    if (errorstate != SDMMC_ERROR_NONE)
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
    }
  }

  /* Get error state */
  if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DTIMEOUT))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_DATA_TIMEOUT;
  }
  else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DCRCFAIL))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_DATA_CRC_FAIL;
  }
  else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXOVERR))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_RX_OVERRUN;
  }

  /* Empty FIFO if there is still any data */
  while ((__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXDAVL)))
  {
    *tempbuff = SDMMC_ReadFIFO(pSDMMC1);
    tempbuff++;

    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_TIMEOUT;
    }
  }

  /* Clear all the static flags */
  __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);

  return SDMMC_ERROR_NONE;
}

/**
  * @brief  Allows to read blocks from a specified address  in a card.  The Data
  *         transfer can be managed by DMA mode or Polling mode. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and EMMC Card status.
  *          - Emmc_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - EmmcGetStatus(): to check that the EMMC Card has finished the 
  *            data transfmer and it is ready for data.   
  * @param  pData: pointer to the buffer that will contain the received data.
  * @param  BlockAdd: Address from where data are to be read.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval EmmcError: EMMC Card Error code.
  */
uint32_t EmmcReadBlocksDMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;
  uint32_t tickstart = HAL_GetTick();

  if (pData == NULL)
  {
    return EMMC_INVALID_PARAMETER;
  }

  while (EMMC_CARD_TRANSFER != EmmcGetState())
  {
    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_BUSY;
    }
  }

  __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
  // __HAL_DMA_CLEAR_FLAG(pHdma_sdmmc1_rx, DMA_FLAG_TC5 | DMA_FLAG_HT5 | DMA_FLAG_TE5 | DMA_FLAG_GL5);
  __HAL_DMA_CLEAR_FLAG(pHdma_sdmmc1_rx, DMA_FLAG_GL5);

  if (sdmmcDirect != SDMMC_DIRECTION_READ)
  {
    pHdma_sdmmc1_rx->Instance = DMA2_Channel5;
    pHdma_sdmmc1_rx->Init.Request = DMA_REQUEST_7;
    pHdma_sdmmc1_rx->Init.Direction = DMA_PERIPH_TO_MEMORY;
    pHdma_sdmmc1_rx->Init.PeriphInc = DMA_PINC_DISABLE;
    pHdma_sdmmc1_rx->Init.MemInc = DMA_MINC_ENABLE;
    pHdma_sdmmc1_rx->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    pHdma_sdmmc1_rx->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    pHdma_sdmmc1_rx->Init.Mode = DMA_NORMAL;
    pHdma_sdmmc1_rx->Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(pHdma_sdmmc1_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    sdmmcDirect = SDMMC_DIRECTION_READ;
  }

  /* Initialize data control register */
  pSDMMC1->DCTRL = 0x0U;

  /* Enable the DMA Channel */
  if (HAL_OK != HAL_DMA_Start(pHdma_sdmmc1_rx, (uint32_t)(&(pSDMMC1->FIFO)), (uint32_t)pData, (uint32_t)(NumberOfBlocks * BLOCKSIZE) / 4))
  {
    HAL_DMA_DeInit(pHdma_sdmmc1_rx);

    pHdma_sdmmc1_rx->Instance = DMA2_Channel5;
    pHdma_sdmmc1_rx->Init.Request = DMA_REQUEST_7;
    pHdma_sdmmc1_rx->Init.Direction = DMA_PERIPH_TO_MEMORY;
    pHdma_sdmmc1_rx->Init.PeriphInc = DMA_PINC_DISABLE;
    pHdma_sdmmc1_rx->Init.MemInc = DMA_MINC_ENABLE;
    pHdma_sdmmc1_rx->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    pHdma_sdmmc1_rx->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    pHdma_sdmmc1_rx->Init.Mode = DMA_NORMAL;
    pHdma_sdmmc1_rx->Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(pHdma_sdmmc1_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    HAL_DMA_Start(pHdma_sdmmc1_rx, (uint32_t)(&(pSDMMC1->FIFO)), (uint32_t)pData, (uint32_t)(NumberOfBlocks * BLOCKSIZE) / 4);
  }

  /* Enable SDMMC DMA transfer */
  __SDMMC_DMA_ENABLE(pSDMMC1);

  /* Configure the SDMMC DPSM (Data Path State Machine) */
  SDMMC_DataInitStructure.DataTimeOut = SDMMC_DATATIMEOUT;
  SDMMC_DataInitStructure.DataLength = NumberOfBlocks * BLOCKSIZE;
  SDMMC_DataInitStructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
  SDMMC_DataInitStructure.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC;
  SDMMC_DataInitStructure.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
  SDMMC_DataInitStructure.DPSM = SDMMC_DPSM_ENABLE;
  SDMMC_ConfigData(pSDMMC1, &SDMMC_DataInitStructure);

  /* Read Blocks in DMA mode */
  if (NumberOfBlocks > 1U)
  {
    /* Read Multi Block command */
    errorstate = SDMMC_CmdReadMultiBlock(pSDMMC1, BlockAdd);
  }
  else
  {
    /* Read Single Block command */
    errorstate = SDMMC_CmdReadSingleBlock(pSDMMC1, BlockAdd);
  }

  if (errorstate != SDMMC_ERROR_NONE)
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return EMMC_REQUEST_NOT_APPLICABLE;
  }

  errorstate = HAL_DMA_PollForTransfer(pHdma_sdmmc1_rx, HAL_DMA_FULL_TRANSFER, Timeout);
  
  while (!__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
  {
    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      __SDMMC_DMA_DISABLE(pSDMMC1);
      errorstate = HAL_DMA_Abort(pHdma_sdmmc1_rx);
      return SDMMC_ERROR_TIMEOUT;
    }
  }

	/* Send stop transmission command in case of multiblock read */
  if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
  {
    /* Send stop transmission command */
    errorstate = SDMMC_CmdStopTransfer(pSDMMC1);
    if (errorstate != SDMMC_ERROR_NONE)
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      __SDMMC_DMA_DISABLE(pSDMMC1);
      errorstate = HAL_DMA_Abort(pHdma_sdmmc1_rx);
      return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
    }
  }

  __SDMMC_DMA_DISABLE(pSDMMC1);
  errorstate = HAL_DMA_Abort(pHdma_sdmmc1_rx);

  /* Get error state */
  if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DTIMEOUT))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_DATA_TIMEOUT;
  }
  else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DCRCFAIL))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_DATA_CRC_FAIL;
  }
  else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXOVERR))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_RX_OVERRUN;
  }

  /* Empty FIFO if there is still any data */
  while ((__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_RXDAVL)))
  {
    uint32_t tempbuff = SDMMC_ReadFIFO(pSDMMC1);

    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_TIMEOUT;
    }
  }

  /* Clear all the static flags */
  __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);

  return SDMMC_ERROR_NONE;
}

/**
  * @brief  Allows to write block(s) to a specified address in a card. The Data
  *         transfer is managed by polling mode.  
  * @note   This API should be followed by a check on the card state through
  *         EmmcGetState().
  * @param  pData: pointer to the buffer that will contain the data to transmit
  * @param  BlockAdd: Block Address where data will be written  
  * @param  NumberOfBlocks: Number of EMMC blocks to write 
  * @param  Timeout: Specify timeout value
  * @retval status
  */
uint32_t EMMC_WriteBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;
  uint32_t tickstart = HAL_GetTick();
  uint32_t count = 0;
  uint32_t *tempbuff = (uint32_t *)pData;
  
  if(NULL == pData)
  {
    return SDMMC_ERROR_INVALID_PARAMETER;
  }

  while (EMMC_CARD_TRANSFER != EmmcGetState())
  {
    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_BUSY;
    }
  }
    
    /* Initialize data control register */
    pSDMMC1->DCTRL = 0;
    
    // /* Set Block Size for Card */ 
    // errorstate = SDMMC_CmdBlockLength(pSDMMC1, BLOCKSIZE);
    // if(errorstate != SDMMC_ERROR_NONE)
    // {
    //   /* Clear all the static flags */
    //   __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    //   return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
    // }
    
    /* Configure the SD DPSM (Data Path State Machine) */
    SDMMC_DataInitStructure.DataTimeOut = SDMMC_DATATIMEOUT;
    SDMMC_DataInitStructure.DataLength = NumberOfBlocks * BLOCKSIZE;
    SDMMC_DataInitStructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    SDMMC_DataInitStructure.TransferDir = SDMMC_TRANSFER_DIR_TO_CARD;
    SDMMC_DataInitStructure.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
    SDMMC_DataInitStructure.DPSM = SDMMC_DPSM_ENABLE;
    SDMMC_ConfigData(pSDMMC1, &SDMMC_DataInitStructure);

    /* Write Blocks in Polling mode */
    if(NumberOfBlocks > 1U)
    {
      /* Write Multi Block command */ 
      errorstate = SDMMC_CmdWriteMultiBlock(pSDMMC1, BlockAdd);
    }
    else
    {
      /* Write Single Block command */
      errorstate = SDMMC_CmdWriteSingleBlock(pSDMMC1, BlockAdd);
    }

    if(errorstate != SDMMC_ERROR_NONE)
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
    }

    #if EMMC_NO_DMA_SUPPORT_OS
        taskENTER_CRITICAL();
    #endif /* EMMC_NO_DMA_SUPPORT_OS */
    /* Write block(s) in polling mode */
    while(!__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
    {
      if(__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_TXFIFOHE))
      {
        /* Write data to SDMMC Tx FIFO */
        for(count = 0U; count < 8U; count++)
        {
          SDMMC_WriteFIFO(pSDMMC1, (tempbuff + count));
        }
        tempbuff += 8U;
      }
      
      if((Timeout == 0U)||((HAL_GetTick()-tickstart) >=  Timeout))
      {
        /* Clear all the static flags */
        __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
        return SDMMC_ERROR_TIMEOUT;
      }
    }
    #if EMMC_NO_DMA_SUPPORT_OS
        taskEXIT_CRITICAL();
    #endif /* EMMC_NO_DMA_SUPPORT_OS */

    /* Send stop transmission command in case of multiblock write */
    if(__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
    {
      /* Send stop transmission command */
      errorstate = SDMMC_CmdStopTransfer(pSDMMC1);
      if (errorstate != HAL_SD_ERROR_NONE)
      {
        /* Clear all the static flags */
        __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
        return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
      }
    }
    
    /* Get error state */
    if(__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DTIMEOUT))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_DATA_TIMEOUT;
    }
    else if(__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DCRCFAIL))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_DATA_CRC_FAIL;
    }
    else if(__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_TXUNDERR))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_TX_UNDERRUN;
    }
    
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    
    return SDMMC_ERROR_NONE;
}

/**
  * @brief  Allows to write blocks starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode only. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and EMMC Card status.
  *          - Emmc_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - Emmc_GetStatus(): to check that the SDMMC Card has finished the 
  *            data transfer and it is ready for data.     
  * @param  BlockAdd: Address from where data are to be read.
  * @param  pData: pointer to the buffer that contain the data to be transferred.
  * @param  BlockSize: the SDMMC card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be written.
  */
uint32_t EmmcWriteBlocksDMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
  uint32_t errorstate = SDMMC_ERROR_NONE;
  uint32_t tickstart = HAL_GetTick();
  
  if (pData == NULL)
  {
    return EMMC_INVALID_PARAMETER;
  }

  while (EMMC_CARD_TRANSFER != EmmcGetState())
  {
    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      return SDMMC_ERROR_BUSY;
    }
  }

  __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
  // __HAL_DMA_CLEAR_FLAG(pHdma_sdmmc1_tx, DMA_FLAG_TC5 | DMA_FLAG_HT5 | DMA_FLAG_TE5 | DMA_FLAG_GL5);
  __HAL_DMA_CLEAR_FLAG(pHdma_sdmmc1_tx, DMA_FLAG_GL5);

  if (sdmmcDirect != SDMMC_DIRECTION_WRITE)
  {
    pHdma_sdmmc1_tx->Instance = DMA2_Channel5;
    pHdma_sdmmc1_tx->Init.Request = DMA_REQUEST_7;
    pHdma_sdmmc1_tx->Init.Direction = DMA_MEMORY_TO_PERIPH;
    pHdma_sdmmc1_tx->Init.PeriphInc = DMA_PINC_DISABLE;
    pHdma_sdmmc1_tx->Init.MemInc = DMA_MINC_ENABLE;
    pHdma_sdmmc1_tx->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    pHdma_sdmmc1_tx->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    pHdma_sdmmc1_tx->Init.Mode = DMA_NORMAL;
    pHdma_sdmmc1_tx->Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(pHdma_sdmmc1_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }
    sdmmcDirect = SDMMC_DIRECTION_WRITE;
  }

  /* Initialize data control register */
  pSDMMC1->DCTRL = 0x0U;

  /* Write Blocks in Polling mode */
  if (NumberOfBlocks > 1U)
  {
    errorstate = SDMMC_CmdWriteMultiBlock(pSDMMC1, BlockAdd);
  }
  else
  {
    errorstate = SDMMC_CmdWriteSingleBlock(pSDMMC1, BlockAdd);
  }

  if (errorstate != HAL_SD_ERROR_NONE)
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return EMMC_ERROR;
  }
	
  /* Enable the DMA Channel */
  if (HAL_OK != HAL_DMA_Start(pHdma_sdmmc1_tx, (uint32_t)pData, (uint32_t)(&(pSDMMC1->FIFO)), (uint32_t)(NumberOfBlocks * BLOCKSIZE) / 4))
  {
    HAL_DMA_DeInit(pHdma_sdmmc1_tx);

    pHdma_sdmmc1_tx->Instance = DMA2_Channel5;
    pHdma_sdmmc1_tx->Init.Request = DMA_REQUEST_7;
    pHdma_sdmmc1_tx->Init.Direction = DMA_MEMORY_TO_PERIPH;
    pHdma_sdmmc1_tx->Init.PeriphInc = DMA_PINC_DISABLE;
    pHdma_sdmmc1_tx->Init.MemInc = DMA_MINC_ENABLE;
    pHdma_sdmmc1_tx->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    pHdma_sdmmc1_tx->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    pHdma_sdmmc1_tx->Init.Mode = DMA_NORMAL;
    pHdma_sdmmc1_tx->Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(pHdma_sdmmc1_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    HAL_DMA_Start(pHdma_sdmmc1_tx, (uint32_t)pData, (uint32_t)(&(pSDMMC1->FIFO)), (uint32_t)(NumberOfBlocks * BLOCKSIZE) / 4);
  }

  /* Enable SDMMC DMA transfer */
  __SDMMC_DMA_ENABLE(pSDMMC1);

  /* Configure the SD DPSM (Data Path State Machine) */
  SDMMC_DataInitStructure.DataTimeOut = SDMMC_DATATIMEOUT;
  SDMMC_DataInitStructure.DataLength = NumberOfBlocks * BLOCKSIZE;
  SDMMC_DataInitStructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
  SDMMC_DataInitStructure.TransferDir = SDMMC_TRANSFER_DIR_TO_CARD;
  SDMMC_DataInitStructure.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
  SDMMC_DataInitStructure.DPSM = SDMMC_DPSM_ENABLE;
  SDMMC_ConfigData(pSDMMC1, &SDMMC_DataInitStructure);

  errorstate = HAL_DMA_PollForTransfer(pHdma_sdmmc1_tx, HAL_DMA_FULL_TRANSFER, Timeout);

  /* Write block(s) in polling mode */
  while (!__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
  {
    if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout))
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      __SDMMC_DMA_DISABLE(pSDMMC1);
      errorstate = HAL_DMA_Abort(pHdma_sdmmc1_tx);
      return SDMMC_ERROR_TIMEOUT;
    }
  }

  /* Send stop transmission command in case of multiblock write */
  if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
  {
    /* Send stop transmission command */
    errorstate = SDMMC_CmdStopTransfer(pSDMMC1);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
      __SDMMC_DMA_DISABLE(pSDMMC1);
      errorstate = HAL_DMA_Abort(pHdma_sdmmc1_tx);
      return SDMMC_ERROR_REQUEST_NOT_APPLICABLE;
    }
  }

  __SDMMC_DMA_DISABLE(pSDMMC1);
  errorstate = HAL_DMA_Abort(pHdma_sdmmc1_tx);

  /* Get error state */
  if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DTIMEOUT))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_DATA_TIMEOUT;
  }
  else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_DCRCFAIL))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_DATA_CRC_FAIL;
  }
  else if (__SDMMC_GET_FLAG(pSDMMC1, SDMMC_FLAG_TXUNDERR))
  {
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);
    return SDMMC_ERROR_TX_UNDERRUN;
  }

  /* Clear all the static flags */
  __SDMMC_CLEAR_FLAG(pSDMMC1, SDMMC_STATIC_FLAGS);

  return SDMMC_ERROR_NONE;
}

#define EMMC_DATA_TRANSFER_TIMEOUT_MS (5 * 1000)
#define EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE (1 * 1024)
uint8_t EmmcWriteBuf[EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE] = {0};
uint8_t EmmcReadBuf[EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE] = {0};

uint32_t Emmc_ReadData(uint8_t * pData, uint32_t len, uint32_t address)
{
  uint32_t count = 0;
  uint32_t left = 0, firstLen = 0;
  uint32_t i = 0, j = 0, k = 0, alignAddr = 0, offset = 0, blockAddr = 0;

  if (pData == NULL || len == 0)
  {
    return 1;
  }

  alignAddr = address & ~EMMC_SECTOR_ADDRESS_ALIGNMENT_MASK;
  offset = address - alignAddr;

  if (offset)
  {
    memset(EmmcReadBuf, 0, EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE);

    firstLen = ((len + offset) > EMMC_BLOCK_SIZE_BYTE) ? (EMMC_BLOCK_SIZE_BYTE - offset) : len;

    if (EmmcReadBlocksDMA(EmmcReadBuf, alignAddr / EMMC_BLOCK_SIZE_BYTE, 1, EMMC_DATA_TRANSFER_TIMEOUT_MS))
    {
      return 2;
    }

    memcpy(pData, EmmcReadBuf + offset, firstLen);

    pData += firstLen;
    len -= firstLen;
    address = alignAddr + EMMC_BLOCK_SIZE_BYTE;
  }

  if (len == 0)
  {
    return 0;
  }

  count = len / EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE;
  left = len % EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE;
  blockAddr = address / EMMC_BLOCK_SIZE_BYTE;

  for (i = 0; i < count; i++)
  {
    if (EmmcReadBlocksDMA(pData + i * EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE
    , blockAddr + i * (EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE / EMMC_BLOCK_SIZE_BYTE)
    , EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE / EMMC_BLOCK_SIZE_BYTE
    , EMMC_DATA_TRANSFER_TIMEOUT_MS))
    {
      return 3;
    }
  }

  if (left)
  {
    memset(EmmcReadBuf, 0, EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE);
    j = left / EMMC_BLOCK_SIZE_BYTE;
    k = left % EMMC_BLOCK_SIZE_BYTE;

    if (k)
    {
      j++;
    }
    
    if (EmmcReadBlocksDMA(EmmcReadBuf
    , blockAddr + count * (EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE / EMMC_BLOCK_SIZE_BYTE)
    , j , EMMC_DATA_TRANSFER_TIMEOUT_MS))
    {
      return 4;
    }

    memcpy(pData + count * EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE, EmmcReadBuf, left);
  }
  
  return 0;
}

uint32_t Emmc_WriteData(uint8_t * pData, uint32_t len, uint32_t address)
{
  uint32_t count = 0;
  uint32_t left = 0, firstLen = 0;
  uint32_t i = 0, j = 0, k = 0, alignAddr = 0, offset = 0, blockAddr = 0;

  if (pData == NULL || len == 0)
  {
    return 1;
  }

  alignAddr = address & ~EMMC_SECTOR_ADDRESS_ALIGNMENT_MASK;
  offset = address - alignAddr;

  if (offset)
  {
    firstLen = ((len + offset) > EMMC_BLOCK_SIZE_BYTE) ? (EMMC_BLOCK_SIZE_BYTE - offset) : len;

    if (EmmcReadBlocksDMA(EmmcWriteBuf, alignAddr / EMMC_BLOCK_SIZE_BYTE, 1, EMMC_DATA_TRANSFER_TIMEOUT_MS))
    {
      return 2;
    }

    memcpy(EmmcWriteBuf + offset, pData , firstLen);

    if ((len + offset) < EMMC_BLOCK_SIZE_BYTE)
    {
      memset(EmmcWriteBuf + offset + firstLen, 0, EMMC_BLOCK_SIZE_BYTE - offset - firstLen);
    }

    if (EmmcWriteBlocksDMA(EmmcWriteBuf, alignAddr / EMMC_BLOCK_SIZE_BYTE, 1, EMMC_DATA_TRANSFER_TIMEOUT_MS))
    {
      return 3;
    }

    pData += firstLen;
    len -= firstLen;
    address = alignAddr + EMMC_BLOCK_SIZE_BYTE;
  }

  if (len == 0)
  {
    return 0;
  }

  count = len / EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE;
  left = len % EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE;
  blockAddr = address / EMMC_BLOCK_SIZE_BYTE;

  for (i = 0; i < count; i++)
  {
    if (EmmcWriteBlocksDMA(pData + i * EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE
    , blockAddr + i * (EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE / EMMC_BLOCK_SIZE_BYTE)
    , EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE / EMMC_BLOCK_SIZE_BYTE
    , EMMC_DATA_TRANSFER_TIMEOUT_MS))
    {
      return 4;
    }
  }

  if (left)
  {
    memset(EmmcWriteBuf, 0, EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE);
    memcpy(EmmcWriteBuf, pData + count * EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE, left);
    j = left / EMMC_BLOCK_SIZE_BYTE;
    k = left % EMMC_BLOCK_SIZE_BYTE;

    if (k)
    {
      j++;
    }
    
    if (EmmcWriteBlocksDMA(EmmcWriteBuf
    , blockAddr + count * (EMMC_DATA_TRANSFER_BUFFER_SIZE_BYTE / EMMC_BLOCK_SIZE_BYTE)
    , j , EMMC_DATA_TRANSFER_TIMEOUT_MS))
    {
      return 5;
    }
  }
  
  return 0;
}
