/**
  ******************************************************************************
  * @file    clrc663_def.h
  * @brief   Register definitions and constants for NXP CLRC663 / CLRC66303
  *          NFC and RFID frontend IC.
  ******************************************************************************
  */

#ifndef __CLRC663_DEF_H
#define __CLRC663_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ========================================================================== */
/*                           CLRC663 REGISTERS                                */
/* ========================================================================== */
#define CLRC663_REG_COMMAND             0x00    /*!< Command register */
#define CLRC663_REG_HOSTCTRL            0x01    /*!< Host control register */
#define CLRC663_REG_FIFOCONTROL         0x02    /*!< FIFO control register */
#define CLRC663_REG_WATERLEVEL          0x03    /*!< Water level register */
#define CLRC663_REG_FIFOLENGTH          0x04    /*!< FIFO length register */
#define CLRC663_REG_FIFODATA            0x05    /*!< FIFO data register */
#define CLRC663_REG_IRQ0                0x06    /*!< Interrupt register 0 */
#define CLRC663_REG_IRQ1                0x07    /*!< Interrupt register 1 */
#define CLRC663_REG_IRQ0EN              0x08    /*!< Interrupt enable register 0 */
#define CLRC663_REG_IRQ1EN              0x09    /*!< Interrupt enable register 1 */
#define CLRC663_REG_ERROR               0x0A    /*!< Error status register */
#define CLRC663_REG_STATUS              0x0B    /*!< Status register */
#define CLRC663_REG_RXBITCTRL           0x0C    /*!< Receiver bit control register */
#define CLRC663_REG_RXCOLL              0x0D    /*!< Receiver collision register */
#define CLRC663_REG_TCONTROL            0x0E    /*!< Timer control register */
#define CLRC663_REG_T0CONTROL           0x0F    /*!< Timer 0 control */
#define CLRC663_REG_T0RELOADHI          0x10    /*!< Timer 0 reload high */
#define CLRC663_REG_T0RELOADLO          0x11    /*!< Timer 0 reload low */
#define CLRC663_REG_T0COUNTERVALHI      0x12    /*!< Timer 0 counter value high */
#define CLRC663_REG_T0COUNTERVALLO      0x13    /*!< Timer 0 counter value low */
#define CLRC663_REG_T1CONTROL           0x14    /*!< Timer 1 control */
#define CLRC663_REG_T1RELOADHI          0x15    /*!< Timer 1 reload high */
#define CLRC663_REG_T1RELOADLO          0x16    /*!< Timer 1 reload low */
#define CLRC663_REG_T1COUNTERVALHI      0x17    /*!< Timer 1 counter value high */
#define CLRC663_REG_T1COUNTERVALLO      0x18    /*!< Timer 1 counter value low */
#define CLRC663_REG_T2CONTROL           0x19    /*!< Timer 2 control */
#define CLRC663_REG_T2RELOADHI          0x1A    /*!< Timer 2 reload high */
#define CLRC663_REG_T2RELOADLO          0x1B    /*!< Timer 2 reload low */
#define CLRC663_REG_T2COUNTERVALHI      0x1C    /*!< Timer 2 counter value high */
#define CLRC663_REG_T2COUNTERVALLO      0x1D    /*!< Timer 2 counter value low */
#define CLRC663_REG_T3CONTROL           0x1E    /*!< Timer 3 control */
#define CLRC663_REG_T3RELOADHI          0x1F    /*!< Timer 3 reload high */
#define CLRC663_REG_T3RELOADLO          0x20    /*!< Timer 3 reload low */
#define CLRC663_REG_T3COUNTERVALHI      0x21    /*!< Timer 3 counter value high */
#define CLRC663_REG_T3COUNTERVALLO      0x22    /*!< Timer 3 counter value low */
#define CLRC663_REG_T4CONTROL           0x23    /*!< Timer 4 control */
#define CLRC663_REG_T4RELOADHI          0x24    /*!< Timer 4 reload high */
#define CLRC663_REG_T4RELOADLO          0x25    /*!< Timer 4 reload low */
#define CLRC663_REG_T4COUNTERVALHI      0x26    /*!< Timer 4 counter value high */
#define CLRC663_REG_T4COUNTERVALLO      0x27    /*!< Timer 4 counter value low */
#define CLRC663_REG_DRVMOD              0x28    /*!< Driver mode register (TX driver & RF on) */
#define CLRC663_REG_TXAMP               0x29    /*!< Transmitter amplifier */
#define CLRC663_REG_DRVCON              0x2A    /*!< Driver configuration */
#define CLRC663_REG_TXL                 0x2B    /*!< Transmitter register */
#define CLRC663_REG_TXCRCPRESET         0x2C    /*!< Transmitter CRC preset */
#define CLRC663_REG_RXCRCCON            0x2D    /*!< Receiver CRC control */
#define CLRC663_REG_TXDATANUM           0x2E    /*!< Transmitter data number */
#define CLRC663_REG_TXMODWIDTH          0x2F    /*!< Transmitter modulation width */
#define CLRC663_REG_TXSYM10BURSTLEN     0x30    /*!< Transmitter symbol burst length */
#define CLRC663_REG_TXWAITCTRL          0x31    /*!< Transmitter wait control */
#define CLRC663_REG_TXWAITLO            0x32    /*!< Transmitter wait low */
#define CLRC663_REG_FRAMECON            0x33    /*!< Frame control */
#define CLRC663_REG_RXSOFD              0x34    /*!< Receiver SOF detection */
#define CLRC663_REG_RXCTRL              0x35    /*!< Receiver control */
#define CLRC663_REG_RXWAIT              0x36    /*!< Receiver wait */
#define CLRC663_REG_RXTHRESHOLD         0x37    /*!< Receiver threshold */
#define CLRC663_REG_RCV                 0x38    /*!< Receiver */
#define CLRC663_REG_RXANA               0x39    /*!< Receiver analog */
#define CLRC663_REG_SERIALSPEED         0x3B    /*!< Serial speed register */
#define CLRC663_REG_LFO_TRIMM           0x3C    /*!< Low-frequency oscillator trim */
#define CLRC663_REG_PLL_CTRL            0x3D    /*!< PLL control */
#define CLRC663_REG_PLL_DIVOUT          0x3E    /*!< PLL divider */
#define CLRC663_REG_LPCD_QMIN           0x3F    /*!< LPCD Q channel min */
#define CLRC663_REG_LPCD_QMAX           0x40    /*!< LPCD Q channel max */
#define CLRC663_REG_LPCD_IMIN           0x41    /*!< LPCD I channel min */
#define CLRC663_REG_LPCD_I_RESULT       0x42    /*!< LPCD I channel result */
#define CLRC663_REG_LPCD_Q_RESULT       0x43    /*!< LPCD Q channel result */
#define CLRC663_REG_PADEN               0x44    /*!< Pin enable register */
#define CLRC663_REG_PADOUT              0x45    /*!< Pin output register */
#define CLRC663_REG_PADIN               0x46    /*!< Pin input register */
#define CLRC663_REG_SIGOUT              0x47    /*!< SIGOUT pin control */
#define CLRC663_REG_VERSION             0x7F    /*!< Version register (0x1A for CLRC66303, 0x18 for CLRC66301/2) */

/* ========================================================================== */
/*                            CLRC663 COMMANDS                                */
/* ========================================================================== */
#define CLRC663_CMD_IDLE                0x00    /*!< Idle / cancel command */
#define CLRC663_CMD_LPCD                0x01    /*!< Low-power card detection */
#define CLRC663_CMD_LOADKEY             0x02    /*!< Load MIFARE key */
#define CLRC663_CMD_MFAUTHENT           0x03    /*!< MIFARE standard authentication */
#define CLRC663_CMD_RECEIVE             0x05    /*!< Activate receiver */
#define CLRC663_CMD_TRANSMIT            0x06    /*!< Transmit FIFO data */
#define CLRC663_CMD_TRANSCEIVE          0x07    /*!< Transmit data and enable receiver */
#define CLRC663_CMD_WRITEE2             0x08    /*!< Write byte to EEPROM */
#define CLRC663_CMD_WRITEE2PAGE         0x09    /*!< Write EEPROM page */
#define CLRC663_CMD_READE2              0x0A    /*!< Read EEPROM */
#define CLRC663_CMD_LOADREG             0x0C    /*!< Load registers from EEPROM */
#define CLRC663_CMD_LOADPROTOCOL        0x0D    /*!< Load protocol from EEPROM */
#define CLRC663_CMD_LOADKEYE2           0x0E    /*!< Load key from EEPROM */
#define CLRC663_CMD_STOREKEYE2          0x0F    /*!< Store key into EEPROM */
#define CLRC663_CMD_READRNR             0x1C    /*!< Read random numbers */
#define CLRC663_CMD_SOFTRESET           0x1F    /*!< Soft reset */

/* ========================================================================== */
/*                             STATUS & IRQ BITS                              */
/* ========================================================================== */
// IRQ0 register bits
#define CLRC663_IRQ0_SET                (1 << 7)
#define CLRC663_IRQ0_HIALERT_IRQ        (1 << 6)
#define CLRC663_IRQ0_LOALERT_IRQ        (1 << 5)
#define CLRC663_IRQ0_IDLE_IRQ           (1 << 4)
#define CLRC663_IRQ0_TX_IRQ             (1 << 3)
#define CLRC663_IRQ0_RX_IRQ             (1 << 2)
#define CLRC663_IRQ0_ERR_IRQ            (1 << 1)
#define CLRC663_IRQ0_RXSOF_IRQ          (1 << 0)

// IRQ1 register bits
#define CLRC663_IRQ1_SET                (1 << 7)
#define CLRC663_IRQ1_GLOBAL_IRQ         (1 << 6)
#define CLRC663_IRQ1_LPCD_IRQ           (1 << 5)
#define CLRC663_IRQ1_TIMER4_IRQ         (1 << 4)
#define CLRC663_IRQ1_TIMER3_IRQ         (1 << 3)
#define CLRC663_IRQ1_TIMER2_IRQ         (1 << 2)
#define CLRC663_IRQ1_TIMER1_IRQ         (1 << 1)
#define CLRC663_IRQ1_TIMER0_IRQ         (1 << 0)

// IRQ0EN register bits
#define CLRC663_IRQ0EN_IRQ_INV          (1 << 7)
#define CLRC663_IRQ0EN_HIALERT_IRQEN    (1 << 6)
#define CLRC663_IRQ0EN_LOALERT_IRQEN    (1 << 5)
#define CLRC663_IRQ0EN_IDLE_IRQEN       (1 << 4)
#define CLRC663_IRQ0EN_TX_IRQEN         (1 << 3)
#define CLRC663_IRQ0EN_RX_IRQEN         (1 << 2)
#define CLRC663_IRQ0EN_ERR_IRQEN        (1 << 1)
#define CLRC663_IRQ0EN_RXSOF_IRQEN      (1 << 0)

// IRQ1EN register bits
#define CLRC663_IRQ1EN_IRQ_PUSHPULL     (1 << 7)
#define CLRC663_IRQ1EN_IRQ_PINEN        (1 << 6)
#define CLRC663_IRQ1EN_LPCD_IRQEN       (1 << 5)
#define CLRC663_IRQ1EN_TIMER4_IRQEN     (1 << 4)
#define CLRC663_IRQ1EN_TIMER3_IRQEN     (1 << 3)
#define CLRC663_IRQ1EN_TIMER2_IRQEN     (1 << 2)
#define CLRC663_IRQ1EN_TIMER1_IRQEN     (1 << 1)
#define CLRC663_IRQ1EN_TIMER0_IRQEN     (1 << 0)

// ERROR register bits
#define CLRC663_ERROR_EE_ERR            (1 << 7)
#define CLRC663_ERROR_FIFOWRERR         (1 << 6)
#define CLRC663_ERROR_FIFOOVL           (1 << 5)
#define CLRC663_ERROR_MINFRAMEERR       (1 << 4)
#define CLRC663_ERROR_NODATAERR         (1 << 3)
#define CLRC663_ERROR_COLLDET           (1 << 2)
#define CLRC663_ERROR_PROTERR           (1 << 1)
#define CLRC663_ERROR_INTEGERR          (1 << 0)

// TIMER Control bits
#define CLRC663_TCONTROL_STOPRX         (1 << 7)
#define CLRC663_TCONTROL_START_NOT      (0b00 << 4)
#define CLRC663_TCONTROL_START_TX_END   (0b01 << 4)
#define CLRC663_TCONTROL_START_LFO_WO   (0b10 << 4)
#define CLRC663_TCONTROL_START_LFO_WITH (0b11 << 4)
#define CLRC663_TCONTROL_AUTO_RESTART   (1 << 3)
#define CLRC663_TCONTROL_CLK_13MHZ      (0b00)
#define CLRC663_TCONTROL_CLK_211KHZ     (0b01)

// TXDATANUM bits
#define CLRC663_TXDATANUM_DATAEN        (1 << 3)

// CRC & Protocol presets
#define CLRC663_RECOM_14443A_CRC        0x18
#define CLRC663_CRC_ON                  1
#define CLRC663_CRC_OFF                 0

#define CLRC663_PROTO_ISO14443A_106     0
#define CLRC663_PROTO_ISO14443A_212     1
#define CLRC663_PROTO_ISO14443A_424     2
#define CLRC663_PROTO_ISO14443A_848     3

/* ========================================================================== */
/*                             ISO14443A COMMANDS                             */
/* ========================================================================== */
#define CLRC663_ISO14443_CMD_REQA       0x26
#define CLRC663_ISO14443_CMD_WUPA       0x52
#define CLRC663_ISO14443_CAS_LEVEL_1    0x93
#define CLRC663_ISO14443_CAS_LEVEL_2    0x95
#define CLRC663_ISO14443_CAS_LEVEL_3    0x97

// MIFARE Commands
#define CLRC663_MF_AUTH_KEY_A           0x60
#define CLRC663_MF_AUTH_KEY_B           0x61
#define CLRC663_MF_CMD_READ             0x30
#define CLRC663_MF_CMD_WRITE            0xA0
#define CLRC663_MF_ACK                  0x0A

#ifdef __cplusplus
}
#endif

#endif /* __CLRC663_DEF_H */
