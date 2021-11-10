/*
 * veml7700_driver.h
 *
 *  Created on: 20 Sep 2020
 *      Author: George Todorovic
 *		ENGG3800 driver file for light sensor
 */

#ifndef INC_VEML7700_DRIVER_H_
#define INC_VEML7700_DRIVER_H_

#include "main.h"

I2C_HandleTypeDef hi2c1;

void veml7700_init();
uint16_t veml7700_read_als();

#endif /* INC_VEML7700_DRIVER_H_ */
