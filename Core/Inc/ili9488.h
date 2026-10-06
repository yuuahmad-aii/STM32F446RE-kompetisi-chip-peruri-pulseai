#ifndef __ILI9488_H
#define __ILI9488_H

#include "stm32f4xx_hal.h"
#include "fonts.h"

// Screen dimensions
#define ILI9488_WIDTH  480
#define ILI9488_HEIGHT 320

// Basic colors (RGB565)
#define ILI9488_BLACK       0x0000
#define ILI9488_NAVY        0x000F
#define ILI9488_DARKGREEN   0x03E0
#define ILI9488_DARKCYAN    0x03EF
#define ILI9488_MAROON      0x7800
#define ILI9488_PURPLE      0x780F
#define ILI9488_OLIVE       0x7BE0
#define ILI9488_LIGHTGREY   0xC618
#define ILI9488_DARKGREY    0x7BEF
#define ILI9488_BLUE        0x001F
#define ILI9488_GREEN       0x07E0
#define ILI9488_CYAN        0x07FF
#define ILI9488_RED         0xF800
#define ILI9488_MAGENTA     0xF81F
#define ILI9488_YELLOW      0xFFE0
#define ILI9488_WHITE       0xFFFF
#define ILI9488_ORANGE      0xFD20
#define ILI9488_GREENYELLOW 0xAFE5
#define ILI9488_PINK        0xF81F

// Initialization function
void ILI9488_Init(void);

// Drawing primitives
void ILI9488_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9488_FillScreen(uint16_t color);
void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9488_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9488_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

// Text functions
void ILI9488_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9488_WriteCharScaled(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale);
void ILI9488_WriteStringScaled(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale);

// LVGL specific functions
void ILI9488_DrawBitmapLVGL(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const uint8_t *data);

// Camera specific functions
void ILI9488_DrawCameraFrame(uint16_t *frame_buf);

#endif /* __ILI9488_H */
