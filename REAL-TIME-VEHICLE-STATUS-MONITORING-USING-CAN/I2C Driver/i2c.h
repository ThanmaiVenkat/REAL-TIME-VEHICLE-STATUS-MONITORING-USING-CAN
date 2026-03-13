/**
 * @file    i2c.h
 * @brief   I2C master driver public interface for LPC2129
 *
 * Provides low-level I2C primitives. Higher-level device drivers
 * (e.g. MMA7660) build sequences from these calls:
 *
 *   Write sequence : i2c_start → i2c_write(addr) → i2c_write(reg)
 *                    → i2c_write(data) → i2c_stop
 *
 *   Read sequence  : i2c_start → i2c_write(addr|W) → i2c_write(reg)
 *                    → i2c_restart → i2c_write(addr|R)
 *                    → i2c_nack() → i2c_stop
 *
 * All functions are blocking — they poll the SI (interrupt) flag.
 */

#ifndef I2C_H
#define I2C_H

#include "types.h"

void i2c_init(void);        /* Initialise I2C peripheral at 100 kHz    */
void i2c_start(void);       /* Generate START condition, wait for SI    */
void i2c_restart(void);     /* Generate repeated START (no STOP first)  */
void i2c_stop(void);        /* Generate STOP condition                  */
void i2c_write(u8 dat);     /* Write one byte, wait for ACK             */
u8   i2c_nack(void);        /* Read last byte with NACK (end of read)   */
u8   i2c_mack(void);        /* Read byte with ACK (more bytes follow)   */

#endif /* I2C_H */
