#include "ov7670.h"

#define OV7670_I2C_ADDR (0x21 << 1)

uint8_t OV7670_WriteReg(uint8_t regAddr, uint8_t data) {
  return HAL_I2C_Mem_Write(&hi2c2, OV7670_I2C_ADDR, regAddr,
                           I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

uint8_t OV7670_ReadReg(uint8_t regAddr) {
  uint8_t data = 0;
  // SCCB tidak mendukung "Repeated Start" standar I2C.
  // Kita harus menggunakan Transmit (untuk set register address) lalu STOP,
  // baru kemudian Receive (untuk membaca data) dengan START baru.
  HAL_I2C_Master_Transmit(&hi2c2, OV7670_I2C_ADDR, &regAddr, 1, 100);
  HAL_I2C_Master_Receive(&hi2c2, OV7670_I2C_ADDR, &data, 1, 100);
  return data;
}

const uint8_t ov7670_qqvga_rgb565[][2] = {
    {0x12, 0x80}, // COM7 Reset
    {0xFF, 100},  // Delay 100ms

    // Menggunakan resolusi QCIF (176x144) bawaan sensor tanpa perlu scaling register tambahan
    {0x12, 0x0E}, // COM7: QCIF (0x08), RGB (0x04) + Color Bar (0x02) = 0x0E
    {0x11, 0x01}, // CLKRC: Prescaler = 1
    {0x40, 0xD0}, // COM15: RGB565
    {0x8C, 0x00}, // RGB444: disable
    {0x15, 0x00}, // COM10: PCLK does not toggle on HBLANK (0), normal VSYNC
    
    // Matrix (Standard RGB)
    {0x4f, 0x80}, {0x50, 0x80}, {0x51, 0x00}, {0x52, 0x22},
    {0x53, 0x5e}, {0x54, 0x80}, {0x58, 0x9e},
    
    // Enable DSP Color Bar (COM17) - Tampilkan 8 pita warna
    {0x42, 0x08},

    {0xFF, 0xFF}
};

void OV7670_Error_Blink(void) {
  while (1) {
    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    HAL_Delay(100); // Kedip sangat cepat (10Hz) pertanda I2C gagal
  }
}

void OV7670_Init(void) {
  // Hardware Reset (if pins are connected)
  HAL_GPIO_WritePin(OV7670_RET_GPIO_Port, OV7670_RET_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OV7670_PWDN_GPIO_Port, OV7670_PWDN_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(OV7670_RET_GPIO_Port, OV7670_RET_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(OV7670_RET_GPIO_Port, OV7670_RET_Pin, GPIO_PIN_SET);
  HAL_Delay(100);

  // 1. Cek apakah perangkat OV7670 membalas I2C (ACK)
  if (HAL_I2C_IsDeviceReady(&hi2c2, OV7670_I2C_ADDR, 10, 1000) != HAL_OK) {
    OV7670_Error_Blink(); // Gagal I2C! Berhenti di sini dan kedipkan LED
  }

  // 2. Cek apakah PID benar
  uint8_t pid = OV7670_ReadReg(0x0A); // PID harus 0x76
  if (pid != 0x76) {
    OV7670_Error_Blink(); // Salah PID! Berhenti di sini dan kedipkan LED
  }

  uint8_t ver = OV7670_ReadReg(0x0B); // VER should be 0x73
  (void)ver;

  // Proses pengiriman konfigurasi ke kamera
  for (int i = 0; i < sizeof(ov7670_qqvga_rgb565)/sizeof(ov7670_qqvga_rgb565[0]); i++) {
    // Jika menemukan {0xFF, 0xFF}, maka array berakhir
    if (ov7670_qqvga_rgb565[i][0] == 0xFF && ov7670_qqvga_rgb565[i][1] == 0xFF) {
      break; 
    }
    // Jika menemukan {0xFF, nilai}, lakukan delay selama 'nilai' ms
    else if (ov7670_qqvga_rgb565[i][0] == 0xFF) {
      HAL_Delay(ov7670_qqvga_rgb565[i][1]);
    }
    // Selain itu, kirim ke register
    else {
      uint8_t ret = OV7670_WriteReg(ov7670_qqvga_rgb565[i][0], ov7670_qqvga_rgb565[i][1]);
      if (ret != HAL_OK) {
        OV7670_Error_Blink(); // Jika gagal mengirim konfigurasi, blink LED!
      }
      HAL_Delay(1); // Kasih jeda sedikit antar pengiriman
    }
  }
}
