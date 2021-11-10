/*
 * veml7700_driver.c
 *
 *  Created on: 20 Sep 2020
 *      Author: George Todorovic
 *		ENGG3800 driver file for light sensor
 */
#include "veml7700_driver.h"

const uint8_t VEML7700_WRITE = (0x10 << 1) | 0x00;
const uint8_t VEML7700_READ = (0x10 << 1) | 0x01;
const uint16_t VEML7700_CONFIG_REG_0 = ((uint16_t)0x01 << 11) | ((uint16_t)0x00 << 6) | ((uint16_t)0x00 << 1) | ((uint16_t)0x00 << 0);
const uint16_t VEML7700_POWER_MODE = ((uint16_t)0x02 << 1) | ((uint16_t)0x00 << 0);

/**
  * @brief  Initialise VEML17700 Sensor
  * @param  none
  * @retval none
  */
void veml7700_init() 
{
	uint8_t txLight[3];
	txLight[0] = 0x00;
	txLight[1] = (uint8_t)(VEML7700_CONFIG_REG_0 & 0xFF); //LSB
	txLight[2] = (uint8_t)(VEML7700_CONFIG_REG_0 >> 8); //MSB
	HAL_I2C_Master_Transmit(&hi2c1, VEML7700_WRITE, (uint8_t *)txLight, 3, 0xFF);
	HAL_Delay(3);

	txLight[0] = 0x01;
	txLight[1] = (uint8_t)(0x0000 & 0xFF); //LSB
	txLight[2] = (uint8_t)(0x0000 >> 8); //MSB
	HAL_I2C_Master_Transmit(&hi2c1, VEML7700_WRITE, (uint8_t *)txLight, 3, 0xFF);
	HAL_Delay(3);

	txLight[0] = 0x02;
	txLight[1] = (uint8_t)(0xFFFF & 0xFF); //LSB
	txLight[2] = (uint8_t)(0xFFFF >> 8); //MSB
	HAL_I2C_Master_Transmit(&hi2c1, VEML7700_WRITE, (uint8_t *)txLight, 3, 0xFF);
	HAL_Delay(3);
	
	txLight[0] = 0x03;
	txLight[1] = (uint8_t)(VEML7700_POWER_MODE & 0xFF); //LSB
	txLight[2] = (uint8_t)(VEML7700_POWER_MODE >> 8); //MSB
	HAL_I2C_Master_Transmit(&hi2c1, VEML7700_WRITE, (uint8_t *)txLight, 3, 0xFF);
	HAL_Delay(3);
}

/**
  * @brief  read light sensor data
  * @param  none
  * @retval uint16_t lux - light value measured as lux
  */
uint16_t veml7700_read_als()
{
	uint8_t rxAls[2];
	uint16_t lux;
	HAL_I2C_Mem_Read(&hi2c1, VEML7700_READ, 0x04, 1, rxAls, 2, 0xFF);
	lux = rxAls[1];
	lux = lux << 8;
	lux |= rxAls[0];
	return lux;
}
 

