/**
 * @file    delay.h
 * @brief   Busy-wait delay functions for LPC2129 @ 60 MHz CCLK
 *
 * All delays are software loops calibrated for CCLK = 60 MHz.
 * If CCLK changes, update the multipliers in delay.c accordingly:
 *   delay_us : tdly * 12       (12 cycles ≈ 1 µs at 60 MHz / ~5 CPI)
 *   delay_ms : tdly * 12 000
 *   delay_s  : tdly * 12 000 000
 *
 * @warning These are blocking delays — CPU does nothing while waiting.
 *          For time-critical ISRs keep delay calls as short as possible.
 */

#ifndef DELAY_H
#define DELAY_H

#include "types.h"

void delay_us(u32 tdly);    /* Delay in microseconds */
void delay_ms(u32 tdly);    /* Delay in milliseconds */
void delay_s(u32 tdly);     /* Delay in seconds      */

#endif /* DELAY_H */
