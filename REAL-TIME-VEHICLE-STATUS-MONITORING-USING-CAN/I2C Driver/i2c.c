/**
 * @file    I2C.c
 * @brief   I2C master driver for LPC2129 (polling, standard mode 100 kHz)
 *
 * All functions poll the SI (serial interrupt) flag in I2CONSET.
 * SI is set by hardware after each I2C event (START sent, byte shifted,
 * ACK received, etc.).  The driver clears SI via I2CONCLR after handling
 * each event to allow the next event to proceed.
 *
 * No timeout guards are used here — if the bus hangs (e.g. slave holds
 * SDA low) the MCU will loop forever.  Add watchdog resets for production.
 */

#include <LPC21xx.h>
#include "delay.h"
#include "types.h"
#include "defines.h"
#include "i2c_defines.h"
#include "mma7660.h"

/* ─────────────────────────────────────────────────────────────
 * i2c_init — configure I2C1 peripheral
 * ───────────────────────────────────────────────────────────── */
void i2c_init(void)
{
    /* Route P0.2 → SCL and P0.3 → SDA via PINSEL0 */
    PINSEL0 |= (SCL_0_2 | SDA_0_3);

    /* Set SCL high/low periods: both = DIVIDER → 50% duty, 100 kHz */
    I2SCLH = DIVIDER;
    I2SCLL = DIVIDER;

    /* Enable the I2C interface (I2EN = 1) */
    I2CONSET = (1 << I2EN_BIT);
}

/* ─────────────────────────────────────────────────────────────
 * i2c_start — generate START condition
 * ───────────────────────────────────────────────────────────── */
void i2c_start(void)
{
    I2CONSET = (1 << STA_BIT);              /* Request START                */
    while (READBIT(I2CONSET, SI_BIT) == 0); /* Wait until START is sent     */
    I2CONCLR = (1 << STAC_BIT);            /* Clear STA bit                */
}

/* ─────────────────────────────────────────────────────────────
 * i2c_restart — generate repeated START (no STOP in between)
 * ───────────────────────────────────────────────────────────── */
void i2c_restart(void)
{
    I2CONSET = (1 << STA_BIT);              /* Request repeated START       */
    I2CONCLR = (1 << SIC_BIT);             /* Clear SI to continue bus     */
    while (READBIT(I2CONSET, SI_BIT) == 0); /* Wait for START sent          */
    I2CONCLR = (1 << STAC_BIT);            /* Clear STA bit                */
}

/* ─────────────────────────────────────────────────────────────
 * i2c_stop — generate STOP condition
 * ───────────────────────────────────────────────────────────── */
void i2c_stop(void)
{
    I2CONSET = (1 << STO_BIT);  /* Request STOP — hardware clears STO when done */
    I2CONCLR = (1 << SIC_BIT); /* Clear SI to release the bus                  */
}

/* ─────────────────────────────────────────────────────────────
 * i2c_write — send one byte, wait for ACK/NACK
 * ───────────────────────────────────────────────────────────── */
void i2c_write(u8 dat)
{
    I2DAT = dat;                            /* Load byte into shift register */
    I2CONCLR = (1 << SIC_BIT);             /* Clear SI to start shifting    */
    while (READBIT(I2CONSET, SI_BIT) == 0); /* Wait until byte sent + ACK    */
}

/* ─────────────────────────────────────────────────────────────
 * i2c_nack — read last byte, send NACK (signals end of read)
 * ───────────────────────────────────────────────────────────── */
u8 i2c_nack(void)
{
    I2CONCLR = (1 << SIC_BIT);             /* Clear SI to clock in byte     */
    while (READBIT(I2CONSET, SI_BIT) == 0); /* Wait for byte received        */
    return I2DAT;                           /* Return received byte (NACK sent automatically) */
}

/* ─────────────────────────────────────────────────────────────
 * i2c_mack — read byte and send ACK (more bytes will follow)
 * ───────────────────────────────────────────────────────────── */
u8 i2c_mack(void)
{
    I2CONSET = (1 << AA_BIT);              /* Assert ACK after this byte    */
    I2CONCLR = (1 << SIC_BIT);            /* Clear SI to clock in byte     */
    while (READBIT(I2CONSET, SI_BIT) == 0); /* Wait for byte received       */
    I2CONCLR = (1 << AAC_BIT);            /* De-assert AA ready for NACK   */
    return I2DAT;                          /* Return received byte          */
}
