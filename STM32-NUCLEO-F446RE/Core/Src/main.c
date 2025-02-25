#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

UART_HandleTypeDef huart4;
GPIO_InitTypeDef GPIO_InitStruct = {0};

void SystemClock_Config(void);

static void MX_GPIO_Init(void);

static void MX_UART4_Init(void);

void UART_SendString(char *str);

volatile uint8_t rx_flag = 0;
char rx_buffer[50];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART4) {
        rx_flag = 1;
        HAL_UART_Receive_IT(&huart4, (uint8_t *) rx_buffer, sizeof(rx_buffer));
    }
}

DMA_HandleTypeDef htim1;
#define DHT11_PORT TEMP_SENSOR_GPIO_Port
#define DHT11_PIN TEMP_SENSOR_Pin
uint8_t RHI, RHD, TCI, TCD, SUM;
uint32_t pMillis, cMillis;
float tCelsius = 0;
float tFahrenheit = 0;
float RH = 0;
uint8_t TFI = 0;
uint8_t TFD = 0;
char strCopy[15];

void microDelay(uint16_t delay) {
    __HAL_DMA_SET_COUNTER(&htim1, 0);
    while (__HAL_DMA_GET_COUNTER(&htim1) < delay);
    //printf("%d ticks \n",__HAL_TIM_GET_COUNTER(&htim1));
}

uint8_t DHT11_Start(void) {
    uint8_t Response = 0;
    GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
    GPIO_InitStructPrivate.Pin = DHT11_PIN;
    GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // set the pin as output
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 0);   // pull the pin low
    HAL_Delay(20);   // wait for 20ms
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 1);   // pull the pin high
    microDelay(30);   // wait for 30us
    GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructPrivate.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // set the pin as input
    microDelay(40);
    if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))) {
        microDelay(80);
        if ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))) Response = 1;
    }
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis) {
        cMillis = HAL_GetTick();
    }
    return Response;
}

uint8_t DHT11_Read(void) {
    uint8_t a, b;
    for (a = 0; a < 8; a++) {
        pMillis = HAL_GetTick();
        cMillis = HAL_GetTick();
        while (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis) {  // wait for the pin to go high
            cMillis = HAL_GetTick();
        }
        microDelay(40);   // wait for 40 us
        if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))   // if the pin is low
            b &= ~(1 << (7 - a));
        else
            b |= (1 << (7 - a));
        pMillis = HAL_GetTick();
        cMillis = HAL_GetTick();
        while ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis) {  // wait for the pin to go low
            cMillis = HAL_GetTick();
        }
    }
    return b;
}


int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_UART4_Init();

    UART_SendString("[DEBUG] STM32 USART6 Initialized!\n");

    char rx_buffer[1], uart_buf[120], error_buf[50];

    float RH2 = 0.0, Temp2 = 0.0;
    int rgbr2 = 255, rgbg2 = 255, rgbb2 = 255;
    bool rgb2 = false, led2 = false, buz2 = false, but2 = false;

    HAL_UART_Receive_IT(&huart4, (uint8_t *) rx_buffer, sizeof(rx_buffer));

    while (1) {

        if (DHT11_Start()) {
            RHI = DHT11_Read();
            RHD = DHT11_Read();
            TCI = DHT11_Read();
            TCD = DHT11_Read();
            SUM = DHT11_Read();
            if (RHI + RHD + TCI + TCD == SUM) {
                tCelsius = (float) TCI + (float) (TCD / 10.0);
                Temp2 = tCelsius;
                RH = (float) RHI + (float) (RHD / 10.0);
                RH2 = RH;

                tFahrenheit = tCelsius * 9 / 5 + 32;
                TFI = tFahrenheit;
                TFD = tFahrenheit * 10 - TFI * 10;
                printf("%d.%d C   ", TCI, TCD);
                printf("%d.%d F   ", TFI, TFD);
            }
        }

        uint8_t buffer[200];
        HAL_UART_Receive(&huart4, buffer, sizeof(buffer), HAL_MAX_DELAY);
        if (sizeof(buffer) != 200) {
            sprintf(
                    uart_buf,
                    "recue:%s %d\n",
                    buffer, sizeof(buffer)
            );
            UART_SendString(uart_buf);
            if (sizeof(buffer) > 0) {
                if (strncmp(buffer, "[LED#SWITCH:True]", 6) == 0) {
                    led2 = true;
                    UART_SendString("LED activée\n");
                } else if (strncmp(buffer, "[LED#SWITCH:False]", 7) == 0) {
                    led2 = false;
                    UART_SendString("LED désactivée\n");
                } else if (strncmp(buffer, "BUZZER ON", 9) == 0) {
                    buz2 = true;
                    UART_SendString("Buzzer activé\n");
                } else if (strncmp(buffer, "BUZZER OFF", 10) == 0) {
                    buz2 = false;
                    UART_SendString("Buzzer désactivé\n");
                } else if (strncmp(buffer, "RGB ", 4) == 0) {
                    sscanf(buffer, "RGB R%d G%d B%d", &rgbr2, &rgbg2, &rgbb2);
                    UART_SendString("Couleur RGB mise à jour\n");
                } else { UART_SendString("Commande inconnue\n"); }
                memset(buffer, 0, sizeof(buffer));
            }
        }

        float RH = RH2, Temp = Temp2;
        int rgbr = rgbr2, rgbg = rgbg2, rgbb = rgbb2;
        bool rgb = rgb2, led = led2, buz = buz2, but = but2;

        sprintf(
                uart_buf,
                "{\"Temperature\":%.2f,\"Humidity\":%.2f,\"RGB\":{\"red\":%d,\"green\":%d,\"blue\":%d,\"state\":%s},\"Led\":%s,\"Buzzer\":%s,\"Button\":%s}\n",
                Temp, RH, rgbr, rgbg, rgbb, rgb ? "true" : "false", led ? "true" : "false", buz ? "true" : "false",
                but ? "true" : "false"
        );
        UART_SendString(uart_buf);

        HAL_Delay(2000);

        if (HAL_UART_Receive(&huart4, (uint8_t *) rx_buffer, 1, 100) == HAL_OK) {
            HAL_UART_Transmit(&huart4, (uint8_t *) rx_buffer, 1, HAL_MAX_DELAY);
        }

    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) { Error_Handler(); }
}

static void MX_UART4_Init(void) {
    huart4.Instance = UART4;
    huart4.Init.BaudRate = 115200;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart4) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void) {

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    //__HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TEMP_SENSOR_GPIO_Port, TEMP_SENSOR_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : RGBBLUE_Pin */
    GPIO_InitStruct.Pin = RGBBLUE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(RGBBLUE_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : RGBTED_Pin */
    GPIO_InitStruct.Pin = RGBTED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(RGBTED_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : RGBGREEN_Pin */
    GPIO_InitStruct.Pin = RGBGREEN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(RGBGREEN_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : PB3 */
    //GPIO_InitStruct.Pin = GPIO_PIN_3;
    //GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    //GPIO_InitStruct.Pull = GPIO_NOPULL;
    //HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : PB5 */
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);

}

void UART_SendString(char *str) { HAL_UART_Transmit(&huart4, (uint8_t *) str, strlen(str), HAL_MAX_DELAY); }

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line){}
#endif
