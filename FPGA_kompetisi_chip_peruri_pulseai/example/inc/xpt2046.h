#ifndef XPT2046_H
#define XPT2046_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

void XPT2046_Init(void);
bool XPT2046_IsTouched(void);
bool XPT2046_GetTouch(uint16_t* x, uint16_t* y);

#endif // XPT2046_H
