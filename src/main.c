/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dmp_app.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t mpu_int_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  char uart_buf[128];
  HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n=== STM32 UART & I2C Test ===\r\n", 33, 100);

  /* Scan alamat I2C 7-bit (0x01 .. 0x7E) */
  uint8_t found_count = 0;
  for (uint8_t addr = 1; addr < 127; addr++)
  {
    /* HAL meminta alamat 8-bit (7-bit << 1) */
    if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 2, 20) == HAL_OK)
    {
      int len = snprintf(uart_buf, sizeof(uart_buf), "I2C device found at 0x%02X\r\n", addr);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, (uint16_t)len, 100);
      found_count++;
    }
  }

  if (found_count == 0)
  {
    HAL_UART_Transmit(&huart1, (uint8_t *)"No I2C device found! Check VCC/GND/SDA/SCL/pull-up.\r\n", 54, 100);
  }
  else
  {
    /* Coba baca WHO_AM_I (register 0x75) di alamat 0x68 */
    uint8_t who_am_i = 0;
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(&hi2c1, (0x68 << 1), 0x75, I2C_MEMADD_SIZE_8BIT, &who_am_i, 1, 100);
    if (st == HAL_OK)
    {
      int len = snprintf(uart_buf, sizeof(uart_buf), "MPU6050 WHO_AM_I = 0x%02X\r\n", who_am_i);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, (uint16_t)len, 100);
    }
    else
    {
      HAL_UART_Transmit(&huart1, (uint8_t *)"Failed to read WHO_AM_I at 0x68\r\n", 34, 100);
    }
  }

  /* Inisialisasi DMP 6-Axis */
  bool dmp_ok = DMP_App_Init();
  uint32_t last_print_tick = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
    if (dmp_ok && DMP_App_ReadYPR(&yaw, &pitch, &roll))
    {
      if ((HAL_GetTick() - last_print_tick) >= 100U)
      {
        last_print_tick = HAL_GetTick();

        int y_i = (int)yaw;
        int y_d = abs((int)((yaw - (float)y_i) * 100.0f));

        int p_i = (int)pitch;
        int p_d = abs((int)((pitch - (float)p_i) * 100.0f));

        int r_i = (int)roll;
        int r_d = abs((int)((roll - (float)r_i) * 100.0f));

        int len = snprintf(
          uart_buf,
          sizeof(uart_buf),
          "YPR (deg): Y=%d.%02d  P=%d.%02d  R=%d.%02d\r\n",
          y_i, y_d, p_i, p_d, r_i, r_d
        );
        HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, (uint16_t)len, 100);
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0)
  {
    mpu_int_flag = 1;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */