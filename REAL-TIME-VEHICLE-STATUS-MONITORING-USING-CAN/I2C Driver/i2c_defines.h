/**
 * @file    i2c_defines.h
 * @brief   I2C peripheral configuration macros for LPC2129
 *
 * Targets I2C standard mode at 100 kHz.
 * The divider is split equally between high/low SCL periods (50/50 duty).
 *
 * Clock chain:
 *   FOSC=12 MHz → CCLK=60 MHz (PLL×5) → PCLK=15 MHz (÷4)
 *   DIVIDER = (PCLK / I2C_SPEED) / 2 = (15 000 000 / 100 000) / 2 = 75
 *   I2SCLH = I2SCLL = 75
 */

#ifndef I2C_DEFINES_H
#define I2C_DEFINES_H

/* ── System clock chain ────────────────────────────────────── */
#define FOSC        12000000            /* Crystal: 12 MHz              */
#define CCLK        (FOSC * 5)         /* CPU clock: 60 MHz            */
#define PCLK        (CCLK / 4)         /* Peripheral clock: 15 MHz     */

/* ── I2C speed ─────────────────────────────────────────────── */
#define I2C_SPEED   100000             /* Standard mode: 100 kHz       */
#define DIVIDER     ((PCLK / I2C_SPEED) / 2)  /* = 75 (loaded into I2SCLH/I2SCLL) */

/* ── I2CONSET bit positions (set bits by writing 1) ────────── */
#define AA_BIT      2   /* Assert acknowledge                          */
#define SI_BIT      3   /* I2C interrupt flag (1 = event occurred)     */
#define STO_BIT     4   /* STOP condition                              */
#define STA_BIT     5   /* START condition                             */
#define I2EN_BIT    6   /* I2C interface enable                        */

/* ── I2CONCLR bit positions (clear bits by writing 1) ──────── */
#define AAC_BIT     2   /* Clear AA bit                                */
#define SIC_BIT     3   /* Clear SI (acknowledge interrupt)            */
/* STO has no clear bit — hardware clears it automatically */
#define STAC_BIT    5   /* Clear STA bit                               */
#define I2ENC_BIT   6   /* Clear I2EN (disable I2C)                    */

/* ── PINSEL0 masks for I2C pins ────────────────────────────── */
#define SCL_0_2     0x00000010  /* P0.2 → SCL (bits 5:4 = 01)         */
#define SDA_0_3     0x00000040  /* P0.3 → SDA (bits 7:6 = 01)         */

#endif /* I2C_DEFINES_H */
