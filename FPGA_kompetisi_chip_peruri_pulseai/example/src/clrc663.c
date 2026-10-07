/**
  ******************************************************************************
  * @file    clrc663.c
  * @brief   Driver implementation for NXP CLRC663 / CLRC66303 RFID Reader IC
  *          over SPI3 on STM32F405.
  ******************************************************************************
  */

#include "clrc663.h"
#include <string.h>
#include <stdio.h>

static SPI_HandleTypeDef *clrc_hspi = NULL;

/* Hardware control macros */
#define CLRC663_CS_LOW()     HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET)
#define CLRC663_CS_HIGH()    HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET)

/* CLRC663 PDOWN pin is Active HIGH!
 * High = Power Down / Reset
 * Low  = Normal Operating Mode */
#define CLRC663_PDOWN_HIGH() HAL_GPIO_WritePin(SPI3_RST_GPIO_Port, SPI3_RST_Pin, GPIO_PIN_SET)
#define CLRC663_PDOWN_LOW()  HAL_GPIO_WritePin(SPI3_RST_GPIO_Port, SPI3_RST_Pin, GPIO_PIN_RESET)

/* Internal SPI Transfer helper */
static void CLRC663_SpiTransfer(const uint8_t *tx, uint8_t *rx, uint16_t len) {
    if (clrc_hspi == NULL || len == 0) return;

    if (rx == NULL) {
        uint8_t dummy[32];
        while (len > 0) {
            uint16_t chunk = (len > sizeof(dummy)) ? sizeof(dummy) : len;
            HAL_SPI_TransmitReceive(clrc_hspi, (uint8_t*)tx, dummy, chunk, 100);
            if (tx != NULL) tx += chunk;
            len -= chunk;
        }
    } else if (tx == NULL) {
        uint8_t zeros[32] = {0};
        while (len > 0) {
            uint16_t chunk = (len > sizeof(zeros)) ? sizeof(zeros) : len;
            HAL_SPI_TransmitReceive(clrc_hspi, zeros, rx, chunk, 100);
            rx += chunk;
            len -= chunk;
        }
    } else {
        HAL_SPI_TransmitReceive(clrc_hspi, (uint8_t*)tx, rx, len, 100);
    }
}

/* ========================================================================== */
/*                       LOW LEVEL REGISTER & FIFO ACCESS                     */
/* ========================================================================== */

uint8_t CLRC663_ReadReg(uint8_t reg) {
    uint8_t tx[2] = { (uint8_t)((reg << 1) | 0x01), 0x00 };
    uint8_t rx[2] = { 0x00, 0x00 };
    CLRC663_CS_LOW();
    CLRC663_SpiTransfer(tx, rx, 2);
    CLRC663_CS_HIGH();
    return rx[1];
}

void CLRC663_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t tx[2] = { (uint8_t)((reg << 1) & 0xFE), value };
    CLRC663_CS_LOW();
    CLRC663_SpiTransfer(tx, NULL, 2);
    CLRC663_CS_HIGH();
}

void CLRC663_WriteRegs(uint8_t reg, const uint8_t *values, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        CLRC663_WriteReg(reg + i, values[i]);
    }
}

void CLRC663_ReadFIFO(uint8_t *rx, uint16_t len) {
    if (rx == NULL || len == 0) return;
    uint8_t read_cmd = (CLRC663_REG_FIFODATA << 1) | 0x01;
    uint8_t discard = 0;

    CLRC663_CS_LOW();
    /* First byte: address byte */
    CLRC663_SpiTransfer(&read_cmd, &discard, 1);

    /* Intermediate bytes: pipeline read */
    for (uint16_t i = 1; i < len; i++) {
        CLRC663_SpiTransfer(&read_cmd, rx++, 1);
    }
    /* Final byte: dummy byte to complete the read */
    uint8_t dummy = 0;
    CLRC663_SpiTransfer(&dummy, rx++, 1);
    CLRC663_CS_HIGH();
}

void CLRC663_WriteFIFO(const uint8_t *data, uint16_t len) {
    if (data == NULL || len == 0) return;
    uint8_t write_cmd = (CLRC663_REG_FIFODATA << 1) & 0xFE;
    CLRC663_CS_LOW();
    CLRC663_SpiTransfer(&write_cmd, NULL, 1);
    CLRC663_SpiTransfer(data, NULL, len);
    CLRC663_CS_HIGH();
}

void CLRC663_FlushFIFO(void) {
    CLRC663_WriteReg(CLRC663_REG_FIFOCONTROL, 0xB0);
}

uint16_t CLRC663_GetFIFOLength(void) {
    return CLRC663_ReadReg(CLRC663_REG_FIFOLENGTH);
}

/* ========================================================================== */
/*                             COMMAND HELPERS                                */
/* ========================================================================== */

void CLRC663_Cmd_Idle(void) {
    CLRC663_WriteReg(CLRC663_REG_COMMAND, CLRC663_CMD_IDLE);
}

void CLRC663_Cmd_Transceive(const uint8_t *data, uint16_t len) {
    CLRC663_Cmd_Idle();
    CLRC663_FlushFIFO();
    CLRC663_WriteFIFO(data, len);
    CLRC663_WriteReg(CLRC663_REG_COMMAND, CLRC663_CMD_TRANSCEIVE);
}

void CLRC663_Cmd_LoadProtocol(uint8_t rx, uint8_t tx) {
    uint8_t params[2] = { rx, tx };
    CLRC663_Cmd_Idle();
    CLRC663_FlushFIFO();
    CLRC663_WriteFIFO(params, 2);
    CLRC663_WriteReg(CLRC663_REG_COMMAND, CLRC663_CMD_LOADPROTOCOL);
}

/* ========================================================================== */
/*                        TIMER & IRQ HELPERS                                 */
/* ========================================================================== */

void CLRC663_ClearIRQ0(void) {
    CLRC663_WriteReg(CLRC663_REG_IRQ0, (uint8_t)~(1 << 7));
}

void CLRC663_ClearIRQ1(void) {
    CLRC663_WriteReg(CLRC663_REG_IRQ1, (uint8_t)~(1 << 7));
}

uint8_t CLRC663_GetIRQ0(void) {
    return CLRC663_ReadReg(CLRC663_REG_IRQ0);
}

uint8_t CLRC663_GetIRQ1(void) {
    return CLRC663_ReadReg(CLRC663_REG_IRQ1);
}

void CLRC663_Timer_SetControl(uint8_t timer, uint8_t value) {
    CLRC663_WriteReg(CLRC663_REG_T0CONTROL + (5 * timer), value);
}

void CLRC663_Timer_SetReload(uint8_t timer, uint16_t value) {
    CLRC663_WriteReg(CLRC663_REG_T0RELOADHI + (5 * timer), (uint8_t)(value >> 8));
    CLRC663_WriteReg(CLRC663_REG_T0RELOADLO + (5 * timer), (uint8_t)(value & 0xFF));
}

void CLRC663_Timer_SetValue(uint8_t timer, uint16_t value) {
    CLRC663_WriteReg(CLRC663_REG_T0COUNTERVALHI + (5 * timer), (uint8_t)(value >> 8));
    CLRC663_WriteReg(CLRC663_REG_T0COUNTERVALLO + (5 * timer), (uint8_t)(value & 0xFF));
}

/* ========================================================================== */
/*                         HARDWARE & PROTOCOL SETUP                          */
/* ========================================================================== */

void CLRC663_HardReset(void) {
    CLRC663_CS_HIGH();
    /* PDOWN = HIGH (Power Down / Reset) */
    CLRC663_PDOWN_HIGH();
    HAL_Delay(5);
    /* PDOWN = LOW (Normal operation) */
    CLRC663_PDOWN_LOW();
    /* Wait for oscillator and regulator to stabilize (min 10ms) */
    HAL_Delay(15);
}

void CLRC663_SoftReset(void) {
    CLRC663_WriteReg(CLRC663_REG_COMMAND, CLRC663_CMD_SOFTRESET);
    HAL_Delay(10);
    CLRC663_Cmd_Idle();
}

uint8_t CLRC663_ReadVersion(void) {
    return CLRC663_ReadReg(CLRC663_REG_VERSION);
}

void CLRC663_AntennaOn(void) {
    uint8_t drvmod = CLRC663_ReadReg(CLRC663_REG_DRVMOD);
    CLRC663_WriteReg(CLRC663_REG_DRVMOD, drvmod | 0x80);
}

void CLRC663_AntennaOff(void) {
    uint8_t drvmod = CLRC663_ReadReg(CLRC663_REG_DRVMOD);
    CLRC663_WriteReg(CLRC663_REG_DRVMOD, drvmod & ~0x80);
}

CLRC663_Status_t CLRC663_SetupISO14443A(void) {
    /* 1. Stop current commands */
    CLRC663_Cmd_Idle();

    /* 2. Load ISO14443A 106 kbit/s Protocol from internal EEPROM */
    CLRC663_Cmd_LoadProtocol(CLRC663_PROTO_ISO14443A_106, CLRC663_PROTO_ISO14443A_106);
    HAL_Delay(5);

    /* 3. Recommended analog front-end settings from NXP AN11022 */
    static const uint8_t recom_14443a_106[] = {
        0x8E, /* 0x28: DrvMod - TxEn=1, differential antenna driver, 13.56 MHz carrier */
        0x08, /* 0x29: TxAmp */
        0x21, /* 0x2A: DrvCon */
        0x1A, /* 0x2B: TxI */
        0x18, /* 0x2C: TxCrcPreset */
        0x18, /* 0x2D: RxCrcCon */
        0x0F, /* 0x2E: TxDataNum */
        0x27, /* 0x2F: TxModWidth */
        0x00, /* 0x30: SymbolConfig */
        0xC0, /* 0x31: FrameCon */
        0x12, /* 0x32: RxBitMod */
        0xCF, /* 0x33: RxWait */
        0x00, /* 0x34: RxThreshold */
        0x04, /* 0x35: Rcv */
        0x90, /* 0x36: RxAna */
        0x32, /* 0x37: */
        0x12, /* 0x38: SerialSpeed */
        0x0A  /* 0x39: */
    };
    CLRC663_WriteRegs(CLRC663_REG_DRVMOD, recom_14443a_106, sizeof(recom_14443a_106));

    /* Ensure RF carrier is active */
    CLRC663_AntennaOn();

    return CLRC663_OK;
}

CLRC663_Status_t CLRC663_Init(SPI_HandleTypeDef *hspi) {
    clrc_hspi = hspi;

    /* Deselect chip and perform hardware reset */
    CLRC663_CS_HIGH();
    CLRC663_HardReset();

    /* Software reset */
    CLRC663_SoftReset();

    /* Verify chip communication by reading version */
    uint8_t version = CLRC663_ReadVersion();
    /* Version 0x1A: CLRC66303
     * Version 0x18: CLRC66301/02
     * If 0x00 or 0xFF: Communication failed */
    if (version == 0x00 || version == 0xFF) {
        return CLRC663_ERROR;
    }

    /* Configure ISO14443A protocol */
    return CLRC663_SetupISO14443A();
}

/* ========================================================================== */
/*                       ISO14443A CARD DETECTION                             */
/* ========================================================================== */

static uint16_t CLRC663_ISO14443A_WUPA_REQA(uint8_t instruction) {
    CLRC663_Cmd_Idle();
    CLRC663_FlushFIFO();

    /* REQA / WUPA is 7 bits long */
    CLRC663_WriteReg(CLRC663_REG_TXDATANUM, 7 | CLRC663_TXDATANUM_DATAEN);

    /* Disable CRC */
    CLRC663_WriteReg(CLRC663_REG_TXCRCPRESET, CLRC663_RECOM_14443A_CRC | CLRC663_CRC_OFF);
    CLRC663_WriteReg(CLRC663_REG_RXCRCCON, CLRC663_RECOM_14443A_CRC | CLRC663_CRC_OFF);
    CLRC663_WriteReg(CLRC663_REG_RXBITCTRL, 0);

    CLRC663_ClearIRQ0();
    CLRC663_ClearIRQ1();

    /* Enable IRQs */
    CLRC663_WriteReg(CLRC663_REG_IRQ0EN, CLRC663_IRQ0EN_RX_IRQEN | CLRC663_IRQ0EN_ERR_IRQEN);
    CLRC663_WriteReg(CLRC663_REG_IRQ1EN, CLRC663_IRQ1EN_TIMER0_IRQEN);

    /* Configure Timer 0 (~5 ms timeout) */
    CLRC663_Timer_SetControl(0, CLRC663_TCONTROL_CLK_211KHZ | CLRC663_TCONTROL_START_TX_END);
    CLRC663_Timer_SetReload(0, 1000);
    CLRC663_Timer_SetValue(0, 1000);

    /* Transmit REQA/WUPA command */
    uint8_t send_req = instruction;
    CLRC663_Cmd_Transceive(&send_req, 1);

    /* Wait for response or timeout */
    uint32_t start_time = HAL_GetTick();
    uint8_t irq1_val = 0;
    while (!(irq1_val & CLRC663_IRQ1_TIMER0_IRQ)) {
        irq1_val = CLRC663_GetIRQ1();
        if (irq1_val & CLRC663_IRQ1_GLOBAL_IRQ) {
            break;
        }
        if (HAL_GetTick() - start_time > 20) {
            break;
        }
    }
    CLRC663_Cmd_Idle();

    uint8_t irq0_val = CLRC663_GetIRQ0();
    if ((!(irq0_val & CLRC663_IRQ0_RX_IRQ)) || (irq0_val & CLRC663_IRQ0_ERR_IRQ)) {
        return 0;
    }

    uint8_t rx_len = (uint8_t)CLRC663_GetFIFOLength();
    if (rx_len == 2) {
        uint16_t atqa = 0;
        CLRC663_ReadFIFO((uint8_t*)&atqa, 2);
        return atqa;
    }

    return 0;
}

uint16_t CLRC663_ISO14443A_REQA(void) {
    return CLRC663_ISO14443A_WUPA_REQA(CLRC663_ISO14443_CMD_REQA);
}

uint16_t CLRC663_ISO14443A_WUPA(void) {
    return CLRC663_ISO14443A_WUPA_REQA(CLRC663_ISO14443_CMD_WUPA);
}

/* ========================================================================== */
/*                   ISO14443A ANTI-COLLISION & SELECT                        */
/* ========================================================================== */

uint8_t CLRC663_ISO14443A_Select(uint8_t *uid, uint8_t *sak) {
    if (uid == NULL || sak == NULL) return 0;

    CLRC663_Cmd_Idle();
    CLRC663_FlushFIFO();

    CLRC663_WriteReg(CLRC663_REG_IRQ0EN, CLRC663_IRQ0EN_RX_IRQEN | CLRC663_IRQ0EN_ERR_IRQEN);
    CLRC663_WriteReg(CLRC663_REG_IRQ1EN, CLRC663_IRQ1EN_TIMER0_IRQEN);

    CLRC663_Timer_SetControl(0, CLRC663_TCONTROL_CLK_211KHZ | CLRC663_TCONTROL_START_TX_END);
    CLRC663_Timer_SetReload(0, 1000);
    CLRC663_Timer_SetValue(0, 1000);

    for (uint8_t cascade_level = 1; cascade_level <= 3; cascade_level++) {
        uint8_t cmd = 0;
        switch (cascade_level) {
            case 1: cmd = CLRC663_ISO14443_CAS_LEVEL_1; break;
            case 2: cmd = CLRC663_ISO14443_CAS_LEVEL_2; break;
            case 3: cmd = CLRC663_ISO14443_CAS_LEVEL_3; break;
            default: return 0;
        }

        uint8_t known_bits = 0;
        uint8_t send_req[7] = {0};
        uint8_t *uid_this_level = &(send_req[2]);
        uint8_t message_length = 0;

        /* Disable CRC for collision resolution */
        CLRC663_WriteReg(CLRC663_REG_TXCRCPRESET, CLRC663_RECOM_14443A_CRC | CLRC663_CRC_OFF);
        CLRC663_WriteReg(CLRC663_REG_RXCRCCON, CLRC663_RECOM_14443A_CRC | CLRC663_CRC_OFF);

        /* Anti-collision resolution loop (max 32 iterations) */
        for (uint8_t coll_n = 0; coll_n < 32; coll_n++) {
            CLRC663_ClearIRQ0();
            CLRC663_ClearIRQ1();

            send_req[0] = cmd;
            send_req[1] = 0x20 + known_bits;

            CLRC663_WriteReg(CLRC663_REG_TXDATANUM, (known_bits % 8) | CLRC663_TXDATANUM_DATAEN);
            uint8_t rxalign = known_bits % 8;
            CLRC663_WriteReg(CLRC663_REG_RXBITCTRL, (rxalign << 4));

            if ((known_bits % 8) == 0) {
                message_length = (known_bits / 8) + 2;
            } else {
                message_length = (known_bits / 8) + 3;
            }

            CLRC663_Cmd_Transceive(send_req, message_length);

            /* Wait until complete or timeout */
            uint32_t t_start = HAL_GetTick();
            uint8_t irq1_val = 0;
            while (!(irq1_val & CLRC663_IRQ1_TIMER0_IRQ)) {
                irq1_val = CLRC663_GetIRQ1();
                if (irq1_val & CLRC663_IRQ1_GLOBAL_IRQ) break;
                if (HAL_GetTick() - t_start > 20) break;
            }
            CLRC663_Cmd_Idle();

            uint8_t irq0_val = CLRC663_GetIRQ0();
            uint8_t error = CLRC663_ReadReg(CLRC663_REG_ERROR);
            uint8_t coll = CLRC663_ReadReg(CLRC663_REG_RXCOLL);
            uint8_t collision_pos = 0;

            if (irq0_val & CLRC663_IRQ0_ERR_IRQ) {
                if (error & CLRC663_ERROR_COLLDET) {
                    if (coll & (1 << 7)) {
                        collision_pos = coll & (~(1 << 7));
                        uint8_t choice_pos = known_bits + collision_pos;
                        uint8_t selection = (uid[((choice_pos + (cascade_level - 1) * 3) / 8)] >> (choice_pos % 8)) & 1;
                        uid_this_level[choice_pos / 8] |= (selection << (choice_pos % 8));
                        known_bits++;
                    } else {
                        collision_pos = 0x20 - known_bits;
                    }
                } else {
                    collision_pos = 0x20 - known_bits;
                }
            } else if (irq0_val & CLRC663_IRQ0_RX_IRQ) {
                collision_pos = 0x20 - known_bits;
            } else {
                return 0; // No card response
            }

            uint8_t rx_len = (uint8_t)CLRC663_GetFIFOLength();
            uint8_t buf[5] = {0};
            CLRC663_ReadFIFO(buf, rx_len < 5 ? rx_len : 5);

            for (uint8_t r = 0; r < rx_len && ((known_bits / 8) + r < 5); r++) {
                uid_this_level[(known_bits / 8) + r] |= buf[r];
            }
            known_bits += collision_pos;

            if (known_bits >= 32) {
                break;
            }
        }

        /* Verify BCC (Block Check Character) */
        uint8_t bcc_val = uid_this_level[4];
        uint8_t bcc_calc = uid_this_level[0] ^ uid_this_level[1] ^ uid_this_level[2] ^ uid_this_level[3];
        if (bcc_val != bcc_calc) {
            return 0; // Checksum error
        }

        /* Send Select Command (NVB = 0x70) with CRC */
        CLRC663_ClearIRQ0();
        CLRC663_ClearIRQ1();

        send_req[0] = cmd;
        send_req[1] = 0x70;
        send_req[6] = bcc_calc;
        message_length = 7;

        CLRC663_WriteReg(CLRC663_REG_TXCRCPRESET, CLRC663_RECOM_14443A_CRC | CLRC663_CRC_ON);
        CLRC663_WriteReg(CLRC663_REG_RXCRCCON, CLRC663_RECOM_14443A_CRC | CLRC663_CRC_ON);
        CLRC663_WriteReg(CLRC663_REG_TXDATANUM, (known_bits % 8) | CLRC663_TXDATANUM_DATAEN);
        CLRC663_WriteReg(CLRC663_REG_RXBITCTRL, 0);

        CLRC663_Cmd_Transceive(send_req, message_length);

        uint32_t t_start = HAL_GetTick();
        uint8_t irq1_val = 0;
        while (!(irq1_val & CLRC663_IRQ1_TIMER0_IRQ)) {
            irq1_val = CLRC663_GetIRQ1();
            if (irq1_val & CLRC663_IRQ1_GLOBAL_IRQ) break;
            if (HAL_GetTick() - t_start > 20) break;
        }
        CLRC663_Cmd_Idle();

        uint8_t irq0_val = CLRC663_GetIRQ0();
        if (irq0_val & CLRC663_IRQ0_ERR_IRQ) {
            return 0;
        }

        uint8_t sak_len = (uint8_t)CLRC663_GetFIFOLength();
        if (sak_len != 1) {
            return 0;
        }
        uint8_t sak_value = 0;
        CLRC663_ReadFIFO(&sak_value, 1);

        if (sak_value & (1 << 2)) {
            /* UID not complete, cascade tag was byte 0 */
            for (uint8_t n = 0; n < 3; n++) {
                uid[(cascade_level - 1) * 3 + n] = uid_this_level[n + 1];
            }
        } else {
            /* UID complete */
            for (uint8_t n = 0; n < 4; n++) {
                uid[(cascade_level - 1) * 3 + n] = uid_this_level[n];
            }
            *sak = sak_value;
            return (cascade_level * 3 + 1);
        }
    }

    return 0;
}

/* ========================================================================== */
/*                     CARD TYPE CLASSIFICATION & SCAN                        */
/* ========================================================================== */

const char* CLRC663_GetCardTypeName(uint8_t sak, uint16_t atqa) {
    if (sak == 0x08) {
        return "MIFARE Classic 1K";
    } else if (sak == 0x18) {
        return "MIFARE Classic 4K";
    } else if (sak == 0x00) {
        return "MIFARE Ultralight / NTAG";
    } else if (sak == 0x20) {
        return "MIFARE DESFire / ISO14443-4";
    } else if (sak == 0x28) {
        return "e-KTP / ISO14443-4";
    } else if (sak == 0x09) {
        return "MIFARE Mini 0.3K";
    } else if (sak == 0x88) {
        return "Infineon Classic 1K";
    } else if ((sak & 0x20) != 0) {
        return "ISO14443-4 Card";
    } else {
        return "ISO14443A Card";
    }
}

bool CLRC663_ReadCard(CLRC663_Card_t *card) {
    if (card == NULL) return false;

    /* Step 1: Detect card via REQA or WUPA */
    uint16_t atqa = CLRC663_ISO14443A_REQA();
    if (atqa == 0) {
        atqa = CLRC663_ISO14443A_WUPA();
    }

    if (atqa != 0) {
        /* Step 2: Anti-collision & Select card to retrieve UID and SAK */
        uint8_t sak = 0;
        uint8_t uid[10] = {0};
        uint8_t uid_len = CLRC663_ISO14443A_Select(uid, &sak);

        if (uid_len > 0) {
            card->uid_len = uid_len;
            memcpy(card->uid, uid, uid_len);
            card->atqa = atqa;
            card->sak = sak;
            const char *type = CLRC663_GetCardTypeName(sak, atqa);
            strncpy(card->type_name, type, sizeof(card->type_name) - 1);
            card->type_name[sizeof(card->type_name) - 1] = '\0';
            return true;
        }
    }

    return false;
}
