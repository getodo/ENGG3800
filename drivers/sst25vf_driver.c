///*
// * sst25vf_driver.c
// *
// *  Created on: 03 Oct 2020
// *      Author: George Todorovic
// *		ENGG3800 driver file for flash memory
// */
//#include "sst25vf_driver.h"
//
//const uint8_t FLASH_INIT_REGISTER = 0x00; // Init the status register (remove blocking bits)
///*
// * Status register byte values represent:
// * 0 BUSY
// * 1 WEL Write enable
// * 2 BP0 Current level of block write protection (Refer to data sheet)
// * 3 BP1 Current level of block write protection
// * 4 BP2 Current level of block write protection
// * 5 BP3 Current level of block write protection
// * 6 AAI Auto address increment status
// * 7 BPL Block protection lockdown
// * Default at power up: 00011100
//*/
//const uint8_t FLASH_WSRS = 0x01; // Write to status register
//const uint8_t FLASH_WRITE_BYTE = 0x02; // Write a byte
//const uint8_t FLASH_READ = 0x03; // Read memory
//const uint8_t FLASH_WRDI = 0x04; // Write diable
//const uint8_t FLASH_RDSR = 0x05; // Read status register
//const uint8_t FLASH_WREN = 0x06; // Write enable
//const uint8_t FLASH_READ_FAST = 0x0B; // Read memory at high speed
//const uint8_t FLASH_EWSR = 0x50; // Enable write to status register
//const uint8_t FLASH_ERASE = 0x60; // Erase the chip
//
///**
//  * @brief  Initialise the flash chip status register.
//  * @param  None
//  * @retval None
//  */
//void sst25vf_init()
//{
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&FLASH_EWSR, 1, 100);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//	HAL_Delay(50);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&FLASH_WSRS, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&FLASH_INIT_REGISTER, 1, 100);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//	HAL_Delay(50);
//}
//
//uint8_t sst25vf_read_status_register()
//{
//	uint8_t statusBuffer[1], status;
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&FLASH_RDSR, 1, 100);
//	HAL_SPI_Receive(&hspi1, (uint8_t *)&statusBuffer, 1, 100);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//	status = statusBuffer[0];
//	return status;
//}
//
//void sst25vf_write_byte(uint8_t add1, uint8_t add2, uint8_t add3, uint8_t data)
//{
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&FLASH_WREN, 1, 100);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//	HAL_Delay(50);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&FLASH_WRITE_BYTE, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add1, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add2, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add3, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&data, 1, 100);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//	HAL_Delay(50);
//}
//
//uint8_t sst25vf_read_byte(uint8_t add1, uint8_t add2, uint8_t add3)
//{
//	uint8_t byteBuffer[1], byte;
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&FLASH_READ, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add1, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add2, 1, 100);
//	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add3, 1, 100);
//	HAL_SPI_Receive(&hspi1, (uint8_t *)&byteBuffer, 1, 100);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//	byte = byteBuffer[0];
//	return byte;
//}
