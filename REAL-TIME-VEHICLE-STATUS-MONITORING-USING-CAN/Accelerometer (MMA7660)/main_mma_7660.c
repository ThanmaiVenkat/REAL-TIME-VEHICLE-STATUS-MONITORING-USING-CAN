/**
 * @file    MAIN_MMA_7660.c
 * @brief   Airbag trigger logic using MMA7660FC accelerometer (LPC2129)
 *
 * ── Function ───────────────────────────────────────────────────────────
 * airbag_trigger() is called from the MAIN NODE main loop and from
 * the INDICATOR_GEN node ISRs.  It reads all three accelerometer axes
 * and activates the airbag output if any axis exceeds the impact threshold.
 *
 * ── Impact detection ───────────────────────────────────────────────────
 * The MMA7660 returns 6-bit signed values (±31 counts ≈ ±1.5g).
 * Threshold = ±25 counts ≈ ±1.2g  (change AIRBAG_THRESHOLD to tune).
 * Any single axis breaching the threshold counts as an impact.
 *
 * ── Output ─────────────────────────────────────────────────────────────
 *   P0.21 (GLED) → green LED: ON normally, OFF on impact
 *   P0.22 (RLED) → red   LED: ON normally, OFF on impact
 *   LCD line 4   → "AIRBAG STATUS: BOOM!" during impact, then " OK! "
 *
 * ── Hardware ───────────────────────────────────────────────────────────
 *   MMA7660FC connected via I2C (I2C.c)
 *   LEDs active-low on P0.21 and P0.22
 */

#include <LPC21xx.h>
#include "delay.h"
#include "mma_7660.h"
#include "i2c.h"
#include "lcd.h"
#include "lcd_defines.h"

/* ── Pin definitions ──────────────────────────────────────── */
#define GLED    21      /* P0.21: green status LED (ON = safe)     */
#define RLED    22      /* P0.22: red   status LED (ON = safe)     */

/* ── Airbag detection threshold ───────────────────────────── */
#define AIRBAG_THRESHOLD    25  /* counts (±25 of ±31 max ≈ ±1.2g) */

/* ── LCD position for airbag status (line 4, pos 14) ──────── */
#define AIRBAG_LCD_POS  (0xD4 + 14)

/* ─────────────────────────────────────────────────────────────
 * airbag_trigger — read accelerometer and deploy airbag if needed
 *
 * Call this function periodically from the main loop.
 * It is non-blocking except for the 800 ms impact display delay.
 * ───────────────────────────────────────────────────────────── */
void airbag_trigger(void)
{
    s8 x, y, z;

    /* Configure LED pins as outputs */
    IODIR0 |= (1 << GLED) | (1 << RLED);

    /* Read all three axes from MMA7660 over I2C */
    x = mma7660_get_x();
    y = mma7660_get_y();
    z = mma7660_get_z();

    /* Check if any axis exceeds the impact threshold */
    if ((x <= -AIRBAG_THRESHOLD || x >= AIRBAG_THRESHOLD) ||
        (y <= -AIRBAG_THRESHOLD || y >= AIRBAG_THRESHOLD) ||
        (z <= -AIRBAG_THRESHOLD || z >= AIRBAG_THRESHOLD))
    {
        /* ── IMPACT DETECTED ── */

        /* Turn off both status LEDs (active-low: IOCLR = ON, but LEDs off) */
        IOCLR0 = (1 << GLED);
        IOCLR0 = (1 << RLED);

        /* Show airbag deployed message on LCD line 4 */
        lcd_cmd(AIRBAG_LCD_POS);
        lcd_char(7);                /* CGRAM char 7 = border box symbol  */

        delay_ms(800);              /* hold display for 800 ms            */

        /* Restore status LEDs */
        IOSET0 = (1 << GLED);
        IOSET0 = (1 << RLED);

        /* Clear the airbag alert from LCD (overwrite with spaces) */
        lcd_cmd(AIRBAG_LCD_POS - 1);
        lcd_str("      ");          /* 6 spaces to clear the symbol area  */
    }
    /* No else: if no impact, LEDs and LCD status remain unchanged.
     * The normal "STATUS: OK!" text is written by display_static_labels()
     * in MAIN_CAN_TX.c and is only updated here on impact. */
}
