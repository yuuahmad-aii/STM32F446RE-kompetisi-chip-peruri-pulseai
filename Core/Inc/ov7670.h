#ifndef OV7670_H
#define OV7670_H

#include "main.h"

extern I2C_HandleTypeDef hi2c2;

void OV7670_Init(void);
uint8_t OV7670_WriteReg(uint8_t regAddr, uint8_t data);
uint8_t OV7670_ReadReg(uint8_t regAddr);

#endif /* OV7670_H */
