/**
 * @file    MMA_7660.c
 * @brief   MMA7660FC 3-axis accelerometer driver for LPC2129
 *
 * All communication uses the I2C master driver (I2C.c).
 * The MMA7660 returns 6-bit two's-complement values per axis.
 * convert_data() sign-extends them to 8-bit s8 for easy comparison.
 *
 * @note    BUG FIXED: the original mma7660_get_y() had a rogue
 *          #include <LPC21xx.H> inside the function body — removed.
 */

#include <LPC21xx.h>
#include "i2c.h"
#include "delay.h"
#include "mma_7660.h"

/* ─────────────────────────────────────────────────────────────
 * mma7660_write — write one register over I2C
 * ───────────────────────────────────────────────────────────── */
void mma7660_write(u8 reg, u8 data)
{
    i2c_start();
    i2c_write(MMA7660_SA);  /* slave address + WRITE bit */
    i2c_write(reg);         /* target register           */
    i2c_write(data);        /* value to write            */
    i2c_stop();
    delay_ms(2);            /* allow register to settle  */
}

/* ─────────────────────────────────────────────────────────────
 * mma7660_read — read one register over I2C
 * ───────────────────────────────────────────────────────────── */
u8 mma7660_read(u8 reg)
{
    u8 data;

    /* Write phase: send slave address + register pointer */
    i2c_start();
    i2c_write(MMA7660_SA);  /* slave address + WRITE     */
    i2c_write(reg);         /* register to read from     */

    /* Read phase: repeated START then slave address + READ */
    i2c_restart();
    i2c_write(MMA7660_SA_R); /* slave address + READ     */

    data = i2c_nack();       /* read single byte, send NACK to end */

    i2c_stop();
    delay_ms(2);

    return data;
}

/* ─────────────────────────────────────────────────────────────
 * mma7660_init — put device into active mode at 120 sps
 * ───────────────────────────────────────────────────────────── */
void mma7660_init(void)
{
    delay_ms(10);                       /* Power-on settling time           */

    mma7660_write(MMA_MODE, 0x00);      /* Step 1: enter standby mode       */
    delay_ms(10);

    mma7660_write(MMA_SR, 0x00);        /* Step 2: set 120 samples/sec      */

    mma7660_write(MMA_MODE, 0x01);      /* Step 3: activate (leave standby) */
    delay_ms(10);
}

/* ─────────────────────────────────────────────────────────────
 * convert_data — sign-extend 6-bit raw value to signed 8-bit
 *
 * The MMA7660 stores data as 6-bit two's complement in bits[5:0].
 * Bit 5 is the sign bit.  To use it as s8:
 *   mask to 6 bits, then if bit5=1 set the upper 2 bits (0xC0)
 *   so the value becomes a proper negative s8.
 * ───────────────────────────────────────────────────────────── */
static s8 convert_data(u8 val)
{
    val &= 0x3F;            /* Keep only bits[5:0] (6-bit data)    */

    if (val & 0x20)         /* Bit 5 = sign bit: negative number   */
        val |= 0xC0;        /* Sign-extend to 8 bits               */

    return (s8)val;
}

/* ─────────────────────────────────────────────────────────────
 * Axis read functions
 * ───────────────────────────────────────────────────────────── */

s8 mma7660_get_x(void)
{
    return convert_data(mma7660_read(MMA_XOUT));
}

s8 mma7660_get_y(void)
{
    /* NOTE: original code had "#include <LPC21xx.H>" here — removed (bug fix) */
    return convert_data(mma7660_read(MMA_YOUT));
}

s8 mma7660_get_z(void)
{
    return convert_data(mma7660_read(MMA_ZOUT));
}
