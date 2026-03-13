/**
 * @file    INDICATOR.c
 * @brief   8-LED sweep animation for turn-indicator display (LPC2129)
 *
 * Eight LEDs on P0.0-P0.7 simulate a sweeping indicator light.
 *   left_swipe  — lights travel from right to left  (pos increments)
 *   right_swipe — lights travel from left  to right (pos decrements)
 *   led_off     — all LEDs off
 *
 * @note    BUG FIXED: original code used undeclared variable "position"
 *          instead of the declared static "pos" — unified to use "pos".
 *
 * Call led_left_step() or led_right_step() repeatedly from the main
 * loop to advance the animation one step at a time (100 ms per step).
 */

#include <LPC21XX.h>
#include "delay.h"
#include "types.h"

/* ── LED field definition ──────────────────────────────────── */
#define LED_START   0               /* P0.0 = first LED                 */
#define INDICATOR   (0xFF << LED_START) /* 8-LED mask on P0.0-P0.7     */

/* ── Animation state ───────────────────────────────────────── */
static s32 pos = 0;     /* Current lit LED position (0-7)              */

/* ─────────────────────────────────────────────────────────────
 * led_left_step — light one LED, then advance position left
 * ───────────────────────────────────────────────────────────── */
void led_left_step(void)
{
    IODIR0 |= INDICATOR;            /* ensure all 8 pins are outputs    */
    IOCLR0  = INDICATOR;            /* turn all 8 LEDs off              */
    IOSET0  = (1 << (LED_START + pos)); /* light the current LED        */

    pos++;                          /* advance to next position         */
    if (pos >= 8) pos = 0;          /* wrap around after LED 7          */

    delay_ms(100);                  /* 100 ms per step = sweep speed    */
}

/* ─────────────────────────────────────────────────────────────
 * led_right_step — light one LED, then advance position right
 * ───────────────────────────────────────────────────────────── */
void led_right_step(void)
{
    IODIR0 |= INDICATOR;            /* ensure all 8 pins are outputs    */
    IOCLR0  = INDICATOR;            /* turn all 8 LEDs off              */
    IOSET0  = (1 << (LED_START + pos)); /* light the current LED        */

    pos--;                          /* advance to previous position     */
    if (pos < 0) pos = 7;           /* wrap around before LED 0         */

    delay_ms(100);
}

/* ─────────────────────────────────────────────────────────────
 * led_off — clear all indicator LEDs
 * Called when mode is MODE_OFF in MAIN_CAN_RX.
 * ───────────────────────────────────────────────────────────── */
void led_off(void)
{
    IODIR0 |= INDICATOR;            /* ensure pins are outputs          */
    IOCLR0  = INDICATOR;            /* turn all LEDs off                */
    pos = 0;                        /* reset position for clean restart */
}
