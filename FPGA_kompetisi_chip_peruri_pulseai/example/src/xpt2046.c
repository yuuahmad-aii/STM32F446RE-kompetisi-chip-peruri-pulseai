#include "xpt2046.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

#define XPT2046_CMD_RDX 0xD0 // Read X
#define XPT2046_CMD_RDY 0x90 // Read Y

// Calibrations (assuming roughly a 12-bit ADC mapping to 480x320)
#define XPT2046_MIN_RAW_X 300
#define XPT2046_MAX_RAW_X 3800
#define XPT2046_MIN_RAW_Y 300
#define XPT2046_MAX_RAW_Y 3800

#define ILI9488_WIDTH 480
#define ILI9488_HEIGHT 320

static void XPT2046_CS_LOW(void) {
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
}

static void XPT2046_CS_HIGH(void) {
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

static uint16_t XPT2046_ReadValue(uint8_t command) {
  uint8_t data[3] = {command, 0x00, 0x00};
  uint8_t rx_data[3] = {0};

  // SPI2 speed is relatively slow to be safe for XPT2046, handled by CubeMX
  // (Prescaler 8)
  HAL_SPI_TransmitReceive(&hspi2, data, rx_data, 3, HAL_MAX_DELAY);

  // The 12-bit ADC value is typically in rx_data[1] (MSB) and rx_data[2] (LSB)
  return ((rx_data[1] << 8) | rx_data[2]) >> 3;
}

void XPT2046_Init(void) { XPT2046_CS_HIGH(); }

bool XPT2046_IsTouched(void) {
  // If the IRQ pin is LOW, it means it's being touched
  return HAL_GPIO_ReadPin(SPI2_IRQ_GPIO_Port, SPI2_IRQ_Pin) == GPIO_PIN_RESET;
}

bool XPT2046_GetTouch(uint16_t *x, uint16_t *y) {
  if (!XPT2046_IsTouched()) {
    return false;
  }

  XPT2046_CS_LOW();

  // Discard first read (dummy read to wake up / settle)
  XPT2046_ReadValue(XPT2046_CMD_RDX);

  uint32_t raw_x = 0;
  uint32_t raw_y = 0;

  // Read multiple times and average to reduce noise
  for (int i = 0; i < 3; i++) {
    raw_x += XPT2046_ReadValue(XPT2046_CMD_RDX);
    raw_y += XPT2046_ReadValue(XPT2046_CMD_RDY);
  }

  XPT2046_CS_HIGH();

  raw_x /= 3;
  raw_y /= 3;

  // The physical touch panel's X axis is actually the LCD's Y axis (short edge)
  // The physical touch panel's Y axis is actually the LCD's X axis (long edge)

  // So we map raw_y to LCD X (0-480)
  int32_t cal_x = (raw_y - XPT2046_MIN_RAW_Y) * ILI9488_WIDTH /
                  (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);
  // And map raw_x to LCD Y (0-320)
  int32_t cal_y = (raw_x - XPT2046_MIN_RAW_X) * ILI9488_HEIGHT /
                  (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);

  if (cal_x < 0)
    cal_x = 0;
  if (cal_x >= ILI9488_WIDTH)
    cal_x = ILI9488_WIDTH - 1;
  if (cal_y < 0)
    cal_y = 0;
  if (cal_y >= ILI9488_HEIGHT)
    cal_y = ILI9488_HEIGHT - 1;

  // Assign to output with inversion
  *x = ILI9488_WIDTH - cal_x;
  *y = ILI9488_HEIGHT - cal_y;

  return true;
}
