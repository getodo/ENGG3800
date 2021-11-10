
#include "adxl343.h"
#include "stdio.h"

/*variables used for tap*/
uint8_t tap = 0;
uint8_t previousTap = 0;
/*chip address*/
uint8_t chipid = 0;

/*pause and reset state*/
int pause = 0;
int reset = 0;

/*variable for USART transmitting*/
char uartBuff[10];
int uartBuffLen;

/**
  * @brief  write to adxl sensor
  * @param  uint8_t reg, uint8_t value
  * @retval none
  */
void adxl_write(uint8_t reg, uint8_t value)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, data, 2, 100);
}

/**
  * @brief  read data from adxl sensor
  * @param  uint8_t reg
  * @retval none
  */
void adxl_read_values(uint8_t reg)
{
	HAL_I2C_Mem_Read(&hi2c1, adxl_address, reg, 1, (uint8_t *)data_rec, 6, 100);
}

/**
  * @brief  read raw data
  * @param  uint8_t reg
  * @retval none
  */
void adxl_read_address(uint8_t reg)
{
	HAL_I2C_Mem_Read(&hi2c1, adxl_address, reg, 1, &chipid, 1, 100);
}

/**
  * @brief  Initialise adxl Sensor
  * @param  none
  * @retval none
  */
void adxl_init(void)
{
	adxl_read_address(0x00); // read the DEVID
	adxl_write(0x31, 0x01);  // data_format range= +- 4g
	adxl_write(0x2d, 0x00);  // reset all bits
	adxl_write(0x2d, 0x08);  // power_cntl measure and wake up 8hz
}

/**
  * @brief  Init interrupt for adxl sensor
  * @param  none
  * @retval none
  */
void adxl_init_interrupt()
{
	uint8_t interruptReg[2] = {0x2E, 0xE3};
	uint8_t threshReg[2] = {0x1D, 35}; // Weight required was event greater then the threshhold tap
	uint8_t durationReg[2] = {0x21, 0xFF}; // Time event must be triggered for (how long weight is on chip)
	uint8_t latentReg[2] = {0x22, 20}; // Time before the window of a 2nd tap
	uint8_t windowReg[2] = {0x23, 0xFF}; // Time after latent time a 2nd tap can be registered
	uint8_t axesReg[2] = {0x2A, 0x0F}; //

	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, threshReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, durationReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, latentReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, windowReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, axesReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, interruptReg, 2, 100);
}

/**
  * @brief  Detect tap from sensor
  * @param  none
  * @retval none
  */
void adxl_detect_tap()
{
	/*variable for tap register value*/
	uint8_t data[1];
	data[0] = 0x30;

	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, data, 1, 100);
	HAL_I2C_Mem_Read(&hi2c1, adxl_address, 0x30, 1, data, 1, 100);

	tap = data[0]; // Store data from tap interrupt register

	uartBuffLen = sprintf(uartBuff, "%d\n", tap); // TESTING PURPOSES
	HAL_UART_Transmit(&huart1, (uint8_t*)uartBuff, uartBuffLen, 100); // TESTING PURPOSES

	if(previousTap == 0xC3) { // If we previously received a tap
		if(tap == 0x83) {
			pause ^= 1;
		}
		else if(tap == 0xC3) {
			reset ^= 1;
		}
	}
	if(previousTap == 0xC3 && tap == 0xC3) { /*apply debouncing to tap*/
		previousTap = 0x83;
	}
	else {
		previousTap = tap;
	}
}

/**
  * @brief  Read XYZ values from sensor
  * @param  none
  * @retval none
  */
void calc_x_y_z_values(void)
{
	adxl_read_values(0x32);
	x = ((data_rec[1]<<8)|data_rec[0]);
	y = ((data_rec[3]<<8)|data_rec[2]);
	z = ((data_rec[5]<<8)|data_rec[4]);
}
