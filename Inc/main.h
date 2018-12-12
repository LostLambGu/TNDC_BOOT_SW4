/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define PC13_WIFI_IRQ_Pin GPIO_PIN_13
#define PC13_WIFI_IRQ_GPIO_Port GPIOC
#define PC13_WIFI_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define PH0_MCU_PH0_Pin GPIO_PIN_0
#define PH0_MCU_PH0_GPIO_Port GPIOH
#define PH1_WIFI_RESET_Pin GPIO_PIN_1
#define PH1_WIFI_RESET_GPIO_Port GPIOH
#define PA0_MCU_TX4_Pin GPIO_PIN_0
#define PA0_MCU_TX4_GPIO_Port GPIOA
#define PA1_MCU_RX4_Pin GPIO_PIN_1
#define PA1_MCU_RX4_GPIO_Port GPIOA
#define PA2_3V7_PWR_EN_Pin GPIO_PIN_2
#define PA2_3V7_PWR_EN_GPIO_Port GPIOA
#define PA3_eMMC_PWR_EN_Pin GPIO_PIN_3
#define PA3_eMMC_PWR_EN_GPIO_Port GPIOA
#define PA4_SPI3_NSS_Pin GPIO_PIN_4
#define PA4_SPI3_NSS_GPIO_Port GPIOA
#define PA5_WIFI_EN_Pin GPIO_PIN_5
#define PA5_WIFI_EN_GPIO_Port GPIOA
#define PA6_ACC_IN2_Pin GPIO_PIN_6
#define PA6_ACC_IN2_GPIO_Port GPIOA
#define PA6_ACC_IN2_EXTI_IRQn EXTI9_5_IRQn
#define PA7_WAKE_BLE_Pin GPIO_PIN_7
#define PA7_WAKE_BLE_GPIO_Port GPIOA
#define PB0_BLE_IRQ_Pin GPIO_PIN_0
#define PB0_BLE_IRQ_GPIO_Port GPIOB
#define PB0_BLE_IRQ_EXTI_IRQn EXTI0_IRQn
#define PB1_ACC_IRQ_Pin GPIO_PIN_1
#define PB1_ACC_IRQ_GPIO_Port GPIOB
#define PB1_ACC_IRQ_EXTI_IRQn EXTI1_IRQn
#define PB2_eMMC_RCLK_Pin GPIO_PIN_2
#define PB2_eMMC_RCLK_GPIO_Port GPIOB
#define PB12_BLE_CS_Pin GPIO_PIN_12
#define PB12_BLE_CS_GPIO_Port GPIOB
#define PB14_WIFI_WAKE_Pin GPIO_PIN_14
#define PB14_WIFI_WAKE_GPIO_Port GPIOB
#define PB15_CAN0_ENn_Pin GPIO_PIN_15
#define PB15_CAN0_ENn_GPIO_Port GPIOB
#define PC6_eMMC_DAT6_Pin GPIO_PIN_6
#define PC6_eMMC_DAT6_GPIO_Port GPIOC
#define PC7_eMMC_DAT7_Pin GPIO_PIN_7
#define PC7_eMMC_DAT7_GPIO_Port GPIOC
#define PC8_eMMC_DAT0_Pin GPIO_PIN_8
#define PC8_eMMC_DAT0_GPIO_Port GPIOC
#define PC9_eMMC_DAT1_Pin GPIO_PIN_9
#define PC9_eMMC_DAT1_GPIO_Port GPIOC
#define PA8_eMMC_RST_Pin GPIO_PIN_8
#define PA8_eMMC_RST_GPIO_Port GPIOA
#define PA15_GPIO0_Pin GPIO_PIN_15
#define PA15_GPIO0_GPIO_Port GPIOA
#define PC10_eMMC_DAT2_Pin GPIO_PIN_10
#define PC10_eMMC_DAT2_GPIO_Port GPIOC
#define PC11_eMMC_DAT3_Pin GPIO_PIN_11
#define PC11_eMMC_DAT3_GPIO_Port GPIOC
#define PC12_eMMC_CLK_Pin GPIO_PIN_12
#define PC12_eMMC_CLK_GPIO_Port GPIOC
#define PD2_eMMC_CMD_Pin GPIO_PIN_2
#define PD2_eMMC_CMD_GPIO_Port GPIOD
#define PB6_CAN_PWR_EN_Pin GPIO_PIN_6
#define PB6_CAN_PWR_EN_GPIO_Port GPIOB
#define PB7_WIFI_SPI1_CS_Pin GPIO_PIN_7
#define PB7_WIFI_SPI1_CS_GPIO_Port GPIOB
#define PB8_eMMC_DAT4_Pin GPIO_PIN_8
#define PB8_eMMC_DAT4_GPIO_Port GPIOB
#define PB9_eMMC_DAT5_Pin GPIO_PIN_9
#define PB9_eMMC_DAT5_GPIO_Port GPIOB

#define ENABLE_HARDWARE_IWDG (0)

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
 #define USE_FULL_ASSERT    1U 

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
