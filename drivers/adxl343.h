
#ifndef INC_ADXL343_H_
#define INC_ADXL343_H_

#include "main.h"

#define adxl_address 0x53<<1

/*Communication variables for I2C and USART*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

/*variables for data being read from chip*/
uint8_t data_rec[6];
int16_t x,y,z;

/*variables for stopwatch state*/
extern int pause;
extern int reset;

void adxl_write(uint8_t reg, uint8_t value);
void adxl_read_values(uint8_t reg);
void adxl_read_address(uint8_t reg);
void adxl_init(void);
void calc_x_y_z_values(void);
void adxl_init_interrupt();
void adxl_detect_tap();

#endif /* INC_ADXL343_H_ */
