/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through 
        * the Code Generation settings)
     PB2   ------> LPTIM1_OUT
     PC6   ------> SDMMC1_D6
     PC7   ------> SDMMC1_D7
     PC8   ------> SDMMC1_D0
     PC9   ------> SDMMC1_D1
     PC10   ------> SDMMC1_D2
     PC11   ------> SDMMC1_D3
     PC12   ------> SDMMC1_CK
     PD2   ------> SDMMC1_CMD
     PB8   ------> SDMMC1_D4
     PB9   ------> SDMMC1_D5
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, PH0_MCU_PH0_Pin|PH1_WIFI_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, PA2_3V7_PWR_EN_Pin/*|PA3_eMMC_PWR_EN_Pin|PA4_SPI3_NSS_Pin|PA5_WIFI_EN_Pin 
                          |PA7_WAKE_BLE_Pin*/|PA8_eMMC_RST_Pin/*|PA15_GPIO0_Pin*/, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, PA3_eMMC_PWR_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, /*PB12_BLE_CS_Pin|*/PB14_WIFI_WAKE_Pin|PB15_CAN0_ENn_Pin|PB6_CAN_PWR_EN_Pin 
                          |PB7_WIFI_SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  // GPIO_InitStruct.Pin = PC13_WIFI_IRQ_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // HAL_GPIO_Init(PC13_WIFI_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PHPin PHPin */
  GPIO_InitStruct.Pin = PH0_MCU_PH0_Pin|PH1_WIFI_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3 
                           PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin PAPin 
                           PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = PA2_3V7_PWR_EN_Pin|PA3_eMMC_PWR_EN_Pin|PA4_SPI3_NSS_Pin|PA5_WIFI_EN_Pin 
                          /*|PA7_WAKE_BLE_Pin*/|PA8_eMMC_RST_Pin|PA15_GPIO0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  // GPIO_InitStruct.Pin = PA6_ACC_IN2_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // HAL_GPIO_Init(PA6_ACC_IN2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin */
  // GPIO_InitStruct.Pin = PB0_BLE_IRQ_Pin|PB1_ACC_IRQ_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  // GPIO_InitStruct.Pin = PB2_eMMC_RCLK_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // GPIO_InitStruct.Alternate = GPIO_AF1_LPTIM1;
  // HAL_GPIO_Init(PB2_eMMC_RCLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB13 PB3 
                           PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin PBPin PBPin 
                           PBPin */
  GPIO_InitStruct.Pin = /*PB12_BLE_CS_Pin|*/PB14_WIFI_WAKE_Pin|PB15_CAN0_ENn_Pin|PB6_CAN_PWR_EN_Pin 
                          |PB7_WIFI_SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // /*Configure GPIO pins : PCPin PCPin PCPin PCPin 
  //                          PCPin PCPin PCPin */
  // GPIO_InitStruct.Pin = PC6_eMMC_DAT6_Pin|PC7_eMMC_DAT7_Pin|PC8_eMMC_DAT0_Pin|PC9_eMMC_DAT1_Pin 
  //                         |PC10_eMMC_DAT2_Pin|PC11_eMMC_DAT3_Pin|PC12_eMMC_CLK_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  // GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  // HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA9 PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // /*Configure GPIO pin : PtPin */
  // GPIO_InitStruct.Pin = PD2_eMMC_CMD_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  // GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  // HAL_GPIO_Init(PD2_eMMC_CMD_GPIO_Port, &GPIO_InitStruct);

  // /*Configure GPIO pins : PBPin PBPin */
  // GPIO_InitStruct.Pin = PB8_eMMC_DAT4_Pin|PB9_eMMC_DAT5_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  // GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  // HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  // HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  // HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  // HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  // HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
