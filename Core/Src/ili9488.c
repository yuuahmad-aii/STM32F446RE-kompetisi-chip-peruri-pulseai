#include "ili9488.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

#define DMA_BUFFER_SIZE 1440
static uint8_t dma_buffer[DMA_BUFFER_SIZE];
volatile uint8_t spi_dma_complete = 0;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI2) {
        spi_dma_complete = 1;
    }
}

static void ILI9488_TransmitDMA(uint8_t *data, uint16_t size) {
    if (hspi2.hdmatx != NULL) {
        spi_dma_complete = 0;
        if (HAL_SPI_Transmit_DMA(&hspi2, data, size) == HAL_OK) {
            uint32_t tickstart = HAL_GetTick();
            while (!spi_dma_complete) {
                if ((HAL_GetTick() - tickstart) > 200) { // 200ms timeout
                    HAL_SPI_Abort(&hspi2);
                    break;
                }
            }
            return;
        }
    }
    HAL_SPI_Transmit(&hspi2, data, size, HAL_MAX_DELAY);
}

// Helper macros for GPIO
#define ILI9488_CS_LOW()  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET)
#define ILI9488_CS_HIGH() HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET)

#define ILI9488_DC_CMD()  HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_RESET)
#define ILI9488_DC_DATA() HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_SET)

#define ILI9488_RST_LOW()  HAL_GPIO_WritePin(ILI9488_RST_GPIO_Port, ILI9488_RST_Pin, GPIO_PIN_RESET)
#define ILI9488_RST_HIGH() HAL_GPIO_WritePin(ILI9488_RST_GPIO_Port, ILI9488_RST_Pin, GPIO_PIN_SET)

static void ILI9488_SendCommand(uint8_t cmd) {
    ILI9488_DC_CMD();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
}

static void ILI9488_SendData(uint8_t data) {
    ILI9488_DC_DATA();
    HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
}



static void ILI9488_SendColor(uint16_t color) {
    uint8_t buffer[3];
    buffer[0] = (color & 0xF800) >> 8;
    buffer[1] = (color & 0x07E0) >> 3;
    buffer[2] = (color & 0x001F) << 3;
    ILI9488_DC_DATA();
    ILI9488_CS_LOW();
    HAL_SPI_Transmit(&hspi2, buffer, 3, HAL_MAX_DELAY);
    ILI9488_CS_HIGH();
}

void ILI9488_Init(void) {
    // Hardware Reset
    ILI9488_RST_HIGH();
    HAL_Delay(5);
    ILI9488_RST_LOW();
    HAL_Delay(20);
    ILI9488_RST_HIGH();
    HAL_Delay(150);

    ILI9488_CS_LOW();

    // Initialization sequence
    ILI9488_SendCommand(0xE0); // Positive Gamma Control
    ILI9488_SendData(0x00);
    ILI9488_SendData(0x03);
    ILI9488_SendData(0x09);
    ILI9488_SendData(0x08);
    ILI9488_SendData(0x16);
    ILI9488_SendData(0x0A);
    ILI9488_SendData(0x3F);
    ILI9488_SendData(0x78);
    ILI9488_SendData(0x4C);
    ILI9488_SendData(0x09);
    ILI9488_SendData(0x0A);
    ILI9488_SendData(0x08);
    ILI9488_SendData(0x16);
    ILI9488_SendData(0x1A);
    ILI9488_SendData(0x0F);

    ILI9488_SendCommand(0XE1); // Negative Gamma Control
    ILI9488_SendData(0x00);
    ILI9488_SendData(0x16);
    ILI9488_SendData(0x19);
    ILI9488_SendData(0x03);
    ILI9488_SendData(0x0F);
    ILI9488_SendData(0x05);
    ILI9488_SendData(0x32);
    ILI9488_SendData(0x45);
    ILI9488_SendData(0x46);
    ILI9488_SendData(0x04);
    ILI9488_SendData(0x0E);
    ILI9488_SendData(0x0D);
    ILI9488_SendData(0x35);
    ILI9488_SendData(0x37);
    ILI9488_SendData(0x0F);

    ILI9488_SendCommand(0XC0); // Power Control 1
    ILI9488_SendData(0x17);
    ILI9488_SendData(0x15);

    ILI9488_SendCommand(0xC1); // Power Control 2
    ILI9488_SendData(0x41);

    ILI9488_SendCommand(0xC5); // VCOM Control
    ILI9488_SendData(0x00);
    ILI9488_SendData(0x12);
    ILI9488_SendData(0x80);

    ILI9488_SendCommand(0x36); // Memory Access Control
    ILI9488_SendData(0x28); // Landscape: BGR, Horizontal

    ILI9488_SendCommand(0x3A); // Pixel Format Set
    ILI9488_SendData(0x66); // 18-bit / pixel (required for SPI)

    ILI9488_SendCommand(0xB0); // Interface Mode Control
    ILI9488_SendData(0x00);

    ILI9488_SendCommand(0xB1); // Frame Rate Control
    ILI9488_SendData(0xA0);

    ILI9488_SendCommand(0xB4); // Display Inversion Control
    ILI9488_SendData(0x02);

    ILI9488_SendCommand(0xB6); // Display Function Control
    ILI9488_SendData(0x02);
    ILI9488_SendData(0x02);
    ILI9488_SendData(0x3B);

    ILI9488_SendCommand(0xB7); // Entry Mode Set
    ILI9488_SendData(0xC6);

    ILI9488_SendCommand(0xF7); // Adjust Control 3
    ILI9488_SendData(0xA9);
    ILI9488_SendData(0x51);
    ILI9488_SendData(0x2C);
    ILI9488_SendData(0x82);

    ILI9488_SendCommand(0x11); // Exit Sleep
    ILI9488_CS_HIGH();
    HAL_Delay(120);

    ILI9488_CS_LOW();
    ILI9488_SendCommand(0x29); // Display on
    ILI9488_CS_HIGH();
    HAL_Delay(25);
}

void ILI9488_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ILI9488_CS_LOW();
    
    ILI9488_SendCommand(0x2A); // Column Address Set
    ILI9488_SendData(x0 >> 8);
    ILI9488_SendData(x0 & 0xFF);
    ILI9488_SendData(x1 >> 8);
    ILI9488_SendData(x1 & 0xFF);

    ILI9488_SendCommand(0x2B); // Page Address Set
    ILI9488_SendData(y0 >> 8);
    ILI9488_SendData(y0 & 0xFF);
    ILI9488_SendData(y1 >> 8);
    ILI9488_SendData(y1 & 0xFF);

    ILI9488_SendCommand(0x2C); // Memory Write
    
    ILI9488_CS_HIGH();
}

void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    ILI9488_SetAddressWindow(x, y, x, y);
    ILI9488_SendColor(color);
}

void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    if ((x + w - 1) >= ILI9488_WIDTH) w = ILI9488_WIDTH - x;
    if ((y + h - 1) >= ILI9488_HEIGHT) h = ILI9488_HEIGHT - y;

    ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    
    uint8_t r = (color & 0xF800) >> 8;
    uint8_t g = (color & 0x07E0) >> 3;
    uint8_t b = (color & 0x001F) << 3;
    
    // Fill the DMA buffer once
    for (uint32_t i = 0; i < DMA_BUFFER_SIZE; i += 3) {
        dma_buffer[i] = r;
        dma_buffer[i+1] = g;
        dma_buffer[i+2] = b;
    }

    uint32_t total_bytes = (uint32_t)w * h * 3;
    
    ILI9488_DC_DATA();
    ILI9488_CS_LOW();
    
    while (total_bytes > 0) {
        uint16_t send_size = (total_bytes > DMA_BUFFER_SIZE) ? DMA_BUFFER_SIZE : total_bytes;
        ILI9488_TransmitDMA(dma_buffer, send_size);
        total_bytes -= send_size;
    }
    
    ILI9488_CS_HIGH();
}

void ILI9488_FillScreen(uint16_t color) {
    ILI9488_FillRectangle(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT, color);
}

void ILI9488_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    ILI9488_FillRectangle(x, y, w, 1, color);
    ILI9488_FillRectangle(x, y + h - 1, w, 1, color);
    ILI9488_FillRectangle(x, y, 1, h, color);
    ILI9488_FillRectangle(x + w - 1, y, 1, h, color);
}

void ILI9488_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    if (y0 == y1) {
        uint16_t start_x = (x0 < x1) ? x0 : x1;
        uint16_t width = (x0 < x1) ? (x1 - x0 + 1) : (x0 - x1 + 1);
        ILI9488_FillRectangle(start_x, y0, width, 1, color);
        return;
    }
    if (x0 == x1) {
        uint16_t start_y = (y0 < y1) ? y0 : y1;
        uint16_t height = (y0 < y1) ? (y1 - y0 + 1) : (y0 - y1 + 1);
        ILI9488_FillRectangle(x0, start_y, 1, height, color);
        return;
    }

    int16_t dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = (y1 >= y0) ? -(y1 - y0) : -(y0 - y1);
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;

    while (1) {
        ILI9488_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void ILI9488_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t b;
    
    // Check bounds
    if ((x + font.width >= ILI9488_WIDTH) || (y + font.height >= ILI9488_HEIGHT)) return;
    
    // Set window for this character
    ILI9488_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);
    
    uint16_t idx = 0;

    for (uint8_t i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (uint8_t j = 0; j < font.width; j++) {
            uint16_t mask = (font.width <= 8) ? (1 << (7 - j)) : (1 << (15 - j));
            uint16_t c = (b & mask) ? color : bgcolor;
            
            dma_buffer[idx++] = (c & 0xF800) >> 8;
            dma_buffer[idx++] = (c & 0x07E0) >> 3;
            dma_buffer[idx++] = (c & 0x001F) << 3;
            
            if (idx >= DMA_BUFFER_SIZE - 2) {
                ILI9488_DC_DATA();
                ILI9488_CS_LOW();
                ILI9488_TransmitDMA(dma_buffer, idx);
                ILI9488_CS_HIGH();
                idx = 0;
            }
        }
    }
    
    if (idx > 0) {
        ILI9488_DC_DATA();
        ILI9488_CS_LOW();
        ILI9488_TransmitDMA(dma_buffer, idx);
        ILI9488_CS_HIGH();
    }
}

void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
    while (*str) {
        if (x + font.width >= ILI9488_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height >= ILI9488_HEIGHT) break;
        }
        ILI9488_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
}

void ILI9488_WriteCharScaled(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale) {
    uint32_t b;
    
    if ((x + font.width * scale >= ILI9488_WIDTH) || (y + font.height * scale >= ILI9488_HEIGHT)) return;
    
    for (uint8_t i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (uint8_t j = 0; j < font.width; j++) {
            uint16_t mask = (font.width <= 8) ? (1 << (7 - j)) : (1 << (15 - j));
            uint16_t c = (b & mask) ? color : bgcolor;
            
            // Draw a scaled block of pixels for this original pixel
            ILI9488_FillRectangle(x + j * scale, y + i * scale, scale, scale, c);
        }
    }
}

void ILI9488_WriteStringScaled(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale) {
    while (*str) {
        if (x + font.width * scale >= ILI9488_WIDTH) {
            x = 0;
            y += font.height * scale;
            if (y + font.height * scale >= ILI9488_HEIGHT) break;
        }
        ILI9488_WriteCharScaled(x, y, *str, font, color, bgcolor, scale);
        x += font.width * scale;
        str++;
    }
}

void ILI9488_DrawBitmapLVGL(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const uint8_t *data) {
    uint32_t size = (x1 - x0 + 1) * (y1 - y0 + 1) * 3;
    
    ILI9488_SetAddressWindow(x0, y0, x1, y1);
    ILI9488_DC_DATA();
    ILI9488_CS_LOW();
    
    uint32_t offset = 0;
    while (size > 0) {
        uint16_t chunk = (size > 65535) ? 65535 : size;
        ILI9488_TransmitDMA((uint8_t*)(data + offset), chunk);
        offset += chunk;
        size -= chunk;
    }
    
    ILI9488_CS_HIGH();
}

void ILI9488_DrawCameraFrame(uint16_t *frame_buf) {
    // 480x320 window (Full Screen)
    ILI9488_SetAddressWindow(0, 0, 479, 319);
    ILI9488_DC_DATA();
    ILI9488_CS_LOW();
    
    static uint8_t spi_buf[1440]; // 480 pixels * 3 bytes
    
    for (int y = 0; y < 160; y++) {
        int idx = 0;
        for (int x = 0; x < 240; x++) {
            uint16_t p = frame_buf[y * 240 + x];
            
            // DCMI packs bytes in little-endian, but OV7670 sends High Byte first.
            // We must swap the bytes to recover the correct RGB565 pixel.
            p = (p >> 8) | (p << 8);

            uint8_t r = (p & 0xF800) >> 8;
            uint8_t g = (p & 0x07E0) >> 3;
            uint8_t b = (p & 0x001F) << 3;
            
            // Pixel 1
            spi_buf[idx++] = r;
            spi_buf[idx++] = g;
            spi_buf[idx++] = b;
            // Pixel 2 (scaled by 2 horizontally)
            spi_buf[idx++] = r;
            spi_buf[idx++] = g;
            spi_buf[idx++] = b;
        }
        
        // Send line twice (scaled by 2 vertically)
        ILI9488_TransmitDMA(spi_buf, 1440);
        ILI9488_TransmitDMA(spi_buf, 1440);
    }
    
    ILI9488_CS_HIGH();
}
