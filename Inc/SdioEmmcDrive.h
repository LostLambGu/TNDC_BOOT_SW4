/*******************************************************************************
* File Name          : SdioEmmcDrive.h
* Author             : Yangjie Gu
* Description        : This file provides all the SdioEmmcDrive functions.

* History:
*  02/27/2018 : SdioEmmcDrive V1.00
*******************************************************************************
	**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDIO_EMMC_H
#define __SDIO_EMMC_H

#ifdef __cplusplus
 extern "C" {
#endif
 
#include "stm32l4xx_hal.h"
	 
/** 
  * @brief  SDIO specific error defines  
  */   
typedef enum
{
 
  EMMC_CMD_CRC_FAIL                    = (1), /*!< Command response received (but CRC check failed) */
  EMMC_DATA_CRC_FAIL                   = (2), /*!< Data bock sent/received (CRC check Failed) */
  EMMC_CMD_RSP_TIMEOUT                 = (3), /*!< Command response timeout */
  EMMC_DATA_TIMEOUT                    = (4), /*!< Data time out */
  EMMC_TX_UNDERRUN                     = (5), /*!< Transmit FIFO under-run */
  EMMC_RX_OVERRUN                      = (6), /*!< Receive FIFO over-run */
  EMMC_START_BIT_ERR                   = (7), /*!< Start bit not detected on all data signals in widE bus mode */
  EMMC_CMD_OUT_OF_RANGE                = (8), /*!< CMD's argument was out of range.*/
  EMMC_ADDR_MISALIGNED                 = (9), /*!< Misaligned address */
  EMMC_BLOCK_LEN_ERR                   = (10), /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
  EMMC_ERASE_SEQ_ERR                   = (11), /*!< An error in the sequence of erase command occurs.*/
  EMMC_BAD_ERASE_PARAM                 = (12), /*!< An Invalid selection for erase groups */
  EMMC_WRITE_PROT_VIOLATION            = (13), /*!< Attempt to program a write protect block */
  EMMC_LOCK_UNLOCK_FAILED              = (14), /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
  EMMC_COM_CRC_FAILED                  = (15), /*!< CRC check of the previous command failed */
  EMMC_ILLEGAL_CMD                     = (16), /*!< Command is not legal for the card state */
  EMMC_CARD_ECC_FAILED                 = (17), /*!< Card internal ECC was applied but failed to correct the data */
  EMMC_CC_ERROR                        = (18), /*!< Internal card controller error */
  EMMC_GENERAL_UNKNOWN_ERROR           = (19), /*!< General or Unknown error */
  EMMC_STREAM_READ_UNDERRUN            = (20), /*!< The card could not sustain data transfer in stream read operation. */
  EMMC_STREAM_WRITE_OVERRUN            = (21), /*!< The card could not sustain data programming in stream mode */
  EMMC_CID_CEMMC_OVERWRITE             = (22), /*!< CID/CSD overwrite error */
  EMMC_WP_ERASE_SKIP                   = (23), /*!< only partial address space was erased */
  EMMC_CARD_ECC_DISABLED               = (24), /*!< Command has been executed without using internal ECC */
  EMMC_ERASE_RESET                     = (25), /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
  EMMC_AKE_SEQ_ERROR                   = (26), /*!< Error in sequence of authentication. */
  EMMC_INVALID_VOLTRANGE               = (27),
  EMMC_ADDR_OUT_OF_RANGE               = (28),
  EMMC_SWITCH_ERROR                    = (29),
  EMMC_SDIO_DISABLED                   = (30),
  EMMC_SDIO_FUNCTION_BUSY              = (31),
  EMMC_SDIO_FUNCTION_FAILED            = (32),
  EMMC_SDIO_UNKNOWN_FUNCTION           = (33),

/** 
  * @brief  Standard error defines   
  */ 
  EMMC_INTERNAL_ERROR, 
  EMMC_NOT_CONFIGURED,
  EMMC_REQUEST_PENDING, 
  EMMC_REQUEST_NOT_APPLICABLE, 
  EMMC_INVALID_PARAMETER,  
  EMMC_UNSUPPORTED_FEATURE,  
  EMMC_UNSUPPORTED_HW,  
  EMMC_ERROR,  
  EMMC_OK = 0 
} EmmcError;

/** 
  * @brief  SDMMC Transfer state  
  */   
typedef enum
{
  EMMC_TRANSFER_STATE_OK  = 0,
  EMMC_TRANSFER_STATE_BUSY = 1,
  EMMC_TRANSFER_STATE_ERROR
} EmmcTransferState;

/** 
  * @brief  EMMC Card States 
  */   
typedef enum
{
  EMMC_CARD_READY                  = ((uint32_t)0x00000001),
  EMMC_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),
  EMMC_CARD_STANDBY                = ((uint32_t)0x00000003),
  EMMC_CARD_TRANSFER               = ((uint32_t)0x00000004),
  EMMC_CARD_SENDING                = ((uint32_t)0x00000005),
  EMMC_CARD_RECEIVING              = ((uint32_t)0x00000006),
  EMMC_CARD_PROGRAMMING            = ((uint32_t)0x00000007),
  EMMC_CARD_DISCONNECTED           = ((uint32_t)0x00000008),
  EMMC_CARD_ERROR                  = ((uint32_t)0x000000FF)
}EmmcCardState;


/** @defgroup MMC_Exported_Types_Group4 Card Specific Data: CSD Register 
  * @{
  */
typedef struct
{
  __IO uint8_t  CSDStruct;            /*!< CSD structure                         */
  __IO uint8_t  SysSpecVersion;       /*!< System specification version          */
  __IO uint8_t  Reserved1;            /*!< Reserved                              */
  __IO uint8_t  TAAC;                 /*!< Data read access time 1               */
  __IO uint8_t  NSAC;                 /*!< Data read access time 2 in CLK cycles */
  __IO uint8_t  MaxBusClkFrec;        /*!< Max. bus clock frequency              */
  __IO uint16_t CardComdClasses;      /*!< Card command classes                  */
  __IO uint8_t  RdBlockLen;           /*!< Max. read data block length           */
  __IO uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed       */
  __IO uint8_t  WrBlockMisalign;      /*!< Write block misalignment              */
  __IO uint8_t  RdBlockMisalign;      /*!< Read block misalignment               */
  __IO uint8_t  DSRImpl;              /*!< DSR implemented                       */
  __IO uint8_t  Reserved2;            /*!< Reserved                              */
  __IO uint32_t DeviceSize;           /*!< Device Size                           */
  __IO uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min           */
  __IO uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max           */
  __IO uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min          */
  __IO uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max          */
  __IO uint8_t  DeviceSizeMul;        /*!< Device size multiplier                */
  __IO uint8_t  EraseGrSize;          /*!< Erase group size                      */
  __IO uint8_t  EraseGrMul;           /*!< Erase group size multiplier           */
  __IO uint8_t  WrProtectGrSize;      /*!< Write protect group size              */
  __IO uint8_t  WrProtectGrEnable;    /*!< Write protect group enable            */
  __IO uint8_t  ManDeflECC;           /*!< Manufacturer default ECC              */
  __IO uint8_t  WrSpeedFact;          /*!< Write speed factor                    */
  __IO uint8_t  MaxWrBlockLen;        /*!< Max. write data block length          */
  __IO uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed      */
  __IO uint8_t  Reserved3;            /*!< Reserved                              */
  __IO uint8_t  ContentProtectAppli;  /*!< Content protection application        */
  __IO uint8_t  FileFormatGrouop;     /*!< File format group                     */
  __IO uint8_t  CopyFlag;             /*!< Copy flag (OTP)                       */
  __IO uint8_t  PermWrProtect;        /*!< Permanent write protection            */
  __IO uint8_t  TempWrProtect;        /*!< Temporary write protection            */
  __IO uint8_t  FileFormat;           /*!< File format                           */
  __IO uint8_t  ECC;                  /*!< ECC code                              */
  __IO uint8_t  CSD_CRC;              /*!< CSD CRC                               */
  __IO uint8_t  Reserved4;            /*!< Always 1                              */
  
} EMMC_CardCSDTypeDef;



/** 
  * @brief  Card Specific Data: EXTCSD Register   
  */ 

typedef union
{
		struct _EXT_CSD
		{
			__IO uint8_t  Reserved26[32];      		
			__IO uint8_t 	FLUSH_CACHE;	
			__IO uint8_t 	CACHE_CTRL; 	
			__IO uint8_t 	POWER_OFF_NOTIFICATION;	
			__IO uint8_t 	PACKED_FAILURE_INDEX;	
			__IO uint8_t 	PACKED_COMMAND_STATUS;	
			__IO uint8_t 	CONTEXT_CONF[15]; 	
			__IO uint8_t 	EXT_PARTITIONS_ATTRIBUTE[2];	
			__IO uint8_t 	EXCEPTION_EVENTS_STATUS[2];	
			__IO uint8_t 	EXCEPTION_EVENTS_CTRL[2];	
			__IO uint8_t 	DYNCAP_NEEDED; 	
			__IO uint8_t 	CLASS_6_CTRL; 	
			__IO uint8_t 	INI_TIMEOUT_EMU; 	
			__IO uint8_t 	DATA_SECTOR_SIZE; 	
			__IO uint8_t 	USE_NATIVE_SECTOR; 	
			__IO uint8_t 	NATIVE_SECTOR_SIZE;	
			__IO uint8_t 	VENDOR_SPECIFIC_FIELD[64];	
			__IO uint8_t  Reserved25;		
			__IO uint8_t 	PROGRAM_CID_CSD_DDR_SUPPORT;	
			__IO uint8_t 	PERIODIC_WAKEUP; 	
			__IO uint8_t 	TCASE_SUPPORT; 	
			__IO uint8_t  Reserved24;		
			__IO uint8_t 	SEC_BAD_BLK_MGMNT;	
			__IO uint8_t  Reserved23;		
			__IO uint8_t 	ENH_START_ADDR[4];	
			__IO uint8_t 	ENH_SIZE_MULT[3]; 	
			__IO uint8_t 	GP_SIZE_MULT[12]; 	
			__IO uint8_t 	PARTITION_SETTING_COMPLETED;	
			__IO uint8_t 	PARTITIONS_ATTRIBUTE; 	
			__IO uint8_t 	MAX_ENH_SIZE_MULT[3]; 	
			__IO uint8_t 	PARTITIONING_SUPPORT; 	
			__IO uint8_t 	HPI_MGMT; 	
			__IO uint8_t 	RST_n_FUNCTION; 	
			__IO uint8_t 	BKOPS_EN; 	
			__IO uint8_t 	BKOPS_START; 	
			__IO uint8_t 	SANITIZE_START; 	
			__IO uint8_t 	WR_REL_PARAM; 	
			__IO uint8_t 	WR_REL_SET; 	
			__IO uint8_t 	RPMB_SIZE_MULT; 	
			__IO uint8_t 	FW_CONFIG; 	
			__IO uint8_t  Reserved22;		
			__IO uint8_t 	USER_WP;	
			__IO uint8_t  Reserved21;		
			__IO uint8_t 	BOOT_WP;	
			__IO uint8_t 	BOOT_WP_STATUS; 	
			__IO uint8_t 	ERASE_GROUP_DEF; 	
			__IO uint8_t  Reserved20;		
			__IO uint8_t  BOOT_BUS_CONDITIONS;		
			__IO uint8_t  BOOT_CONFIG_PROT; 		
			__IO uint8_t 	PARTITION_CONFIG; 	
			__IO uint8_t  Reserved19;		
			__IO uint8_t  ERASED_MEM_CONT;		
			__IO uint8_t  Reserved18;		
			__IO uint8_t  BUS_WIDTH;		
			__IO uint8_t  Reserved17;		
			__IO uint8_t  HS_TIMING;		
			__IO uint8_t  Reserved16;		
			__IO uint8_t  POWER_CLASS;		
			__IO uint8_t  Reserved15;		
			__IO uint8_t  CMD_SET_REV;		
			__IO uint8_t  Reserved14;		
			__IO uint8_t  CMD_SET;		
			__IO uint8_t  Reserved13;		
			__IO uint8_t  EXT_CSD_REV ;		
			__IO uint8_t  Reserved12;		
			__IO uint8_t  Reserved11;		
			__IO uint8_t  Reserved10;		
			__IO uint8_t  DEVICE_TYPE;		
			__IO uint8_t  DRIVER_STRENGTH; 		
			__IO uint8_t  OUT_OF_INTERRUPT_TIME;		
			__IO uint8_t  PARTITION_SWITCH_TIME;		
			__IO uint8_t  PWR_CL_52_195; 		
			__IO uint8_t  PWR_CL_26_195; 		
			__IO uint8_t  PWR_CL_52_360; 		
			__IO uint8_t  PWR_CL_26_360; 		
			__IO uint8_t  Reserved9;		
			__IO uint8_t  MIN_PERF_R_4_26;		
			__IO uint8_t  MIN_PERF_W_4_26; 		
			__IO uint8_t  MIN_PERF_R_8_26_4_52; 		
			__IO uint8_t  MIN_PERF_W_8_26_4_52; 		
			__IO uint8_t  MIN_PERF_R_8_52; 		
			__IO uint8_t  MIN_PERF_W_8_52; 		
			__IO uint8_t  Reserved8;		
			__IO uint8_t  SEC_COUNT[4];		
			__IO uint8_t  Reserved7;		
			__IO uint8_t  S_A_TIMEOUT;		
			__IO uint8_t  Reserved6;		
			__IO uint8_t  S_C_VCCQ; 		
			__IO uint8_t  S_C_VCC; 		
			__IO uint8_t  HC_WP_GRP_SIZE; 		
			__IO uint8_t  REL_WR_SEC_C; 		
			__IO uint8_t  ERASE_TIMEOUT_MULT; 		
			__IO uint8_t  HC_ERASE_GRP_SIZE; 		
			__IO uint8_t  ACC_SIZE; 		
			__IO uint8_t  BOOT_SIZE_MULTI; 		
			__IO uint8_t  Reserved5;		
			__IO uint8_t  BOOT_INFO; 		
			__IO uint8_t  obsolete2;		
			__IO uint8_t  obsolete1;		
			__IO uint8_t  SEC_FEATURE_SUPPORT; 		
			__IO uint8_t  TRIM_MULT; 		
			__IO uint8_t  Reserved4;		
			__IO uint8_t  MIN_PERF_DDR_R_8_52;		
			__IO uint8_t  MIN_PERF_DDR_W_8_52; 		
			__IO uint8_t  PWR_CL_200_195; 		
			__IO uint8_t  PWR_CL_200_360; 		
			__IO uint8_t  PWR_CL_DDR_52_195; 		
			__IO uint8_t  PWR_CL_DDR_52_360; 		
			__IO uint8_t  Reserved3;		
			__IO uint8_t  INI_TIMEOUT_AP;		
			__IO uint8_t  CORRECTLY_PRG_SECTORS_NUM[4];		
			__IO uint8_t  BKOPS_STATUS[2];		
			__IO uint8_t  POWER_OFF_LONG_TIME;		
			__IO uint8_t  GENERIC_CMD6_TIME; 		
			__IO uint8_t  CACHE_SIZE[4]; 		
			__IO uint8_t  Reserved2[255];		
			__IO uint8_t  EXT_SUPPORT;		
			__IO uint8_t  LARGE_UNIT_SIZE_M1; 		
			__IO uint8_t  CONTEXT_CAPABILITIES; 		
			__IO uint8_t  TAG_RES_SIZE; 		
			__IO uint8_t  TAG_UNIT_SIZE; 		
			__IO uint8_t  DATA_TAG_SUPPORT; 		
			__IO uint8_t  MAX_PACKED_WRITES; 		
			__IO uint8_t  MAX_PACKED_READS; 		
			__IO uint8_t  BKOPS_SUPPORT;		
			__IO uint8_t  HPI_FEATURES; 		
			__IO uint8_t  S_CMD_SET;		
			__IO uint8_t  Reserved1[7];	         
		} EXT_CSD;
   __IO uint8_t CsdBuf[512];
} EMMCEXT_CSD;


/** @defgroup MMC_Exported_Types_Group5 Card Identification Data: CID Register
  * @{
  */
typedef struct
{
  __IO uint8_t  ManufacturerID;  /*!< Manufacturer ID       */
  __IO uint16_t OEM_AppliID;     /*!< OEM/Application ID    */
  __IO uint32_t ProdName1;       /*!< Product Name part1    */
  __IO uint8_t  ProdName2;       /*!< Product Name part2    */
  __IO uint8_t  ProdRev;         /*!< Product Revision      */
  __IO uint32_t ProdSN;          /*!< Product Serial Number */
  __IO uint8_t  Reserved1;       /*!< Reserved1             */
  __IO uint16_t ManufactDate;    /*!< Manufacturing Date    */
  __IO uint8_t  CID_CRC;         /*!< CID CRC               */
  __IO uint8_t  Reserved2;       /*!< Always 1              */

} EMMC_CardCIDTypeDef;

/** 
  * @brief EMMC Card Status 
  */
typedef struct
{
  __IO uint8_t DAT_BUS_WIDTH;
  __IO uint8_t SECURED_MODE;
  __IO uint16_t SD_CARD_TYPE;
  __IO uint32_t SIZE_OF_PROTECTED_AREA;
  __IO uint8_t SPEED_CLASS;
  __IO uint8_t PERFORMANCE_MOVE;
  __IO uint8_t AU_SIZE;
  __IO uint16_t ERASE_SIZE;
  __IO uint8_t ERASE_TIMEOUT;
  __IO uint8_t ERASE_OFFSET;
} EmmcCardStatus;


/** 
  * @brief EMMC Card information 
  */
typedef struct
{
  EMMC_CardCSDTypeDef EmmcCsd;
  // EMMCEXT_CSD EmmcExtCsd; 
  EMMC_CardCIDTypeDef EmmcCid;
  uint64_t CardCapacity;  /*!< Card Capacity */
  uint32_t CardBlockSize; /*!< Card Block Size */
  uint16_t RCA;
  uint8_t CardType;
}EmmcCardInfo;
   
/**
  * @}
  */
#define EMMC_SDIO_DMA_FLAG_FEIF DMA_FLAG_FEIF3
#define EMMC_SDIO_DMA_FLAG_DMEIF DMA_FLAG_DMEIF3
#define EMMC_SDIO_DMA_FLAG_TEIF DMA_FLAG_TEIF3
#define EMMC_SDIO_DMA_FLAG_HTIF DMA_FLAG_HTIF3
#define EMMC_SDIO_DMA_FLAG_TCIF DMA_FLAG_TCIF3

 /** 
  * @brief SDIO Commands  Index 
  */
#define EMMC_CMD_GO_IDLE_STATE                       ((uint8_t)0)
#define EMMC_CMD_SEND_OP_COND                        ((uint8_t)1)
#define EMMC_CMD_ALL_SEND_CID                        ((uint8_t)2)
#define EMMC_CMD_SET_REL_ADDR                        ((uint8_t)3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define EMMC_CMD_SET_DSR                             ((uint8_t)4)
#define EMMC_CMD_SDIO_SEN_OP_COND                    ((uint8_t)5)
#define EMMC_CMD_HS_SWITCH                           ((uint8_t)6)
#define EMMC_CMD_SEL_DESEL_CARD                      ((uint8_t)7)
#define EMMC_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8)
#define EMMC_CMD_SEND_CSD                            ((uint8_t)9)
#define EMMC_CMD_SEND_CID                            ((uint8_t)10)
#define EMMC_CMD_READ_DAT_UNTIL_STOP                 ((uint8_t)11) /*!< SD Card doesn't support it */
#define EMMC_CMD_STOP_TRANSMISSION                   ((uint8_t)12)
#define EMMC_CMD_SEND_STATUS                         ((uint8_t)13)
#define EMMC_CMD_HS_BUSTEST_READ                     ((uint8_t)14)
#define EMMC_CMD_GO_INACTIVE_STATE                   ((uint8_t)15)
#define EMMC_CMD_SET_BLOCKLEN                        ((uint8_t)16)
#define EMMC_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)
#define EMMC_CMD_READ_MULT_BLOCK                     ((uint8_t)18)
#define EMMC_CMD_HS_BUSTEST_WRITE                    ((uint8_t)19)
#define EMMC_CMD_WRITE_DAT_UNTIL_STOP                ((uint8_t)20) /*!< SD Card doesn't support it */
#define EMMC_CMD_SET_BLOCK_COUNT                     ((uint8_t)23) /*!< SD Card doesn't support it */
#define EMMC_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)
#define EMMC_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25)
#define EMMC_CMD_PROG_CID                            ((uint8_t)26) /*!< reserved for manufacturers */
#define EMMC_CMD_PROG_CSD                            ((uint8_t)27)
#define EMMC_CMD_SET_WRITE_PROT                      ((uint8_t)28)
#define EMMC_CMD_CLR_WRITE_PROT                      ((uint8_t)29)
#define EMMC_CMD_SEND_WRITE_PROT                     ((uint8_t)30)
#define EMMC_CMD_ERASE_GRP_START                     ((uint8_t)35) /*!< To set the address of the first write block to be erased.
                                                                  (For MMC card only spec 3.31) */

#define EMMC_CMD_ERASE_GRP_END                       ((uint8_t)36) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For MMC card only spec 3.31) */

#define EMMC_CMD_ERASE                               ((uint8_t)38)
#define EMMC_CMD_FAST_IO                             ((uint8_t)39) /*!< SD Card doesn't support it */
#define EMMC_CMD_GO_IRQ_STATE                        ((uint8_t)40) /*!< SD Card doesn't support it */
#define EMMC_CMD_LOCK_UNLOCK                         ((uint8_t)42)
#define EMMC_CMD_APP_CMD                             ((uint8_t)55)
#define EMMC_CMD_GEN_CMD                             ((uint8_t)56)
#define EMMC_CMD_NO_CMD                              ((uint8_t)64)

/** 
  * @brief Supported SD Memory Cards 
  */
#define SDMMC_STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define SDMMC_STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define SDMMC_HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define SDMMC_MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SDMMC_SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define SDMMC_HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SDMMC_SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define SDMMC_HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)

/** 
  * @brief  Mask for errors Card Status R1 (OCR Register) 
  */
#define EMMC_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define EMMC_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define EMMC_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define EMMC_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define EMMC_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define EMMC_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define EMMC_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define EMMC_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define EMMC_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define EMMC_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define EMMC_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define EMMC_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define EMMC_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define EMMC_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define EMMC_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define HOCR_WP_ERASE_SKIP                ((uint32_t)0x00008000)
#define EMMC_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define EMMC_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define EMMC_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define EMMC_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/** 
  * @brief  Masks for R6 Response 
  */
#define EMMC_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define EMMC_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define EMMC_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define EMMC_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define EMMC_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define EMMC_STD_CAPACITY                 ((uint32_t)0x00000000)
#define EMMC_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define EMMC_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define EMMC_ALLZERO                      ((uint32_t)0x00000000)

#define EMMC_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define EMMC_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define EMMC_CARD_LOCKED                  ((uint32_t)0x02000000)

#define EMMC_DATATIMEOUT                  ((uint32_t)0xFFFFFFFF)
#define EMMC_0TO7BITS                     ((uint32_t)0x000000FF)
#define EMMC_8TO15BITS                    ((uint32_t)0x0000FF00)
#define EMMC_16TO23BITS                   ((uint32_t)0x00FF0000)
#define EMMC_24TO31BITS                   ((uint32_t)0xFF000000)
#define EMMC_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define EMMC_HALFFIFO                     ((uint32_t)0x00000008)
#define EMMC_HALFFIFOBYTES                ((uint32_t)0x00000020)

/** 
  * @brief  Command Class Supported 
  */
#define EMMC_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define EMMC_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define EMMC_CCCC_ERASE                   ((uint32_t)0x00000020)

 
/**
  * @}
  */ 

#define EMMC_OCR_REG             0x40FF8080
#define EMMC_POWER_REG           0x03BB0800
#define EMMC_HIGHSPEED_REG       0x03B90100
#define EMMC_4BIT_REG            0x03B70100
#define EMMC_8BIT_REG            0x03B70200

/** @defgroup SDIO_EMMC_Exported_Functions
  * @{
  */ 
// void EMMC_IRQHandler(void);
// EmmcError EmmcWaitReadOperation(void);
// EmmcError EmmcWaitWriteOperation(void);

extern uint32_t EmmcInit(void);
extern HAL_StatusTypeDef SdmmcPowerON(void);
extern uint32_t EmmcGoReadyState(void);
extern uint32_t EmmcPowerOFF(void);
// extern uint32_t EmmcInitializeCards(EmmcCardInfo *E);
// extern uint32_t EmmcGetCardInfo(EmmcCardInfo *E, uint32_t *CSD_Tab, uint32_t *CID_Tab, uint16_t Rca);
extern uint32_t EmmcEnWideBus(FunctionalState NewState);
extern uint32_t EmmcEnableWideBusOperation(uint32_t WideMode);
extern uint32_t EmmcSelectDeselect(uint32_t addr);
// EmmcError EmmcReadExtCsd(EmmcCardInfo *E);
extern uint32_t EMMC_GetCardCID(uint32_t *CID_Tab, EMMC_CardCIDTypeDef *pCID);
extern uint32_t EMMC_GetCardCSD(uint32_t *CSD_Tab, EMMC_CardCSDTypeDef *pCSD);

extern EmmcTransferState EmmcGetTransferState(void);
extern uint32_t EmmcStopTransfer(void);
extern uint32_t EmmcSendStatus(uint32_t *pcardstatus);
extern EmmcCardState EmmcGetState(void);
extern EmmcTransferState EmmcGetStatus(void);

extern uint32_t EMMC_ReadBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);
extern uint32_t EmmcReadBlocksDMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);
extern uint32_t EMMC_WriteBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);
extern uint32_t EmmcWriteBlocksDMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);

extern uint32_t Emmc_ReadData(uint8_t * pData, uint32_t len, uint32_t address);
extern uint32_t Emmc_WriteData(uint8_t * pData, uint32_t len, uint32_t address);

extern EmmcCardInfo MyEmmcCardInfo;

#define EMMC_OPERATION_TIMEOUT (30000)

#define SIZE_DEFINE_1KB (1024u)
#define SIZE_DEFINE_1MB (1024u * 1024u)
#define SIZE_DEFINE_1GB (1024u * 1024u * 1024u)
#define EMMC_SECTOR_ADDRESS_ALIGNMENT_MASK (0x1ffu)
#define EMMC_PHYSICAL_SECTOR_SIZE_BYTE (512)
#define EMMC_SECTOR_SIZE_BYTE (512)
#define EMMC_BLOCK_SIZE_BYTE (512)

#if 1
#define EMMC_CLUST_SIZE_BYTE (SIZE_DEFINE_1KB * 64)
#define EMMC_MEMORY_SIZE_BYTE (SIZE_DEFINE_1GB * 3      \
                              + 512 * SIZE_DEFINE_1MB   \
                              + 128 * SIZE_DEFINE_1MB   \
                              + 16 * SIZE_DEFINE_1MB)
#define EMMC_RESERVE_MEMORY_SIZE_BYTE (SIZE_DEFINE_1MB * 64)
#else
// For test
#define EMMC_CLUST_SIZE_BYTE (512)
#define EMMC_MEMORY_SIZE_BYTE (SIZE_DEFINE_1MB * 256 * 8)
#define EMMC_RESERVE_MEMORY_SIZE_BYTE (SIZE_DEFINE_1MB * 16)
#endif

#define EMMC_MEMORY_FS_SIZE_BYTE (EMMC_MEMORY_SIZE_BYTE - EMMC_RESERVE_MEMORY_SIZE_BYTE)

#define EMMC_SECTOR_COUNT (EMMC_MEMORY_FS_SIZE_BYTE / EMMC_SECTOR_SIZE_BYTE)

#define EMMC_RESERVE_MEMORY_START_ADDR (EMMC_MEMORY_SIZE_BYTE - EMMC_RESERVE_MEMORY_SIZE_BYTE)
#define EMMC_FIRMWARE_INFO_START_ADDR (EMMC_RESERVE_MEMORY_START_ADDR)
#define EMMC_FIRMWARE_START_ADDR ((EMMC_FIRMWARE_INFO_START_ADDR + EMMC_PHYSICAL_SECTOR_SIZE_BYTE) & (~EMMC_SECTOR_ADDRESS_ALIGNMENT_MASK))
#define EMMC_FIRMWARE_MAX_SIZE (512 * 1024)

/**
  * @}
  */ 
#ifdef __cplusplus
}
#endif

#endif /* __SDIO_EMMC_H */
