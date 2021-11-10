/*
 * temphumid.h
 *
 *  Created on: 20 Sep 2020
 *      Author: justinlim
 */

#ifndef INC_HTS221_DRIVER_H_
#define INC_HTS221_DRIVER_H_

#include "main.h"

#define HTS221_ADDRESS 			0x5F<<1
#define HTS221_HR_OUT_L_REG 	0x28
#define HTS221_HR_OUT_H_REG 	0x29
#define HTS221_TEMP_OUT_L_REG 	0x2A
#define HTS221_TEMP_OUT_H_REG 	0x2B
#define HTS221_H0_RH_X2      	0x30
#define HTS221_H1_RH_X2      	0x31
#define HTS221_T0_DEGC_X8    	0x32
#define HTS221_T1_DEGC_X8    	0x33
#define HTS221_T0_T1_DEGC_H2 	0x35
#define HTS221_H0_T0_OUT_L   	0x36
#define HTS221_H0_T0_OUT_H   	0x37
#define HTS221_H1_T0_OUT_L   	0x3A
#define HTS221_H1_T0_OUT_H   	0x3B
#define HTS221_T0_OUT_L      	0x3C
#define HTS221_T0_OUT_H      	0x3D
#define HTS221_T1_OUT_L      	0x3E
#define HTS221_T1_OUT_H      	0x3F

I2C_HandleTypeDef hi2c1;

void hts221_init();
float hts221_read_temp();
float hts221_read_humid();

#endif /* INC_HTS221_DRIVER_H_ */
