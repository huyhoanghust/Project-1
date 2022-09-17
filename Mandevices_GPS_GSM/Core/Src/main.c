/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sim800c.h"
#include "l70.h"
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
// char data[100];
bool SOSFlag = false;
char simNumber[20] = "0987035442";//0921342634
char latData[15] = {0};
char longData[15] = {0};
char simMessage[200] = {0};
char *GPSResponse;
char gps[500];

int count = 0;
char x;
int index1 = 0;
uint16_t ADC_value;

uint32_t Timesleepcurrent = 0;
uint32_t Timesleepstart   = 0;
bool flagsleep= false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  Timesleepstart = Timesleepcurrent;
  if(huart == &huart2)
  {
    sim800c_callback();
  }
  if(huart == &huart3)
  {
    l70_callback();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  Timesleepstart = Timesleepcurrent;
  if(htim == &htim2)
  {
    sim800c_timerCallback();
  }
  //timer1 == 3s
  if(htim == &htim1)
  {
    // wakeup GPS
    //l70_init();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_TIM_Base_Stop_IT(&htim1);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  Timesleepstart = Timesleepcurrent;
  //wakeup
  // l70_sleep();
  if (flagsleep == true)
  {
    SystemClock_Config();
    HAL_ResumeTick();
    HAL_PWR_DisableSleepOnExit();
    // wakeup GPS
    //l70_init();
    // wakeup SIM
    //sim800c_wakeup();
  }
  //read ADC pin
  if (GPIO_Pin == BUT_ADC_Pin)
  {
    ADC_value = HAL_ADC_GetValue(&hadc1);
    //>4.1V
    if (ADC_value > 3134) 
    {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    }
    //3.7-4.1V  
    if (ADC_value <= 3134 && ADC_value > 2828)    
    {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    }
    //3.3->3.7V
    if (ADC_value <= 2828 && ADC_value > 2522)
    {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    }
    //0->3.3V
    if (ADC_value <= 2522)
    {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    }
    // if (flagsleep == true)
    // {
    //   SystemClock_Config();
    //   HAL_ResumeTick();
    //   HAL_PWR_DisableSleepOnExit();
    //   //wakeup GPS
    //   l70_init();
    //   //wakeup SIM
    //   sim800c_wakeup();
    // }
    //l70_sleep();
  }
  if (GPIO_Pin == BUT_SOS_Pin)
  {
    //l70_init();
    SOSFlag = true;
    HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, GPIO_PIN_SET);
  }
  HAL_TIM_Base_Start_IT(&htim1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint32_t timeStart = 0;
  uint32_t timeCurrent = 0;
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
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start(&hadc1);
  timeStart = HAL_GetTick();
  // while(HAL_GPIO_ReadPin(GSM_STATUS_GPIO_Port, GSM_STATUS_Pin) == GPIO_PIN_RESET)
  // {
  //   HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin, SET);
  //   HAL_Delay(1000);
  // }
  //sim800c_init();
  sim800c_sleep();
  HAL_Delay(100);
  l70_init();
  HAL_Delay(1000);
  // do
  // {
  //   GPSResponse = l70_receiveGPS();
  //   l70_handleGPS(latData, longData, GPSResponse);
  //   HAL_Delay(1000);
  // } while (((strstr(latData, ".000000") != NULL) && (strstr(longData, ".000000") != NULL)));

  // HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, SET);

  GPSResponse = l70_receiveGPS();
  l70_handleGPS(latData, longData, GPSResponse);
  sprintf(simMessage, "https://www.google.com/maps/search/?api=1&query=%s,%s\n", latData, longData);
  HAL_UART_Transmit(&huart1, (uint8_t*)simMessage, 100, 200);
  memset(simMessage, '\0', sizeof(simMessage));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // if(SOSFlag == true)
    // {
    //   timeCurrent = HAL_GetTick();
    //   if(timeCurrent - timeStart > 2000)
    //   {
    //     timeStart = timeCurrent;

    //     HAL_UART_Transmit(&huart1, (uint8_t*)"Bat dau gui tin nhan\r\n", strlen("Bat dau gui tin nhan\r\n"), 100);
    //     GPSResponse = l70_receiveGPS();
    //     //HAL_UART_Transmit(&huart1, (uint8_t*)GPSResponse, strlen(GPSResponse), 100);
    //     l70_handleGPS(latData, longData, GPSResponse);
    //     sprintf(simMessage, "https://www.google.com/maps/search/?api=1&query=%s,%s\n", latData, longData);
    //     HAL_UART_Transmit(&huart1, (uint8_t*)simMessage, 100, 200);
    //     //sim800c_sendSMS(latData, longData, simNumber, simMessage);
    //     memset(simMessage, '\0', sizeof(simMessage));
    //     SOSFlag = false;
    //     HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, GPIO_PIN_RESET);
    //   }
    //   else
    //   {
    //     HAL_UART_Transmit(&huart1, (uint8_t*)"Doi mot luc\r\n", strlen("Doi mot luc\r\n"), 100);
    //     SOSFlag = false;
    //   }
    //   Timesleepstart = HAL_GetTick();
    // }

    Timesleepcurrent = HAL_GetTick();
    if(Timesleepcurrent - Timesleepstart > 10000)
    {
      flagsleep = true;
      //l70_sleep();
      // sim800c_sleep();
      
      //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
      /****** Suspend the Ticks before entering the STOP mode or else this can wake the device up **********/
      HAL_SuspendTick();
      HAL_PWR_EnableSleepOnExit();
      /* Enter Stop Mode */
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    }
    HAL_Delay(1000);
    // timeCurrent = HAL_GetTick();
    //   if(timeCurrent - timeStart > 1000)
    //   {
    //     timeStart = timeCurrent;
    //     GPSResponse = l70_receiveGPS();
    //     l70_handleGPS(latData, longData, GPSResponse);
    //     sprintf(simMessage, "%s  %s\n", latData, longData);
    //     int len = strlen(simMessage);
    //     memcpy(gps + len*count, simMessage, len);
    //     count++;
    //     if (count == 10)
    //     {
    //       HAL_UART_Transmit(&huart1, (uint8_t*)gps, strlen(gps), 1000);
    //       count = 0;
    //     }
    //     memset(simMessage, '\0', sizeof(simMessage));
    //   }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
