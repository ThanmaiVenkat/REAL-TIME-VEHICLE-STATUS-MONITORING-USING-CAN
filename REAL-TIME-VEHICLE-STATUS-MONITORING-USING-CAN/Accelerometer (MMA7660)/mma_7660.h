/**
 * @file    mma_7660.h
 * @brief   MMA7660FC 3-axis accelerometer driver interface
 *
 * The MMA7660FC communicates over I2C at address 0x4C (7-bit).
 * Each axis register returns a signed 6-bit value (range ±1.5g).
 *
 * I2C addresses:
 *   Write : 0x98  (0x4C << 1 | 0)
 *   Read  : 0x99  (0x4C << 1 | 1)
 *
 * Usage in this project:
 *   mma7660_init();          // call once after i2c_init()
 *   x = mma7660_get_x();    // returns signed value, ±31 counts ≈ ±1.5g
 *   Threshold used for airbag: |axis| >= 25 counts
 */

#ifndef MMA7660_H
#define MMA7660_H

#include "types.h"

/* ── I2C addresses ─────────────────────────────────────────── */
#define MMA7660_SA      0x98    /* Slave address + WRITE bit    */
#define MMA7660_SA_R    0x99    /* Slave address + READ  bit    */

/* ── Register map ──────────────────────────────────────────── */
#define MMA_XOUT    0x00    /* X-axis output (6-bit signed)     */
#define MMA_YOUT    0x01    /* Y-axis output (6-bit signed)     */
#define MMA_ZOUT    0x02    /* Z-axis output (6-bit signed)     */
#define MMA_TILT    0x03    /* Tilt status register             */
#define MMA_SRST    0x04    /* Sampling rate status             */
#define MMA_SPCNT   0x05    /* Sleep count register             */
#define MMA_INTSU   0x06    /* Interrupt setup register         */
#define MMA_MODE    0x07    /* Mode register: 0=standby, 1=active */
#define MMA_SR      0x08    /* Sample rate register             */

/* ── Function prototypes ───────────────────────────────────── */

/**
 * @brief  Initialise MMA7660: standby → configure → active mode.
 *         Sets sample rate to 120 samples/sec (SR register = 0x00).
 */
void mma7660_init(void);

/**
 * @brief  Write one byte to a MMA7660 register over I2C.
 */
void mma7660_write(u8 reg, u8 data);

/**
 * @brief  Read one byte from a MMA7660 register over I2C.
 * @return Register value (raw, unformatted).
 */
u8 mma7660_read(u8 reg);

/**
 * @brief  Read X-axis acceleration (signed, 6-bit, ±31 counts).
 */
s8 mma7660_get_x(void);

/**
 * @brief  Read Y-axis acceleration (signed, 6-bit, ±31 counts).
 */
s8 mma7660_get_y(void);

/**
 * @brief  Read Z-axis acceleration (signed, 6-bit, ±31 counts).
 */
s8 mma7660_get_z(void);

#endif /* MMA7660_H */
