/*
 * m95m02_driver_driver.c
 *
 *  Created on: 20 Sep 2020
 *      Author: George Todorovic
 *		ENGG3800 driver file for 256KB EEPROM memory
 */
#include "m95m02_driver.h"
#include <string.h>

const uint8_t EEPROM_WREN = 0x06; // Write enable
const uint8_t EEPROM_WRDI = 0x04; // Write disable
const uint8_t EEPROM_RDSR = 0x05; // Read status register
const uint8_t EEPROM_WRSR = 0x01; // Write status register
const uint8_t EEPROM_READ = 0x03; // Read from memory array
const uint8_t EEPROM_WRITE = 0x02; // Write to memory array
const uint8_t EEPROM_RDID = 0x83; // Read ID page
const uint8_t EEPROM_WRID = 0x82; // Write ID page
const uint8_t EEPROM_INIT = 0x02; // Init status register (may not be needed)

void m95m02_init()
{
	uint8_t txEeprom[2];
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_WREN, 1, 0xFF);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	m95m02_wip();

	txEeprom[0] =  EEPROM_WRSR; // Write to status register
	txEeprom[1] =  EEPROM_INIT;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&txEeprom, 2, 0xFF);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	m95m02_wip();
}

void m95m02_wip()
{
	uint8_t rxStatus[1];
	do
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_RDSR, 1, 0xFF);
		HAL_SPI_Receive(&hspi1, (uint8_t *)rxStatus, 1, 0xFF);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	}
	while(rxStatus[0] % 2 == 1);
}

void m95m02_write_byte(uint32_t addr, uint8_t data)
{
	uint8_t txData[4];

	txData[0] = (addr & 0xFF0000) >> 16; // MSB
	txData[1] = (addr & 0x00FF00) >> 8; // middle
	txData[2] = (addr & 0x0000FF); // LSB
	txData[3] = data;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_WREN, 1, 0xFF);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	m95m02_wip();

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_WRITE, 1, 0xFF);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)txData, 4, 0xFF);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	m95m02_wip();
}

uint8_t m95m02_read_byte(uint32_t addr)
{
	uint8_t address[3];
	uint8_t rxData[1];
	uint8_t data;
	address[0] = (addr & 0xFF0000) >> 16; // MSB
	address[1] = (addr & 0x00FF00) >> 8; // middle
	address[2] = (addr & 0x0000FF); // LSB

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_READ, 1, 0xFF);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)address, 3, 0xFF);
	HAL_SPI_Receive(&hspi1, (uint8_t *)rxData, 1, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	m95m02_wip();

	data = rxData[0];

	return data;
}

uint8_t *m95m02_read_page(int page)
{
	uint32_t addr = page * 256;
	uint8_t address[3];
	static uint8_t data[256];
	address[0] = (addr & 0xFF0000) >> 16; // MSB
	address[1] = (addr & 0x00FF00) >> 8; // middle
	address[2] = (addr & 0x0000FF); // LSB

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_READ, 1, 0xFF);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)address, 3, 0xFF);
	HAL_SPI_Receive(&hspi1, (uint8_t *)data, 256, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	m95m02_wip();

	return data;
}

void m95m02_write_array(uint8_t *dataArray, uint32_t bytes, uint16_t page)
{
	uint8_t txAddress[3];
	uint8_t txData[256];
	uint16_t pageCount = bytes / 256;

	uint32_t address;
	/* Loop for as many pages required */
	for(int i = page; i < (pageCount + 1 + page); i++)
	{
		/* Copy elements from dataArray in txData as only 255 bytes
		 * can be stored on a single page essentially we increment dataArray
		 * into txData.
		 */
		for(int j = 0; j < 256; j++)
		{
			txData[j] = dataArray[((i - page) * 256) + j];
		}
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_WREN, 1, 0xFF);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
		m95m02_wip();

		address = i * 256;
		txAddress[0] = (address & 0xFF0000) >> 16; // MSB
		txAddress[1] = (address & 0x00FF00) >> 8; // middle
		txAddress[2] = (address & 0x0000FF); // LSB

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_WRITE, 1, 0xFF);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)txAddress, 3, 0xFF);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)txData, 256, 0xFF);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
		m95m02_wip();
	}
}

