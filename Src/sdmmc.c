/**
  ******************************************************************************
  * File Name          : SDMMC.c
  * Description        : This file provides code for the configuration
  *                      of the SDMMC instances.
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
#include "sdmmc.h"

#include "gpio.h"

DMA_HandleTypeDef hdma_sdmmc1;

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* SDMMC1 init function */
void MX_SDMMC1_Init(void)
{

}


void SDMMC1_DeInit(void)
{
  /* Set Power State to OFF */
  SDMMC_PowerState_OFF(SDMMC1);
  
  /* De-Initialize the MSP layer */
  SDMMC1_MspDeInit();
}

void SDMMC1_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* USER CODE BEGIN SDMMC1_MspInit 0 */

  /* USER CODE END SDMMC1_MspInit 0 */
    /* SDMMC1 clock enable */
    __HAL_RCC_SDMMC1_CLK_ENABLE();

    /**SDMMC1 GPIO Configuration    
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
    /*Configure GPIO pins : PCPin PCPin PCPin PCPin 
                           PCPin PCPin PCPin */
    GPIO_InitStruct.Pin = PC6_eMMC_DAT6_Pin | PC7_eMMC_DAT7_Pin | PC8_eMMC_DAT0_Pin | PC9_eMMC_DAT1_Pin | 
                          PC10_eMMC_DAT2_Pin | PC11_eMMC_DAT3_Pin | PC12_eMMC_CLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = PD2_eMMC_CMD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(PD2_eMMC_CMD_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin */
    GPIO_InitStruct.Pin = PB8_eMMC_DAT4_Pin | PB9_eMMC_DAT5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SDMMC1 DMA Init */
    /* SDMMC1_RX Init */
    // hdma_sdmmc1_rx.Instance = DMA2_Channel4;
    // hdma_sdmmc1_rx.Init.Request = DMA_REQUEST_7;
    // hdma_sdmmc1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    // hdma_sdmmc1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    // hdma_sdmmc1_rx.Init.MemInc = DMA_MINC_ENABLE;
    // hdma_sdmmc1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    // hdma_sdmmc1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    // hdma_sdmmc1_rx.Init.Mode = DMA_NORMAL;
    // hdma_sdmmc1_rx.Init.Priority = DMA_PRIORITY_LOW;
    // if (HAL_DMA_Init(&hdma_sdmmc1_rx) != HAL_OK)
    // {
    //   _Error_Handler(__FILE__, __LINE__);
    // }

    /* SDMMC1_TX Init */
    // hdma_sdmmc1_tx.Instance = DMA2_Channel5;
    // hdma_sdmmc1_tx.Init.Request = DMA_REQUEST_7;
    // hdma_sdmmc1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    // hdma_sdmmc1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    // hdma_sdmmc1_tx.Init.MemInc = DMA_MINC_ENABLE;
    // hdma_sdmmc1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    // hdma_sdmmc1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    // hdma_sdmmc1_tx.Init.Mode = DMA_NORMAL;
    // hdma_sdmmc1_tx.Init.Priority = DMA_PRIORITY_LOW;
    // if (HAL_DMA_Init(&hdma_sdmmc1_tx) != HAL_OK)
    // {
    //   _Error_Handler(__FILE__, __LINE__);
    // }

    // /* SDMMC1 interrupt Init */
    // HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
    // HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
  /* USER CODE BEGIN SDMMC1_MspInit 1 */

  /* USER CODE END SDMMC1_MspInit 1 */
}

void SDMMC1_MspDeInit(void)
{
  /* USER CODE BEGIN SDMMC1_MspDeInit 0 */

  /* USER CODE END SDMMC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC1_CLK_DISABLE();
  
    /**SDMMC1 GPIO Configuration    
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
    HAL_GPIO_DeInit(GPIOC, PC6_eMMC_DAT6_Pin | PC7_eMMC_DAT7_Pin | PC8_eMMC_DAT0_Pin | PC9_eMMC_DAT1_Pin | 
                            PC10_eMMC_DAT2_Pin | PC11_eMMC_DAT3_Pin | PC12_eMMC_CLK_Pin);

    HAL_GPIO_DeInit(PD2_eMMC_CMD_GPIO_Port, PD2_eMMC_CMD_Pin);

    /* SDMMC1 DMA DeInit */
    // HAL_DMA_DeInit(&hdma_sdmmc1_rx);
    // HAL_DMA_DeInit(&hdma_sdmmc1_tx);
    HAL_DMA_DeInit(&hdma_sdmmc1);

    // /* SDMMC1 interrupt Deinit */
    // HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
  /* USER CODE BEGIN SDMMC1_MspDeInit 1 */

  /* USER CODE END SDMMC1_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
