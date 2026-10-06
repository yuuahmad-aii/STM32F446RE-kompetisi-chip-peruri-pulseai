#include "ov7670.h"

#define OV7670_I2C_ADDR (0x21 << 1)

uint8_t OV7670_WriteReg(uint8_t regAddr, uint8_t data) {
    return HAL_I2C_Mem_Write(&hi2c2, OV7670_I2C_ADDR, regAddr, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
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
    {0xFF, 0xFF}, // Delay
    
    // QQVGA RGB565 Full initialization
    {0x11, 0x01}, // CLKRC: Prescaler = 1
    // 0x16 = QVGA, RGB, Color Bar Enable. (Ganti ke 0x14 untuk mematikan Color Bar)
    {0x12, 0x16}, // COM7: QQVGA, RGB + COLOR BAR
    {0x0C, 0x04}, // COM3: Enable scaling, DCW enable
    {0x3E, 0x1A}, // COM14: scaling manual enable, PCLK divided by 4
    {0x40, 0xD0}, // COM15: RGB565, Full range
    {0x8C, 0x00}, // RGB444: disable
    
    {0x32, 0x80}, // HREF
    {0x17, 0x16}, // HSTART
    {0x18, 0x04}, // HSTOP
    {0x19, 0x02}, // VSTART
    {0x1A, 0x7A}, // VSTOP
    {0x03, 0x0A}, // VREF
    {0x15, 0x00}, // COM10: PCLK does not toggle on HBLANK
    
    {0x3A, 0x04}, // TSLB
    {0x3D, 0xC0}, // COM13
    
    {0x70, 0x3A}, {0x71, 0x35}, {0x72, 0x11}, {0x73, 0xF0}, {0xA2, 0x02},
    
    // Color matrix
    {0x4f, 0x80}, {0x50, 0x80}, {0x51, 0x00}, {0x52, 0x22},
    {0x53, 0x5e}, {0x54, 0x80}, {0x58, 0x9e},
    
    {0x1E, 0x27}, // MVFP: Flip / Mirror (adjust if image is upside down)
    
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
    
    // Optional: if (pid != 0x76) { Error_Handler(); }
    // If it fails here, I2C is not working properly. We will just proceed and try to write anyway.
    
    for (int i = 0; ov7670_qqvga_rgb565[i][0] != 0xFF || ov7670_qqvga_rgb565[i][1] != 0xFF; i++) {
        if (ov7670_qqvga_rgb565[i][0] == 0xFF) {
            HAL_Delay(100); // 100ms delay after soft reset
        } else {
            OV7670_WriteReg(ov7670_qqvga_rgb565[i][0], ov7670_qqvga_rgb565[i][1]);
            HAL_Delay(5);
        }
    }
}
