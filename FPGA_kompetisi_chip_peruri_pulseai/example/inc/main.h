/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f4xx_hal.h"

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
#define USER_OUTPUT1_Pin GPIO_PIN_2
#define USER_OUTPUT1_GPIO_Port GPIOA
#define USER_OUTPUT2_Pin GPIO_PIN_3
#define USER_OUTPUT2_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define SPI1_RST_Pin GPIO_PIN_4
#define SPI1_RST_GPIO_Port GPIOC
#define SPI1_DC_Pin GPIO_PIN_5
#define SPI1_DC_GPIO_Port GPIOC
#define USER_LED_Pin GPIO_PIN_2
#define USER_LED_GPIO_Port GPIOB
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define SPI2_IRQ_Pin GPIO_PIN_6
#define SPI2_IRQ_GPIO_Port GPIOC
#define USB_POWER_Pin GPIO_PIN_7
#define USB_POWER_GPIO_Port GPIOC
#define SDIO_DET_Pin GPIO_PIN_8
#define SDIO_DET_GPIO_Port GPIOA
#define SPI3_IRQ_Pin GPIO_PIN_15
#define SPI3_IRQ_GPIO_Port GPIOA
#define SPI3_CS_Pin GPIO_PIN_6
#define SPI3_CS_GPIO_Port GPIOB
#define SPI3_RST_Pin GPIO_PIN_7
#define SPI3_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#ifndef SPI1_RST_Pin
#define SPI1_RST_Pin       SPI3_RST_Pin
#define SPI1_RST_GPIO_Port SPI3_RST_GPIO_Port
#endif
#ifndef SPI1_DC_Pin
#define SPI1_DC_Pin        SPI3_DC_Pin
#define SPI1_DC_GPIO_Port  SPI3_DC_GPIO_Port
#endif
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
