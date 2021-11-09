/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_EN_Pin GPIO_PIN_1
#define LCD_EN_GPIO_Port GPIOE
#define LCD_D7_Pin GPIO_PIN_3
#define LCD_D7_GPIO_Port GPIOE
#define LCD_D6_Pin GPIO_PIN_4
#define LCD_D6_GPIO_Port GPIOE
#define LCD_D5_Pin GPIO_PIN_5
#define LCD_D5_GPIO_Port GPIOE
#define LCD_D4_Pin GPIO_PIN_6
#define LCD_D4_GPIO_Port GPIOE
#define LCD_LED_Pin GPIO_PIN_13
#define LCD_LED_GPIO_Port GPIOC
#define POT_VOL_Pin GPIO_PIN_3
#define POT_VOL_GPIO_Port GPIOA
#define POT_VOL_HP_Pin GPIO_PIN_6
#define POT_VOL_HP_GPIO_Port GPIOA
#define DAC_RST_Pin GPIO_PIN_5
#define DAC_RST_GPIO_Port GPIOC
#define BT_VEN_Pin GPIO_PIN_9
#define BT_VEN_GPIO_Port GPIOE
#define BT_LED2_Pin GPIO_PIN_10
#define BT_LED2_GPIO_Port GPIOE
#define BT_LED1_Pin GPIO_PIN_11
#define BT_LED1_GPIO_Port GPIOE
#define BT_LED0_Pin GPIO_PIN_12
#define BT_LED0_GPIO_Port GPIOE
#define BT_PIO6_Pin GPIO_PIN_13
#define BT_PIO6_GPIO_Port GPIOE
#define BT_PIO8_Pin GPIO_PIN_14
#define BT_PIO8_GPIO_Port GPIOE
#define BT_RST_Pin GPIO_PIN_15
#define BT_RST_GPIO_Port GPIOE
#define DAC_HP_RST_Pin GPIO_PIN_8
#define DAC_HP_RST_GPIO_Port GPIOD
#define PWR_GOOD_Pin GPIO_PIN_9
#define PWR_GOOD_GPIO_Port GPIOD
#define PWR_EN_Pin GPIO_PIN_10
#define PWR_EN_GPIO_Port GPIOD
#define BUT_CANCEL_Pin GPIO_PIN_3
#define BUT_CANCEL_GPIO_Port GPIOD
#define BUT_ENTER_Pin GPIO_PIN_4
#define BUT_ENTER_GPIO_Port GPIOD
#define BUT_PREV_Pin GPIO_PIN_5
#define BUT_PREV_GPIO_Port GPIOD
#define BUT_NEXT_Pin GPIO_PIN_6
#define BUT_NEXT_GPIO_Port GPIOD
#define BUZZER_Pin GPIO_PIN_9
#define BUZZER_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_2
#define LCD_RS_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
