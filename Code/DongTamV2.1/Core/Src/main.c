/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

#include "BoardParameter.h"
#include "MessageHandle.h"

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
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart3_rx;

/* USER CODE BEGIN PV */

FlagGroup_t fUART,fMesg,f1;
int SetVan,ClearVan;
char uartEsp32Buffer[MAX_MESSAGE],uartLogBuffer[MAX_MESSAGE];
uint16_t uartEsp32RxSize,uartLogRxSize;
UART_HandleTypeDef *uartTarget;
char mesgRX[MAX_MESSAGE],mesgTX[MAX_MESSAGE];
MesgValRX mesgRxRet;
uint16_t timerArray[2];
char outputStr[50];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void SetUp();
void ProcedureVan();
HAL_StatusTypeDef GetUartMessage(char *outputStr);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const char *TAG = "MAIN";

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C1){

	}
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if(huart->Instance == USART1){
		HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t*)uartEsp32Buffer, MAX_MESSAGE);
		uartEsp32RxSize = Size;
		SETFLAG(fUART,FLAG_UART_ESP_RX_DONE);
	}
	if(huart->Instance == USART3){
		HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t*)uartLogBuffer, MAX_MESSAGE);
		uartLogRxSize = Size;
		SETFLAG(fUART,FLAG_UART_LOG_RX_DONE);
	}
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2){
		VanProcedure state = Brd_GetVanProcState();
		uint32_t tArray;
		switch(state){
			case BRD_PULSE_TIME:
				tArray = Brd_GetTimerArray(0);
				tArray++;
				Brd_SetTimerArray(0, tArray);
			break;
			case BRD_INTERVAL_TIME:
				tArray = Brd_GetTimerArray(1);
				tArray++;
				Brd_SetTimerArray(1, tArray);
			break;
			case BRD_CYCLE_INTERVAL_TIME:
//				tArray = Brd_GetTimerArray(2);
//				tArray++;
//				Brd_SetTimerArray(2, tArray);
			break;
			default:

			break;
		}
	}
}



extern BoardParameter brdParam;
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  SetUp();
  HAL_GPIO_WritePin(UserLED_GPIO_Port, UserLED_Pin, 1);
  HAL_TIM_Base_Start_IT(&htim2);
  uartTarget = &huart3;
  while(!uartTarget);
  brdParam.cycIntvTime = 2;
  brdParam.intervalTime = 10;
  brdParam.pulseTime = 60;
  Brd_SetTotalVan(2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(!GetUartMessage(mesgRX)){
		  mesgRxRet = MessageRxHandle(mesgRX,mesgTX);
		  if((HAL_StatusTypeDef)mesgRxRet != HAL_OK) {
			  HAL_UART_Transmit(uartTarget,(uint8_t*) mesgTX, strlen(mesgTX), HAL_MAX_DELAY);
			  char *s = "Invalid message, do nothing\n";
			  HAL_UART_Transmit(uartTarget,(uint8_t*) s, strlen(s), HAL_MAX_DELAY);
		  }
		  else HAL_UART_Transmit(uartTarget, (uint8_t*) mesgTX, strlen(mesgTX), HAL_MAX_DELAY);
	  }
	  ProcedureTriggerVan(outputStr);
	  if(Brd_GetVanProcState() == BRD_PULSE_TIME){
		  // log pressure
		  HAL_UART_Transmit(uartTarget,(uint8_t*)outputStr, strlen(outputStr), HAL_MAX_DELAY);
	  }
	  if(Brd_GetVanProcState() == BRD_INTERVAL_TIME && Brd_GetHC165State()){
		  // log VanState
		  HAL_UART_Transmit(uartTarget,(uint8_t*)outputStr, strlen(outputStr), HAL_MAX_DELAY);
		  Brd_SetHC165State(false);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, _74HC595_CLK_Pin|_74HC595_DATA_Pin|_74HC165_LOAD_Pin|OE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, _74HC595_STORE_Pin|UserLED_Pin|_74HC165_CLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : _74HC595_CLK_Pin _74HC595_DATA_Pin _74HC165_LOAD_Pin OE_Pin */
  GPIO_InitStruct.Pin = _74HC595_CLK_Pin|_74HC595_DATA_Pin|_74HC165_LOAD_Pin|OE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : _74HC595_STORE_Pin UserLED_Pin _74HC165_CLK_Pin */
  GPIO_InitStruct.Pin = _74HC595_STORE_Pin|UserLED_Pin|_74HC165_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : _74HC165_DATA_Pin */
  GPIO_InitStruct.Pin = _74HC165_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(_74HC165_DATA_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */





/**
 * @brief Nếu có chuỗi nhận được từ port UART (ESP hoặc máy tính), copy chuỗi nhận được ra outputStr và reset bộ đệm
 * @param outputStr mảng chứa thông tin nhận được từ UART
 * @return
 */
HAL_StatusTypeDef GetUartMessage(char *outputStr)
{
	if(CHECKFLAG(fUART,FLAG_UART_ESP_RX_DONE)){
		uartTarget = &huart1;
		strcpy(mesgRX,uartEsp32Buffer);
		memset(uartEsp32Buffer,0,uartEsp32RxSize);
		CLEARFLAG(fUART,FLAG_UART_ESP_RX_DONE);
		return HAL_OK;
	}
	if(CHECKFLAG(fUART,FLAG_UART_LOG_RX_DONE)){
		uartTarget = &huart3;
		strcpy(mesgRX,uartLogBuffer);
		memset(uartLogBuffer,0,uartLogRxSize);
		CLEARFLAG(fUART,FLAG_UART_LOG_RX_DONE);
		return HAL_OK;
	}
	return HAL_ERROR;
}



void UartIdle_Init()
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)uartEsp32Buffer, MAX_MESSAGE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, (uint8_t*)uartLogBuffer, MAX_MESSAGE);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx,DMA_IT_HT);
}

void hc595_SetUp()
{
	HC595* p595 = Brd_GetAddress_HC595();
	HC595_AssignPin(p595, GPIOA, GPIO_PIN_5, HC595_CLK);
	HC595_AssignPin(p595, GPIOA, GPIO_PIN_7, HC595_DS);
	HC595_AssignPin(p595, GPIOB, GPIO_PIN_0, HC595_LATCH);
	HC595_AssignPin(p595, GPIOA, GPIO_PIN_12,HC595_OE);
	HC595_SetTarget(p595);

	HC595_ClearByteOutput(0xffffffff);
	HC595_ShiftOut(NULL, 2, 1);
	HC595_EnableOutput();
}

void hc165_SetUp()
{
	HC165* p165 = Brd_GetAddress_HC165();
	HC165_AssignPin(p165, GPIOA, GPIO_PIN_11, HC165_PL);
	HC165_AssignPin(p165, GPIOB, GPIO_PIN_3, HC165_DATA);
	HC165_AssignPin(p165, GPIOA, GPIO_PIN_12, HC165_CE);
	HC165_AssignPin(p165, GPIOB, GPIO_PIN_4, HC165_CP);
}

void SetUp()
{
	AMS5915_Init(Brd_GetAddress_AMS5915(),&hi2c1);
	PCF8563_Init(Brd_GetAddress_PCF8563(),&hi2c1);
	PCF8563_StartClock();
	PCF8563_CLKOUT_SetFreq(CLKOUT_1_Hz);
	PCF8563_CLKOUT_Enable(1);
	UartIdle_Init();
	hc165_SetUp();
	hc595_SetUp();
	HAL_UART_Transmit(&huart1, (uint8_t*)"Hello ESP32\n", strlen("Hello ESP32\n"), HAL_MAX_DELAY);
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
