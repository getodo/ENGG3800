/*
 * m95m02_driver_driver.h
 *
 *  Created on: 20 Sep 2020
 *      Author: George Todorovic
 *		ENGG3800 driver file for 256KB EEPROM memory
 */

#ifndef INC_M95M02_DRIVER_H_
#define INC_M95M02_DRIVER_H_

#include "main.h"

SPI_HandleTypeDef hspi1;

void m95m02_init();
void m95m02_wip();
void m95m02_write_byte(uint32_t addr, uint8_t data);
uint8_t m95m02_read_byte(uint32_t address);
void m95m02_write_array(uint8_t *dataArray, uint32_t bytes, uint16_t page);

#endif /* INC_M95M02_DRIVER_H_ */
