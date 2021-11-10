
#include "adxl343.h"

uint8_t chipid = 0;

void adxl_write(uint8_t reg, uint8_t value)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, data, 2, 100);
}

void adxl_read_values(uint8_t reg)
{
	HAL_I2C_Mem_Read(&hi2c1, adxl_address, reg, 1, (uint8_t *)data_rec, 6, 100);
}

void adxl_read_address(uint8_t reg)
{
	HAL_I2C_Mem_Read(&hi2c1, adxl_address, reg, 1, &chipid, 1, 100);
}

void adxl_init(void)
{
	adxl_read_address(0x00); // read the DEVID
	adxl_write(0x31, 0x01);  // data_format range= +- 4g
	adxl_write(0x2d, 0x00);  // reset all bits
	adxl_write(0x2d, 0x08);  // power_cntl measure and wake up 8hz
}

void adxl_init_interrupt()
{
	uint8_t interruptReg[2] = {0x2E, 0x50};
	uint8_t threshReg[2] = {0x1D, 0x70};
	uint8_t durationReg[2] = {0x21, 0x10};
	uint8_t axesReg[2] = {0x2A, 0x0F};

	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, interruptReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, threshReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, durationReg, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, axesReg, 2, 100);
}

void adxl_detect_tap()
{
	uint8_t data[1];
	data[0] = 0x30;

	HAL_I2C_Master_Transmit(&hi2c1, adxl_address, data, 1, 100);
	HAL_I2C_Mem_Read(&hi2c1, adxl_address, 0x30, 1, data, 1, 100);

	if(data[0] == 195)
	{
		HAL_UART_Transmit(&huart1, (uint8_t*)&data[0], sizeof(data), 100);
	}
}

void calc_x_y_z_values(void)
{
	adxl_read_values(0x32);
	x = ((data_rec[1]<<8)|data_rec[0]);
	y = ((data_rec[3]<<8)|data_rec[2]);
	z = ((data_rec[5]<<8)|data_rec[4]);
}
