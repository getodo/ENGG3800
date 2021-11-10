/*
 * hts221_driver.c
 *
 *  Created on: 20 Sep 2020
 *      Author: George Todorovic
 *		ENGG3800 driver file for temperature and humidity sensor
 */

#include "hts221_driver.h"
#include "math.h"

void hts221_init() {
	uint8_t data[2] = {0x20, 0x83};
	HAL_I2C_Master_Transmit(&hi2c1, HTS221_ADDRESS, data, 2, 100);
}

float hts221_read_temp() {
    int16_t T0_out, T1_out, T_out, T0_degC_x8_u16, T1_degC_x8_u16;
    int16_t T0_degC, T1_degC;
    uint8_t buffer[4], tmp;
    float tmp_f;
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_T0_DEGC_X8 | 0x80, 1, buffer, 2, 0xFF);
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_T0_T1_DEGC_H2 | 0x80, 1, &tmp, 1, 0xFF);
    T0_degC_x8_u16 = (((uint16_t)(tmp & 0x03)) << 8) | ((uint16_t)buffer[0]);
    T1_degC_x8_u16 = (((uint16_t)(tmp & 0x0C)) << 6) | ((uint16_t)buffer[1]);
    T0_degC = T0_degC_x8_u16 >> 3;
    T1_degC = T1_degC_x8_u16 >> 3;
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_T0_OUT_L | 0x80, 1, buffer, 4, 0xFF);
    T0_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];
    T1_out = (((uint16_t)buffer[3]) << 8) | (uint16_t)buffer[2];
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_TEMP_OUT_L_REG | 0x80, 1, buffer, 2, 0xFF);
    T_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];
    tmp_f = (float)(T_out - T0_out) * (float)(T1_degC - T0_degC) / (float)(T1_out - T0_out) + T0_degC;
    return tmp_f;
}

float hts221_read_humid() {
    int16_t H0_T0_out, H1_T0_out, H_T_out;
    int16_t H0_rh, H1_rh;
    uint8_t buffer[2];
    float tmp_f;
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_H0_RH_X2 | 0x80, 1, buffer, 2, 0xFF);
    H0_rh = buffer[0] >> 1;
    H1_rh = buffer[1] >> 1;
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_H0_T0_OUT_L | 0x80, 1, buffer, 2, 0xFF);
    H0_T0_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_H1_T0_OUT_L | 0x80, 1, buffer, 2, 0xFF);
    H1_T0_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];
    HAL_I2C_Mem_Read(&hi2c1, HTS221_ADDRESS, HTS221_HR_OUT_L_REG | 0x80, 1, buffer, 2, 0xFF);
    H_T_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];
    tmp_f = (float)(H_T_out - H0_T0_out) * (float)(H1_rh - H0_rh) / (float)(H1_T0_out - H0_T0_out) + H0_rh;
    return (tmp_f > 100.0f) ? 100 : (tmp_f < 0.0f) ? 0 : tmp_f;
}
