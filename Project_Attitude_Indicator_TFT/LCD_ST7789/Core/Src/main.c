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
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdio.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "ST7789H.h"
#include "ADXL345.h"
#include "tjpgd.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

UINT BytesCount;
FATFS myFATFS;
FIL myfile;

BYTE Buf_JPEG[4096]  	__attribute__ ((aligned(2)));		/* Working buffer */
BYTE Buf_Our[1024*10]; 

uint16_t ColorTable[] = {
	0x551E, 0xEFBF, 0x8327, 0xFF86, 0x636A
};


Vector filtered;
int roll = 0, pitch = 0;

enum type {
	Our,
	JPEG
};

enum type change2type = Our;

static unsigned int tjd_input(JDEC* jd, uint8_t* buff, unsigned int nd){
	UINT rb;
	FIL *fp = (FIL*)jd->device;
	if (buff) {	
		f_read(fp, buff, nd, &rb);
		return rb;	
	} 
	else {	
		return (f_lseek(fp, f_tell(fp) + nd) == FR_OK) ? nd : 0;
	}
}

static int tjd_output(JDEC* jd, void* bitmap, JRECT* rect){
	jd = jd;	
	const uint16_t* pat = bitmap;
	uint16_t heigh = rect->bottom - rect->top  + 1, 
					 width = rect->right  - rect->left + 1;
	uint32_t index = 0, size = width * heigh;
	ST7789H2_SetDisplayWindow(rect->left, rect->top, width, heigh);
	ST7789H2_WriteReg(ST7789H2_WRITE_RAM, (uint8_t*)NULL, 0);   /* RAM write data command */
	while(index < size) {
		LCD_IO_WriteData(pat[index]);
		index++;
	}
	return 1;	
}

void load_jpg(FIL* fp,	void *work,	UINT sz_work){
	JDEC jd;		
	JRESULT rc;
	rc = jd_prepare(&jd, tjd_input, work, sz_work, fp);
	if (rc == JDR_OK) {
		rc = jd_decomp(&jd, tjd_output, 0);	
	} 
}

void DrawJPEGData(const char* FileName) {
	f_open(&myfile, FileName, FA_READ);
	load_jpg(&myfile, Buf_JPEG, sizeof(Buf_JPEG));
	f_close(&myfile);
}

void DrawOurData(const char* FileName) {
	f_open(&myfile, FileName, FA_READ);
	const UINT size = f_size(&myfile);
	f_lseek(&myfile, 0);
	f_read(&myfile, Buf_Our, size, (UINT *)&BytesCount);
	f_close(&myfile);

	ST7789H2_SetDisplayWindow(0, 0, 240, 240);
	ST7789H2_WriteReg(ST7789H2_WRITE_RAM, (uint8_t*)NULL, 0);   /* RAM write data command */
	uint32_t index = 0;
	for (uint32_t index_Buf = 0; index_Buf < size; index_Buf += 3) {
		uint16_t N_times     = (Buf_Our[index_Buf + 0] << 8) | Buf_Our[index_Buf + 1];	
		uint16_t Color_index =  Buf_Our[index_Buf + 2];
		uint32_t Temp_index = index;
		while (index < (Temp_index + N_times)) {
			LCD_IO_WriteData(ColorTable[Color_index]);
			index++;
		}
	}
}

void DrawAH(int roll, int pitch, enum type t) {
	char file[30];
	if(t == Our) {
		sprintf(file, "Input1/P%c%d/R%d.bin", (pitch < 0) ? 'N' : '_', abs(pitch), roll);
		DrawOurData(file);
	}
	else if(t == JPEG){
		sprintf(file, "Input2/P%c%d/R%d.jpg", (pitch < 0) ? 'N' : '_', abs(pitch), roll);
		DrawJPEGData(file);
	}
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
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_TIM9_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
	__HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_1, 10);
	
	ST7789H2_Init();
	
	if(f_mount(&myFATFS, SDPath, 1) != FR_OK) {
		ST7789H2_DrawHLine(0xf800,0,0,200);
		while(1);
	}
	
	pitch = 0;
	roll = 0;
	
	//*
	// Set measurement range
  // +/-  2G: ADXL345_RANGE_2G
  // +/-  4G: ADXL345_RANGE_4G
  // +/-  8G: ADXL345_RANGE_8G
  // +/- 16G: ADXL345_RANGE_16G
  ADXL345_setRange(ADXL345_RANGE_16G);
	if (!ADXL345_begin()) {
    HAL_Delay(300);
  }
	//*/
	
	//DrawAH((roll>=0)?roll:(roll + 360), pitch);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, (change2type==JPEG)?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (change2type==Our)?GPIO_PIN_SET:GPIO_PIN_RESET);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		//*
		
		Vector norm = ADXL345_readNormalize(ADXL345_GRAVITY_EARTH); // Read normalized values
		filtered = ADXL345_lowPassFilter(norm, 0.15);        // Low Pass Filter to smooth out data. 0.1 - 0.9
		//pitch = (-(atan2(filtered.XAxis, sqrt(filtered.YAxis*filtered.YAxis + filtered.ZAxis*filtered.ZAxis))*180.0)/M_PI);
		//roll  = (atan2(filtered.YAxis, filtered.ZAxis)*180.0)/M_PI;
		//roll = (roll>=0)?roll:(roll + 360);
		pitch = ((atan2(filtered.ZAxis, sqrt(filtered.XAxis*filtered.XAxis + filtered.YAxis*filtered.YAxis))*180.0)/M_PI);
		roll  = 359 - (((atan2(filtered.XAxis, filtered.YAxis)*180.0)/M_PI) + 180);
		//*
		static bool flag = false;
		if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
			flag = true;
		}
		else if((HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) && (flag == true)) {
			flag = false;
			change2type = (change2type==Our)?JPEG:Our;
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, (change2type==JPEG)?GPIO_PIN_SET:GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (change2type==Our)?GPIO_PIN_SET:GPIO_PIN_RESET);
		}//*/
		HAL_GPIO_WritePin(Test_SIG_GPIO_Port, Test_SIG_Pin, GPIO_PIN_SET);
		DrawAH(roll, pitch, change2type);
		HAL_GPIO_WritePin(Test_SIG_GPIO_Port, Test_SIG_Pin, GPIO_PIN_RESET);
		//*/
		
		
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDIO|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
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
