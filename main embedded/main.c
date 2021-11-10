/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "u8x8.h"
#include "u8g2.h"
#include "adxl343.h"
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include "hts221_driver.h"
#include "veml7700_driver.h"
//#include "xlrmodes.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CS_PORT GPIOB
#define CS_PIN GPIO_PIN_5
#define USER_BUTTON_PORT GPIOA
#define USER_BUTTON_PIN GPIO_PIN_5
#define PWM_PERIOD 65535
#define squirtle_width 56
#define squirtle_height 56
#define STOPWATCH_MODE 1
#define TEMPERATURE_MODE 2
#define BATTERY_MODE 3
#define CLOCK_MODE 4
#define VOLUME_MODE 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac_ch1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
int pressed = 5;
int CurrentTimerVal;
int PreviousTimerVal = 0;
double pitchAngle;
double rollAngle;
double pitchThreshold;
double rollThreshold;
int orientation = STOPWATCH_MODE;
int flag = 0;
int currentVolume;
uint8_t pot_addr[1] = {0};
uint8_t pot_res[1] = {64};
volatile uint16_t rxData;
char rxBuffer[255];
int count = 0;
int readCount = 0;
uint32_t val[64];
int zed = 6;
int freq = 4;
int volume = 1;
char accelSend[64];
int accel_buff_len;
float temp;
float humid;
uint32_t lux;
uint16_t tempBuffer[12];
uint16_t humidBuffer[12];
uint32_t luxBuffer[13];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	HAL_UART_Receive_IT(&huart1, (uint8_t*)&rxData, 1);
	rxBuffer[count] = rxData;
	count++;
}

static const unsigned char squirtle_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0c, 0x00,
   0x00, 0x00, 0x00, 0x80, 0xa8, 0x10, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01,
   0x22, 0x00, 0x00, 0x00, 0x00, 0x40, 0x54, 0x24, 0x00, 0x00, 0x00, 0x00,
   0x80, 0x80, 0x4b, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x6a, 0x67, 0x00, 0x00,
   0x00, 0x00, 0xe0, 0x40, 0x6f, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xd2, 0x55,
   0x00, 0x00, 0x00, 0x00, 0x90, 0x82, 0xf5, 0x01, 0x00, 0x00, 0x00, 0x18,
   0xa8, 0x7f, 0x02, 0x00, 0x00, 0x00, 0x76, 0x05, 0x64, 0x0e, 0x00, 0x00,
   0x00, 0xa2, 0xf8, 0x66, 0x0a, 0x00, 0x00, 0x00, 0xcc, 0x0f, 0x42, 0x18,
   0x00, 0x00, 0x00, 0xa4, 0xa1, 0x9d, 0x0c, 0x00, 0x00, 0x00, 0x88, 0xfd,
   0x91, 0x28, 0x00, 0x00, 0x00, 0xa8, 0x00, 0x4a, 0x39, 0x00, 0x00, 0x00,
   0x90, 0xa8, 0x0c, 0x29, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x9a, 0x34, 0x00,
   0x00, 0x00, 0x40, 0xff, 0xfb, 0x6c, 0x00, 0x00, 0x00, 0x40, 0x08, 0x60,
   0x74, 0x00, 0x00, 0x00, 0xc0, 0x08, 0x48, 0x56, 0x00, 0x00, 0x00, 0x40,
   0x09, 0x44, 0x7a, 0x00, 0x00, 0x00, 0x80, 0x12, 0x42, 0x5a, 0x1e, 0x00,
   0x00, 0x80, 0xfc, 0x83, 0x2c, 0x21, 0x00, 0x00, 0x80, 0x11, 0xfc, 0xb9,
   0x44, 0x00, 0x00, 0x40, 0x22, 0x48, 0x53, 0x52, 0x00, 0x00, 0x40, 0xae,
   0x08, 0x47, 0xc0, 0x00, 0x00, 0x40, 0x74, 0xa6, 0x3e, 0x57, 0x00, 0x00,
   0xc0, 0xf4, 0x07, 0x6b, 0x49, 0x00, 0x00, 0x00, 0x69, 0x54, 0xad, 0x63,
   0x00, 0x00, 0x40, 0x20, 0x04, 0xf7, 0x2e, 0x00, 0x00, 0x80, 0x1f, 0x14,
   0x05, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x38, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint32_t wave_32[32] = {
	128, 153, 178, 200, 220, 236, 247, 254, 255, 251, 242, 228, 211, 189, 166,
	140, 115, 89, 66, 44, 27, 13, 4, 0, 1, 8, 19, 35, 55, 77,
	102, 127
};

uint32_t wave_32_temp[32] = {
	128, 153, 178, 200, 220, 236, 247, 254, 255, 251, 242, 228, 211, 189, 166,
	140, 115, 89, 66, 44, 27, 13, 4, 0, 1, 8, 19, 35, 55, 77,
	102, 127
};

uint32_t wave_64[64] = {
	128, 140, 153, 165, 177, 188, 199, 209, 219, 227, 235, 241, 246, 250, 253,
	255, 255, 254, 252, 248, 244, 238, 231, 223, 214, 204, 194, 183, 171, 159,
	147, 134, 121, 108, 96, 84, 72, 61, 51, 41, 32, 24, 17, 11, 7,
	3, 1, 0, 0, 2, 5, 9, 14, 20, 28, 36, 46, 56, 67, 78,
	90, 102, 115, 127
};

uint8_t byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {

        case U8X8_MSG_BYTE_INIT:
            /* disable chipselect */
            HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
            break;

        case U8X8_MSG_BYTE_SEND:
            HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 0xFF);
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
            HAL_Delay(1);
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            HAL_Delay(1);
            HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
            break;

        case U8X8_MSG_BYTE_SET_DC:
            break;

        default:
            return 0;
    }
    return 1;
}

uint8_t GPIO_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {

        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;

        case U8X8_MSG_DELAY_MILLI:
            HAL_Delay(arg_int);
            break;

        case U8X8_MSG_DELAY_NANO:
            __asm__("nop");
            break;

        default:
            return 0;
    }
    return 1;
}

void generate_sinwave(int zed)
{
	for(int i = 0; i < 64; i++) {
		val[i] = (sin((5) * i * 2 * M_PI/64)*zed/2);
	}
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const char writeBuf[64];
char readBuf[64];
char xread[8] = {0};
char yread[8] = {0};
char zread[8] = {0};
char pitchread[8] = {0};
char rollread[8] = {0};
char volread[8] = {0};
char segread[8] = {0};
char xlrangle[8] = {0};
char volTimeRead[8] = {0};
//uint16_t devAddr = (0x50 << 1);
//uint16_t memAddr = 0;
volatile uint16_t rxData;
char flagPrint [1] = {0};
int volumeLevel [13] = {
		0, 20, 40, 60,
		80, 100, 120, 140,
		160, 180, 200 , 240,
		260
};

int currentAccel = 0;
int savedAccel = 0;
int saveVol = 127;
//uint32_t flashData[] = {};
uint16_t rxMsg[255];
__IO uint32_t rx_data[2];
double pitchAngle;
double rollAngle;
double volAngle;
double volCircle;
int volSeg, previousSeg, currentSeg;
int volMode = 0;
char modeName[6] = {0};
uint32_t currentTime = 0;
uint32_t previousTime = 0;
uint32_t currentVolTime = 0;
int volTime = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int32_t CH1_DC = 0;

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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_DAC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  adxl_init();
  veml7700_init();
  hts221_init();

  u8g2_t u8g2;
  u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, byte_cb, GPIO_and_delay_cb);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
  u8g2_ClearBuffer(&u8g2);
//  u8g2_DrawBitmap(&u8g2, 0, 0, 128/8, 64, code_pic1);
//  ug82_DrawXBM(&u8g2, 0, 0, 128/8, 64,);
//  u8g2_DrawXBM(&u8g2, 0, 0, squirtle_width, squirtle_height, squirtle_bits);
  u8g2_SetFontMode(&u8g2, 1);
  u8g2_SetFontDirection(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_artossans8_8u);
//  u8g2_DrawUTF8(&u8g2, 45, 28, "T");
//  u8g2_DrawUTF8(&u8g2, 55, 28, "E");
//  u8g2_DrawUTF8(&u8g2, 65, 28, "S");
//  u8g2_DrawUTF8(&u8g2, 75, 28, "T");
//  u8g2_DrawUTF8(&u8g2, 75, 10, "T");
//  u8g2_DrawUTF8(&u8g2, 85, 10, "1");
//  u8g2_DrawUTF8(&u8g2, 95, 10, "7");
//  u8g2_DrawUTF8(&u8g2, 60, 32, "");

  u8g2_SendBuffer(&u8g2);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);


  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  generate_sinwave(50);
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, wave_32_temp, 32, DAC_ALIGN_8B_R);
  //HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, wave_64, 64, DAC_ALIGN_8B_R);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&pot_addr, 1, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&pot_res, 1, 100);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
  while (1)
  {
	  int PWM_SCALE = 5;
	  calc_x_y_z_values();
	  accel_buff_len = sprintf(accelSend, "accel_x:%d\naccel_y:%d\naccel_z:%d\n", x, y, z);
	  HAL_UART_Transmit(&huart1, (uint8_t*)accelSend, accel_buff_len, 100);
	  if(rxBuffer[count - 1] == 'a') {
		  readCount = count;
		  const char wait[] = "HELLO\n";
		  HAL_UART_Transmit(&huart1, (uint8_t*)wait, sizeof(wait), 100);
	  }

    // // Determine whether bmp has been sent
    // // Find index of bmp prefix 'p' and assume all following characters until newline
    // // are bmp data
    // char *bmpBeginPtr = strstr(rxBuffer, "png_begin");
    // if (bmpBeginPtr != NULL) {
    //   bmpBeginPtr += 9;
    //   char *bmpEndPtr = strstr(bmpBeginPtr, "png_end");
    //   if (bmpEndPtr != NULL) {
    //     int length = bmpBeginPtr - bmpEndPtr;

    //     // Check that data is correct size and values
    //     if (length == 128*64) {
    //       int bmpData[length];
    //       int validData = 1;

    //       // Iterate through data, assigning char values ('0', '1') to int array (0, 1)
    //       // Continue assigning values until error (non (1,0) value)
    //       for (int i = 0; i < length && validData == 1; i++) {
    //         bmpData[i] = *(bmpBeginPtr + i) - '0';
    //         if (bmpData[i] != 0 && bmpData[i] != 1) {
    //           validData = 0;
    //         }
    //       }

    //       if (validData) {
    //         // Draw bmp data to LCD
    //         u8g2_DrawXBM(&u8g2, 0, 0, 128, 64, bmpData);

    //         // Set start of buffer to null ("clear" buffer)
    //         buffer[0] = '\0';
    //       }          
    //     }
    //   }
    // }

	  lux = veml7700_read_als();
	sprintf(luxBuffer, "light:%06lu\n", lux);
	HAL_UART_Transmit(&huart1, (uint8_t*)luxBuffer, sizeof(luxBuffer), 100);
	HAL_Delay(50);

	temp = hts221_read_temp();
	sprintf(tempBuffer, "temp:%.2f\n", temp);
	HAL_UART_Transmit(&huart1, (uint8_t*)tempBuffer, sizeof(tempBuffer), 100);
	HAL_Delay(50);

	humid = hts221_read_humid();
	sprintf(humidBuffer, "humid:%.2f\n", humid);
	HAL_UART_Transmit(&huart1, (uint8_t*)humidBuffer, sizeof(humidBuffer), 100);
	HAL_Delay(50);

//	if (pressed != 0) {
//		if (pressed == 1){
//			PWM_SCALE = 5;
//			while (CH1_DC < PWM_PERIOD/PWM_SCALE)
//			{
//				TIM1->CCR1 = CH1_DC;
//				CH1_DC += 100;
//				HAL_Delay(1);
//			}
//		}
//		if (pressed == 2){
//			PWM_SCALE = 5/2;
//			while (CH1_DC < PWM_PERIOD/PWM_SCALE)
//			{
//				TIM1->CCR1 = CH1_DC;
//				CH1_DC += 100;
//				HAL_Delay(1);
//			}
//		}
//		if (pressed == 3){
//			PWM_SCALE = 5/3;
//			while (CH1_DC < PWM_PERIOD/PWM_SCALE)
//			{
//				TIM1->CCR1 = CH1_DC;
//				CH1_DC += 100;
//				HAL_Delay(1);
//			}
//		}
//		if (pressed == 4){
//			PWM_SCALE = 5/4;
//			while (CH1_DC < PWM_PERIOD/PWM_SCALE)
//			{
//				TIM1->CCR1 = CH1_DC;
//				CH1_DC += 100;
//				HAL_Delay(1);
//			}
//		}
//		if (pressed == 5){
//			while (CH1_DC < PWM_PERIOD)
//			{
//				TIM1->CCR1 = CH1_DC;
//				CH1_DC += 100;
//				HAL_Delay(1);
//			}
//		}
//	}
//	else if (pressed == 0){
//			while (CH1_DC > 0)
//			{
//			TIM1->CCR1 = CH1_DC;
//			CH1_DC -= 200;
//			HAL_Delay(1);
//			}
//
//	}
	xlrmodes_angle_conversion();
	xlrmodes_switch();
	HAL_Delay(20);
	u8g2_ClearBuffer(&u8g2);

	sprintf(xread, "%d", x);
	sprintf(yread, "%d", y);
	sprintf(zread, "%d", z);
	sprintf (flagPrint, "%d", flag);
	sprintf(pitchread, "%f", pitchAngle);
	sprintf(rollread, "%f", rollAngle);
	sprintf(volread, "%f", volAngle);
	sprintf(segread, "%d", currentSeg);

//	sprintf(volTimeRead, "%d", volTime);

	u8g2_DrawUTF8(&u8g2, 110, 10, "X=");
	u8g2_DrawUTF8(&u8g2, 100, 10, "Y=");
	u8g2_DrawUTF8(&u8g2, 90, 10, "Z=");
	u8g2_DrawUTF8(&u8g2, 70, 10, "VT:");
	u8g2_DrawUTF8(&u8g2, 60, 10, "VA:");
	u8g2_DrawUTF8(&u8g2, 110, 30, xread);
	u8g2_DrawUTF8(&u8g2, 100, 30, yread);
	u8g2_DrawUTF8(&u8g2, 90, 30, zread);
//	u8g2_DrawUTF8(&u8g2, 70, 30, volTimeRead);
	u8g2_DrawUTF8(&u8g2, 60, 30, volread);
	u8g2_DrawUTF8(&u8g2, 50, 10, modeName);
	u8g2_DrawUTF8(&u8g2, 40, 10, segread);
//	u8g2_DrawUTF8(&u8g2, 30, 10, segread);


	u8g2_SendBuffer(&u8g2);
//	HAL_Delay(200);
//	u8g2_DrawBox(&u8g2, 40, 36, 64, 10);
//	u8g2_SendBuffer(&u8g2);
//  	while(CH1_DC < 65535)
//  	{
//  	    TIM1->CCR1 = CH1_DC;
//  	    CH1_DC += 70;
//  	    HAL_Delay(1);
//  	}
//  	while(CH1_DC > 0)
//  	{
//  	    TIM1->CCR1 = CH1_DC;
//  	    CH1_DC -= 70;
//  	    HAL_Delay(1);
//  	}
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */
  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

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
  hi2c1.Init.Timing = 0x00707CBB;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
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
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
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
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 624;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  HAL_UART_Receive_IT(&huart1, (uint8_t*)&rxData, 1);
  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void xlrmodes_angle_conversion(){

	pitchAngle = atan(x / sqrt(y * y + z * z)) * 180.0 / M_PI;
	rollAngle = atan(y / sqrt(x * x + z * z)) * 180.0 / M_PI;
	volAngle = (atan2(-y, -x) + M_PI)  * 180.0 / M_PI;
	volCircle = atan2(-y, -x) + M_PI;

}

void xlrmodes_switch(){
	if(volTime == 0) {
		currentVolTime  = HAL_GetTick();
	}

	switch (orientation)
	{
	case STOPWATCH_MODE:
		strcpy(modeName, "STOP");
		if (pitchAngle >= -90 && pitchAngle <= -80) {
			orientation = TEMPERATURE_MODE;
		} else if (rollAngle >= -90 && rollAngle <= -80){
			orientation = BATTERY_MODE;
		} else if (pitchAngle >= 80 && pitchAngle <= 90) {
			orientation = CLOCK_MODE;
		} else if ( z >= 120 && flag == 0){
			previousTime = HAL_GetTick();
			flag = 1;
		} else if (z <= 50 && flag == 1) {
			flag = 0;
			currentTime = HAL_GetTick();
			if (currentTime - previousTime <= 10000){
				orientation = VOLUME_MODE;
				volMode = 1;
			}
		}
		break;
	case TEMPERATURE_MODE:
		strcpy(modeName, "TEMP");
		if (rollAngle >= 80 && rollAngle <= 90) {
			orientation = STOPWATCH_MODE;
		} else if (rollAngle >= -90 && rollAngle <= -80){
			orientation = BATTERY_MODE;
		} else if (pitchAngle >= 80 && pitchAngle <= 90) {
			orientation = CLOCK_MODE;
		}else if ( z >= 120 && flag == 0){
			previousTime = HAL_GetTick();
			flag = 1;
		} else if (z <= 50 && flag == 1) {
			flag = 0;
			currentTime = HAL_GetTick();
			if (currentTime - previousTime <= 1000){
				orientation = VOLUME_MODE;
				volMode = 1;
			}
		}
		break;
	case BATTERY_MODE:
		strcpy(modeName, "BATT");
		if (rollAngle >= 80 && rollAngle <= 90) {
			orientation = STOPWATCH_MODE;
		} else if (pitchAngle >= -90 && pitchAngle <= -80) {
			orientation = TEMPERATURE_MODE;
		} else if (pitchAngle >= 80 && pitchAngle <= 90) {
			orientation = CLOCK_MODE;
		}else if ( z >= 120 && flag == 0){
			previousTime = HAL_GetTick();
			flag = 1;
		} else if (z <= 50 && flag == 1) {
			flag = 0;
			currentTime = HAL_GetTick();
			if (currentTime - previousTime <= 1000){
				orientation = VOLUME_MODE;
				volMode = 1;
			}
		}
		break;
	case CLOCK_MODE:
		strcpy(modeName, "CLOCK");
		if (rollAngle >= 80 && rollAngle <= 90) {
			orientation = STOPWATCH_MODE;
		} else if (rollAngle >= -90 && rollAngle <= -80){
			orientation = BATTERY_MODE;
		} else if (pitchAngle >= -90 && rollAngle <= -80) {
			orientation = TEMPERATURE_MODE;
		}else if ( z >= 120 && flag == 0){
			previousTime = HAL_GetTick();
			flag = 1;
		} else if (z <= 50 && flag == 1) {
			flag = 0;
			currentTime = HAL_GetTick();
			if (currentTime - previousTime <= 1000){
				orientation = VOLUME_MODE;
				volMode = 1;
			}
		}
		break;
	case VOLUME_MODE:
		strcpy(modeName, "VOLUM");
		volSeg = round((13*volCircle)/(2*M_PI));

		if (volMode != 0) {
			previousSeg = volSeg;
			volMode = 0;
		}
		int difAngle = 	volSeg - previousSeg;

		if (difAngle != 0) {
			if (difAngle < -10 || difAngle > 10) {
				difAngle = 0;
			}
			volTime = 0;
			currentSeg += difAngle;
			if (currentSeg > 13 ) {
				currentSeg = 13;
			} else if (currentSeg < 0) {
				currentSeg = 0;
			}
	} else{
			volTime = 1;
		}
		previousSeg = volSeg;
		if (!volTime){
			currentVolTime  = HAL_GetTick();
		}
		if ((HAL_GetTick() - currentVolTime >= 5000) && (volTime == 1)){
			orientation = CLOCK_MODE;
			volTime = 0;
		}
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	CurrentTimerVal = HAL_GetTick();
	if (GPIO_Pin == USER_BUTTON_PIN) {
		if (CurrentTimerVal - PreviousTimerVal > 100 ){
			pressed++;
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
			if (pressed > 5){
				pressed = 0;
			}

		}

		PreviousTimerVal = CurrentTimerVal;
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
