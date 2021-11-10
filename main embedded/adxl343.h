

#ifndef INC_ADXL343_H_
#define INC_ADXL343_H_

#include "main.h"

#define adxl_address 0x53<<1

I2C_HandleTypeDef hi2c1;
uint8_t data_rec[6];
int16_t x,y,z;

void adxl_write(uint8_t reg, uint8_t value);
void adxl_read_values(uint8_t reg);
void adxl_read_address(uint8_t reg);
void adxl_init(void);
void calc_x_y_z_values(void);

#endif /* INC_ADXL343_H_ */
