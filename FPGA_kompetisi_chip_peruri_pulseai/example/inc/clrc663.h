/**
  ******************************************************************************
  * @file    clrc663.h
  * @brief   Driver header for NXP CLRC663 / CLRC66303 RFID Reader IC over SPI3
  ******************************************************************************
  */

#ifndef __CLRC663_H
#define __CLRC663_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"
#include "clrc663_def.h"
#include <stdbool.h>

/**
 * @brief CLRC663 Execution Status
 */
typedef enum {
    CLRC663_OK = 0,
    CLRC663_ERROR,
    CLRC663_TIMEOUT,
    CLRC663_COLLISION,
    CLRC663_NOT_FOUND
} CLRC663_Status_t;

/**
 * @brief Detected RFID Card Information
 */
typedef struct {
    uint8_t  uid[10];           /*!< Card Unique ID (4, 7, or 10 bytes) */
    uint8_t  uid_len;          /*!< Length of UID in bytes (4, 7, or 10) */
    uint16_t atqa;             /*!< Answer to Request A */
    uint8_t  sak;              /*!< Select Acknowledge byte */
    char     type_name[32];    /*!< Human-readable card type string */
} CLRC663_Card_t;

/* Initialization and Hardware Control */
CLRC663_Status_t CLRC663_Init(SPI_HandleTypeDef *hspi);
uint8_t          CLRC663_ReadVersion(void);
void             CLRC663_HardReset(void);
void             CLRC663_SoftReset(void);
void             CLRC663_AntennaOn(void);
void             CLRC663_AntennaOff(void);
CLRC663_Status_t CLRC663_SetupISO14443A(void);

/* Card Detection & ISO14443A Anti-collision */
uint16_t         CLRC663_ISO14443A_REQA(void);
uint16_t         CLRC663_ISO14443A_WUPA(void);
uint8_t          CLRC663_ISO14443A_Select(uint8_t *uid, uint8_t *sak);
bool             CLRC663_ReadCard(CLRC663_Card_t *card);
const char*      CLRC663_GetCardTypeName(uint8_t sak, uint16_t atqa);

/* Low-level Register and FIFO API */
uint8_t          CLRC663_ReadReg(uint8_t reg);
void             CLRC663_WriteReg(uint8_t reg, uint8_t value);
void             CLRC663_WriteRegs(uint8_t reg, const uint8_t *values, uint8_t len);
void             CLRC663_ReadFIFO(uint8_t *rx, uint16_t len);
void             CLRC663_WriteFIFO(const uint8_t *data, uint16_t len);
void             CLRC663_FlushFIFO(void);
uint16_t         CLRC663_GetFIFOLength(void);

/* Command helpers */
void             CLRC663_Cmd_Idle(void);
void             CLRC663_Cmd_Transceive(const uint8_t *data, uint16_t len);
void             CLRC663_Cmd_LoadProtocol(uint8_t rx, uint8_t tx);

/* Timer & Interrupt helpers */
void             CLRC663_ClearIRQ0(void);
void             CLRC663_ClearIRQ1(void);
uint8_t          CLRC663_GetIRQ0(void);
uint8_t          CLRC663_GetIRQ1(void);
void             CLRC663_Timer_SetControl(uint8_t timer, uint8_t value);
void             CLRC663_Timer_SetReload(uint8_t timer, uint16_t value);
void             CLRC663_Timer_SetValue(uint8_t timer, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* __CLRC663_H */
