/**
 * @file    delay.c
 * @brief   Busy-wait delay implementation for LPC2129 @ 60 MHz CCLK
 *
 * Loop multipliers calibrated assuming ~5 cycles per iteration:
 *   60 000 000 Hz / 5 CPI = 12 000 000 loops per second
 *   → 12 loops  ≈ 1 µs
 *   → 12 000    ≈ 1 ms
 *   → 12 000 000 ≈ 1 s
 *
 * @note  Compiler optimisation can shorten or remove empty loops.
 *        If timing drifts, add "volatile" to the loop variable or
 *        use a hardware timer instead.
 */

#include "types.h"

/* ── Microsecond delay ─────────────────────────────────────── */
void delay_us(u32 tdly)
{
    tdly *= 12;             /* scale: 1 µs ≈ 12 loop iterations */
    while (tdly--);
}

/* ── Millisecond delay ─────────────────────────────────────── */
void delay_ms(u32 tdly)
{
    tdly *= 12000;          /* scale: 1 ms ≈ 12 000 iterations  */
    while (tdly--);
}

/* ── Second delay ──────────────────────────────────────────── */
void delay_s(u32 tdly)
{
    tdly *= 12000000;       /* scale: 1 s  ≈ 12 000 000 iterations */
    while (tdly--);
}
