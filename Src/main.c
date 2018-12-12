/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_hal.h"
#include "dma.h"
#include "iwdg.h"
#include "rtc.h"
#include "sdmmc.h"
#include "usart.h"
#include "gpio.h"

#include "ymodem.h"

#include "flash.h"
#include "initialization.h"
#include "uart_api.h"

#include "string.h"

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{
  uint32_t FlashFlagValue = 0;
  uint8_t StrBuf[12] = {'\0'};
  uint8_t ymodemdowmload = 0;
  uint8_t chmodemdowmload = 0;

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UART4_Init();
  // MX_SDMMC1_Init();
  // MX_RTC_Init();
  #if ENABLE_HARDWARE_IWDG
  MX_IWDG_Init();
  #endif

  SystemInitialization();

  FlashFlagValue = ((FirmwareInfoTypeDef *)IAP_FLAG)->IAPFlag;
  memset(StrBuf, 0, sizeof(StrBuf));
  Int2Str(StrBuf, FlashFlagValue);
  // Show Flag value
  Serial_PutString("\r\nBootloader 1.1 start...");
  Serial_PutString("\r\nFlag:");
  Serial_PutString(StrBuf);
  Serial_PutString("\r\nSize:");
  memset(StrBuf, 0, sizeof(StrBuf));
  Int2Str(StrBuf, ((FirmwareInfoTypeDef *)IAP_FLAG)->FirmwareSize);
  Serial_PutString(StrBuf);

  HAL_Delay(1000);

  if (Uart4RxCount > 0)
  {
    if (NULL != strstr((char *)Uart4RxBuffer, "AT*BOOTYMODEM?"))
    {
      ymodemdowmload = 1;
    }
    else if (NULL != strstr((char *)Uart4RxBuffer, "AT*BOOTCHMODEM?"))
    {
      chmodemdowmload = 1;
    }
  }

  /* Infinite loop */
  //Test if user code is programmed starting from address "APPLICATION_ADDRESS"
  if (((*(__IO uint32_t *)JUMP_TO_APPLICATION_ADDRESS) & 0x2FFE0000) == 0x20000000 
  && (FlashFlagValue != (uint32_t)IAP_BOOT_FLASH) 
  && (FlashFlagValue != (uint32_t)IAP_BOOT_UART) 
  && (FlashFlagValue != (uint32_t)IAP_BOOT_CHMODEM) 
  // && ((FlashFlagValue == (uint32_t)IAP_APP_FLASH) 
  // || (FlashFlagValue == (uint32_t)IAP_APP_UART) 
  // || (FlashFlagValue == (uint32_t)IAP_APP_CHMODEM))
  && (ymodemdowmload == 0) && (chmodemdowmload == 0))
  {
    Serial_PutString("\r\nJump to application...");
    Serial_PutString("\r\n\r\n");
    HAL_Delay(1);
    // Jump to user application
    SoftwareJumping(JUMP_TO_APPLICATION_ADDRESS);
  }
  else
  {
    /* Unlock the Flash Program Erase controller */
    FLASH_If_Init();

    /* Test if any sector of Flash memory where user application will be loaded is write protected */
    // if (FLASH_If_GetWriteProtectionStatus() != 0)
    // {
    //   extern FLASH_ProcessTypeDef pFlash;
    //   /* Unlock the Program memory */
    //   __HAL_UNLOCK(&pFlash);
    // }

    /* Check Flag */
    if (FlashFlagValue == (uint32_t)IAP_BOOT_FLASH)
    {
      UpdateFromEmmc((FirmwareInfoTypeDef *)IAP_FLAG);
    }

    if ((FlashFlagValue == (uint32_t)IAP_BOOT_UART) || (ymodemdowmload == 1))
    {
      Serial_PutString("\r\nPlease Update App Through Ymodem...");
      // while (1)
      {
        /* Download user application in the Flash */
        SerialDownload();
      }
    }
    
    Serial_PutString("\r\nPlease Update App Through Chmodem...");
    // while(1)
    {
      // Update Through Chmodem
      extern int ChmodemReceiveLoadFile(void);
      ChmodemReceiveLoadFile();
    }

    NVIC_SystemReset();
  }

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure LSE Drive Capability 
    */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_SDMMC1;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV4;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
