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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ESP32_I2C_ADDR (0x28U << 1) //Dirección del ESP32 pero desplazada un bit a la izquierda 

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//INTRODUCCIÓN DEL MENÚ
uint8_t inicio=0;
uint8_t intro[]="Escoger una de las opciones:\r\n";
uint8_t opcion1[]="1. Controlar dispostivo SPI\r\n";
uint8_t opcion2[]="2. Obtener mediciones de sensor I2C\r\n";
uint8_t respuesta[1];
uint8_t res1[]="Usted ha escogido controlar el dispositivo SPI\r\n";
uint8_t res2[]="Usted ha escogido obtener las mediciones del sensor I2C\r\n";
uint8_t error[]="Error! Prueba otra vez!\r\n";

//OPCIÓN 1
uint8_t intro2[]="En esta parte debe enviar numeros enteros. No incluya dimensionales y separe sus respuesta con coma. Por ejemplo: '1,500'.\r\n";
uint8_t pregLED[]="Escoja el LED que desea encender: verde (1), amarillo (2), rojo (3)\r\n";
uint8_t pregTIEM[]="Escoja la cantidad de tiempo que desea encender el LED en milisegundos\r\n";

char buffer[16];
uint8_t txSPI[4];
uint8_t rxSPI[4];
long ms, led;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM6_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
bool esNumero(const char *s);
uint8_t leerLinea(char *buf, uint8_t max);
static void enviarTexto(const char *texto);
static void medirPotenciometro(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Comunicación con el I2C
static void enviarTexto(const char *texto){
  HAL_UART_Transmit(&huart2, (uint8_t *)texto, (uint16_t)strlen(texto), 100);
  	  //*texto = puntero, dónde empieza el texto
  	  //strlen mide cuántos caracteres tiene 
}

static void medirPotenciometro(void){
  uint8_t comando = 'M';
  uint8_t respuesta[2] = {0, 0};
  uint16_t lectura;
  char mensaje[80];

  /* Solicitar una nueva medicion al ESP32. */
  if (HAL_I2C_Master_Transmit(&hi2c1, ESP32_I2C_ADDR, &comando, 1, 100) != HAL_OK){ //Nucleo manda M y si no está bien = error
    enviarTexto("\r\nError: el ESP32 no recibio la solicitud I2C.\r\n");
    return;
  }

  /* Esperar a que el ESP32 lo lea y de la respuesta  */
  HAL_Delay(20);

  if (HAL_I2C_Master_Receive(&hi2c1, ESP32_I2C_ADDR, respuesta, 2, 100) != HAL_OK)  { //Pide 2 bytes y los guarda en respuesta
    enviarTexto("\r\nError: no se recibio la medicion I2C.\r\n");
    return;
  }

  lectura = ((uint16_t)respuesta[0] << 8) | respuesta[1]; //Une 2 bytes en un solo número 

  //Seguridad, no más de 4095
  if (lectura > 4095U)  {
    enviarTexto("\r\nError: medicion fuera del rango del ADC.\r\n");
    return;
  }

  //Armar texto
  snprintf(mensaje,
           sizeof(mensaje),
           "\r\nPot1: %u (ADC 0-4095); valor LCD: %u (0-255)\r\n",
           (unsigned int)lectura,
           (unsigned int)(lectura / 16U));

  enviarTexto(mensaje);
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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(inicio==0){
		  //Introducción
		  HAL_UART_Transmit(&huart2, intro, sizeof(intro), 1000);
		  HAL_UART_Transmit(&huart2, opcion1, sizeof(opcion1), 1000);
		  HAL_UART_Transmit(&huart2, opcion2, sizeof(opcion2), 1000);
		  HAL_UART_Receive(&huart2, respuesta, 1, HAL_MAX_DELAY);

		  //Recepción de respuesta
		  //Opción 1
		  if(respuesta[0]=='1'){
			  HAL_UART_Transmit(&huart2, res1, sizeof(res1), 1000);
			  inicio=1;

			  HAL_UART_Transmit(&huart2, intro2, sizeof(intro2), 1000);
			  HAL_UART_Transmit(&huart2, pregLED, sizeof(pregLED), 1000);
			  HAL_UART_Transmit(&huart2, pregTIEM, sizeof(pregTIEM), 1000);

			  //Verificar que la respuesta sea válida
			  leerLinea(buffer, sizeof(buffer));

			  // Validar
			  bool valido = false;
			  char *t1 = strtok(buffer, ","); //STRTOK corta en pedazos
			  char *t2 = strtok(NULL, ","); //NULL = mismo texto
			  if (t1 && t2 && esNumero(t1) && esNumero(t2)) { //¿Existen ambos pedazos? ¿Ambos son números?
			      led = atol(t1);
			      ms  = atol(t2);
			      valido = (led >= 1 && led <= 3 && ms > 0 && ms <= 65535); //Valor de tiempo entre 0 y 65535 (max de 2 bytes)
			  }

			  if (valido) {
			  //[LED, ms_alto, ms_bajo, 0]
			      txSPI[0] = (uint8_t)led;
			      txSPI[1] = (uint8_t)(ms >> 8); //alto
			      txSPI[2] = (uint8_t)(ms & 0xFF); //bajo
			      txSPI[3] = 0; //relleno

			      HAL_GPIO_WritePin(ESP32_CS_GPIO_Port, ESP32_CS_Pin, GPIO_PIN_RESET);
			      HAL_SPI_TransmitReceive(&hspi1, txSPI, rxSPI, 4, 1000);
			      HAL_GPIO_WritePin(ESP32_CS_GPIO_Port, ESP32_CS_Pin, GPIO_PIN_SET);

			   //Generar señal cuadrada
			      HAL_TIM_Base_Stop_IT(&htim6);
			      __HAL_TIM_SET_AUTORELOAD(&htim6, ms-1); //Da 1000 pulsos por segundo 
			      __HAL_TIM_SET_COUNTER(&htim6, 0); //Reinicia cuenta en 0
			      HAL_TIM_GenerateEvent(&htim6, TIM_EVENTSOURCE_UPDATE); //Carga nuevo valor y borra alarma
			      __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);
			      HAL_GPIO_WritePin(LEDnucleo_GPIO_Port, LEDnucleo_Pin, GPIO_PIN_RESET);
			      HAL_TIM_Base_Start_IT(&htim6);

			  }

			  else {
				  HAL_UART_Transmit(&huart2, error, sizeof(error), 1000);
				      // Volver al menú
			  }
			  inicio = 0;
		  }

		  //Opción 2
		  else if(respuesta[0]=='2'){
			  HAL_UART_Transmit(&huart2, res2, sizeof(res2), 1000);
			  inicio=1;
			  medirPotenciometro();
		  }

		  //Opción inválida
		  else{
			  HAL_UART_Transmit(&huart2, error, sizeof(error), 1000);
		  }
		  inicio=0;


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
  RCC_OscInitStruct.PLL.PLLN = 96;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 48000-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 500-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDnucleo_GPIO_Port, LEDnucleo_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ESP32_CS_GPIO_Port, ESP32_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LEDnucleo_Pin */
  GPIO_InitStruct.Pin = LEDnucleo_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDnucleo_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ESP32_CS_Pin */
  GPIO_InitStruct.Pin = ESP32_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(ESP32_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//Lee texto que recive carácter por carácter
uint8_t leerLinea(char *buf, uint8_t max) {
    uint8_t c, idx = 0; //c=carácter que acaba de llegar, idx=cuántos caracteres llevamos
    uint8_t crlf[] = "\r\n";

    while (1) {
        // Espera hasta 1s por el siguiente carácter
        HAL_StatusTypeDef st = HAL_UART_Receive(&huart2, &c, 1, 1000); //¿Llegó algo? ¿Se acabó el tiempo?

        if (st == HAL_OK) {
            if (c == '\r' || c == '\n') {
                if (idx > 0) break;      // terminó con Enter
                continue;                // ignora Enter sobrante
            }
            HAL_UART_Transmit(&huart2, &c, 1, 100); //eco: caracter normal -> manda a la terminal
            if (idx < max - 1) buf[idx++] = c;
        }
        else {
            // Timeout: si ya hay datos, damos la línea por terminada
            if (idx > 0) break;
        }
    }

    buf[idx] = '\0'; //Final del texto
    HAL_UART_Transmit(&huart2, crlf, 2, 100);
    return idx;
}

//Revisar que sean dígitos
bool esNumero(const char *s) { 
    if (*s == '\0') return false;
    while (*s) { if (!isdigit((unsigned char)*s)) return false; s++; }
    return true;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim==&htim6){
		HAL_GPIO_TogglePin(LEDnucleo_GPIO_Port, LEDnucleo_Pin);
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
