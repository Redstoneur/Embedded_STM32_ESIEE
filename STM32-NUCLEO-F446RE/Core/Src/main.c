/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DHT11_PORT TEMP_GPIO_Port
#define DHT11_PIN TEMP_Pin

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// Relative Humidity Integral
uint8_t RHI;
// Relative Humidity Decimal
uint8_t RHD;
// Temperature Celsius Integral
uint8_t TCI;
// Temperature Celsius Decimal
uint8_t TCD;
// Checksum
uint8_t SUM;

// Previous and current milliseconds for timing
uint32_t pMillis, cMillis;

// Temperature in Celsius
float tCelsius = 0;
// Temperature in Fahrenheit
float tFahrenheit = 0;
// Relative Humidity
float RH = 0;

// Temperature Fahrenheit Integral
uint8_t TFI = 0;
// Temperature Fahrenheit Decimal
uint8_t TFD = 0;

// String buffer for copying
char strCopy[15];

// RGB LED color values
int rgbr = 255;
int rgbg = 255;
int rgbb = 255;

// States for RGB LED, general LED, buzzer, and button
bool rgb = false, rgb_state = false;
bool led = false, led_state = false;
bool buz = false, buz_state = false;
bool but = false;

// Previous state of temperature threshold
bool temp_old_state = false;

// Temperature threshold value
int temp_threshold = 30;
// Buzzer intensity (0-100)
int buz_intensity = 50;

// Number of buzzer cycles
int buzzer_cycles = 0;

// UART buffers
char uart_buf[200], uart_buf2[200];
// UART receive buffer
uint8_t rx_buffer[1];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

void UART_SendString(char *str);
void Update_RGB_LED(int red, int green, int blue, bool state);
void Update_Buzzer(bool state, int intensity);
void Update_Radiator(bool state);
void Automate_Actions(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void microDelay (uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
	  //printf("%d ticks \n",__HAL_TIM_GET_COUNTER(&htim1));
}

uint8_t DHT11_Start (void)
{
  uint8_t Response = 0;
  GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
  GPIO_InitStructPrivate.Pin = DHT11_PIN;
  GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // set the pin as output
  HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 0);   // pull the pin low
  HAL_Delay(20);   // wait for 20ms
  HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 1);   // pull the pin high
  microDelay (30);   // wait for 30us
  GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructPrivate.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // set the pin as input
  microDelay (40);
  if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))
  {
    microDelay (80);
    if ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN))) Response = 1;
  }
  pMillis = HAL_GetTick();
  cMillis = HAL_GetTick();
  while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
  {
    cMillis = HAL_GetTick();
  }
  return Response;
}

uint8_t DHT11_Read (void)
{
  uint8_t a,b;
  for (a=0;a<8;a++)
  {
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
    {  // wait for the pin to go high
      cMillis = HAL_GetTick();
    }
    microDelay (40);   // wait for 40 us
    if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))   // if the pin is low
      b&= ~(1<<(7-a));
    else
      b|= (1<<(7-a));
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
    {  // wait for the pin to go low
      cMillis = HAL_GetTick();
    }
  }
  return b;
}

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
  MX_UART4_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);
  //HAL_UART_Transmit(&huart2, "Start\n", 28, 1000u);

  UART_SendString("[DEBUG] STM32 USART6 Initialized!\n");
  //HAL_UART_Receive_IT(&huart4, (uint8_t *) rx_buffer, sizeof(rx_buffer));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if (DHT11_Start()) {
          RHI = DHT11_Read(); // Relative humidity integral
          RHD = DHT11_Read(); // Relative humidity decimal
          TCI = DHT11_Read(); // Celsius integral
          TCD = DHT11_Read(); // Celsius decimal
          SUM = DHT11_Read(); // Check sum
          if (RHI + RHD + TCI + TCD == SUM) {
              // Can use RHI and TCI for any purposes if whole number only needed
              tCelsius = (float) TCI + (float) (TCD / 10.0);
              tFahrenheit = tCelsius * 9 / 5 + 32;
              RH = (float) RHI + (float) (RHD / 10.0);
              // Can use tCelsius, tFahrenheit and RH for any purposes
              TFI = tFahrenheit;  // Fahrenheit integral
              TFD = tFahrenheit * 10 - TFI * 10; // Fahrenheit decimal
              printf("%d.%d C   %d.%d F", TCI, TCD, TFI, TFD);
              char *buf = (char *) malloc(sizeof(char) * 100);
              sprintf(buf, "%d.%d C %d.%d %%\n", TCI, TCD, RHI, RHD);
              //HAL_UART_Transmit(&huart2, buf, strlen(buf), 1000u);
          }
      }

      Automate_Actions();

        sprintf(
                uart_buf,
                "{\"Temperature\":%d.%d,\"Humidity\":%d.%d,\"RGB\":{\"red\":%d,\"green\":%d,\"blue\":%d,\"state\":%s},\"Led\":%s,\"Buzzer\":%s,\"Button\":%s,\"TemperatureThreshold\":%d}\n",
                TCI, TCD, // Temperature
                RHI, RHD, // Humidity
                rgbr, rgbg, rgbb, // RGB color
                rgb_state ? "true" : "false", // RGB state
                led_state ? "true" : "false", // Led state
                buz_state ? "true" : "false", // Buzzer state
                but ? "true" : "false", // Button state
                temp_threshold // Temperature threshold
        );
        UART_SendString(uart_buf);

        HAL_Delay(2000);

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RGBBLUE_Pin|TEMP_Pin|RADIATEUR_Pin|BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RGBRED_GPIO_Port, RGBRED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RGBGREEN_Pin|TEMP_SENSOR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RGBBLUE_Pin TEMP_Pin RADIATEUR_Pin BUZZER_Pin */
  GPIO_InitStruct.Pin = RGBBLUE_Pin|TEMP_Pin|RADIATEUR_Pin|BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : RGBRED_Pin */
  GPIO_InitStruct.Pin = RGBRED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RGBRED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RGBGREEN_Pin TEMP_SENSOR_Pin */
  GPIO_InitStruct.Pin = RGBGREEN_Pin|TEMP_SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Transmits a string via UART.
 * @param str The string to be transmitted.
 */
void UART_SendString(char *str) { HAL_UART_Transmit(&huart4, (uint8_t *) str, strlen(str), HAL_MAX_DELAY); }

/**
 * @brief Updates the RGB LED state and color.
 * @param red The red component (0-255).
 * @param green The green component (0-255).
 * @param blue The blue component (0-255).
 * @param state The state of the RGB LED (true for on, false for off).
 */
void Update_RGB_LED(int red, int green, int blue, bool state) {
    if (state) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, red > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, green > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, blue > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    }
}

/**
 * @brief Updates the buzzer state and intensity.
 * @param state The state of the buzzer (true for on, false for off).
 * @param intensity The intensity of the buzzer (0-100).
 */
void Update_Buzzer(bool state, int intensity) {
    if (state) {
        // Convert intensity to a PWM value (0-100% -> 0-255)
        int pwm_value = (intensity * 255) / 100;
        // Configure the timer to generate a PWM signal with the calculated value
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value);
        HAL_GPIO_WritePin(GPIOC, BUZZER_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOC, BUZZER_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Updates the radiator state.
 * @param state The state of the radiator (true for on, false for off).
 */
void Update_Radiator(bool state) {
    if (state) {
        HAL_GPIO_WritePin(GPIOC, RADIATEUR_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOC, RADIATEUR_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief UART receive complete callback.
 * @param huart Pointer to the UART handle.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    static uint16_t uart_buf2_index = 0;

    if (strcmp(rx_buffer, "\n")) {
        uart_buf2[uart_buf2_index++] = rx_buffer[0];
    } else {
        uart_buf2[uart_buf2_index] = '\0'; // Null-terminate the string
        // UART_SendString(uart_buf2);
        if (strncmp((char *) rx_buffer, "[LED#SWITCH:True]", 16) == 0) {
            led = true;
            UART_SendString("[DEBUG] LED activée\n");
        } else if (strncmp((char *) rx_buffer, "[LED#SWITCH:False]", 17) == 0) {
            led = false;
            UART_SendString("[DEBUG] LED désactivée\n");
        } else if (strncmp((char *) rx_buffer, "[BUZZER#SWITCH:True]", 19) == 0) {
            buz = true;
            UART_SendString("[DEBUG] Buzzer activé\n");
        } else if (strncmp((char *) rx_buffer, "[BUZZER#SWITCH:False]", 20) == 0) {
            buz = false;
            UART_SendString("[DEBUG] Buzzer désactivé\n");
        } else if (strncmp((char *) rx_buffer, "[RGB#SWITCH:True]", 16) == 0) {
            rgb = true;
            UART_SendString("[DEBUG] RGB activé\n");
        } else if (strncmp((char *) rx_buffer, "[RGB#SWITCH:False]", 17) == 0) {
            rgb = false;
            UART_SendString("[DEBUG] RGB désactivé\n");
        } else if (strncmp((char *) rx_buffer, "[RGB#COLOR:", 11) == 0) {
            sscanf((char *) rx_buffer, "[RGB#COLOR:%d,%d,%d]", &rgbr, &rgbg, &rgbb);
            UART_SendString("[DEBUG] Couleur RGB mise à jour\n");
        } else if (strncmp((char *) rx_buffer, "[TEMP#THRESHOLD:", 16) == 0) {
            sscanf((char *) rx_buffer, "[TEMP#THRESHOLD:%d]", &temp_threshold);
            UART_SendString("[DEBUG] Seuil de température mis à jour\n");
        } else {
            UART_SendString("[DEBUG] Commande inconnue\n");
        }
        uart_buf2_index = 0; // Reset the index for the next message
    }

    // Restart reception for the next character
    HAL_UART_Receive_IT(&huart4, rx_buffer, 1);
}

/**
 * @brief Automates actions based on temperature and other states.
 */
void Automate_Actions(void) {
    // Turn on the radiator (LED) if the temperature is below the target temperature
    if (temp_old_state != (tCelsius < temp_threshold)) {
        temp_old_state = (tCelsius < temp_threshold);
        buzzer_cycles = 0;
    }
    if (tCelsius < temp_threshold) {
        rgb_state = true;
        led_state = true;
        if (buzzer_cycles < 4) {
            buz_state = true;
            buzzer_cycles++;
        } else {
            buz_state = false;
        }
    } else {
        rgb_state = false;
        led_state = false;
        if (buzzer_cycles < 4) {
            buz_state = true;
            buzzer_cycles++;
        } else {
            buz_state = false;
        }
    }

    if (rgb) {
        rgb_state = true;
        Update_RGB_LED(rgbr, rgbg, rgbb, true);
    } else {
        Update_RGB_LED(rgbr, rgbg, rgbb, rgb_state);
    }

    if (led) {
        led_state = true;
        Update_Radiator(true);
    } else {
        Update_Radiator(led_state);
    }

    if (buz) {
        buz_state = true;
        Update_Buzzer(true, buz_intensity);
    } else {
        Update_Buzzer(buz_state, buz_intensity);
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
