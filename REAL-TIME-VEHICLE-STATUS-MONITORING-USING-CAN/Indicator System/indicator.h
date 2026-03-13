/**
 * @file    indicator.h
 * @brief   Turn-indicator and airbag public interface (INDICATOR_GEN node)
 *
 * ── Node roles ─────────────────────────────────────────────────────────
 *   INDICATOR_GEN.c  —  TX node: button ISRs send CAN frames + drive LEDs
 *   INDICATOR.c      —  LED swipe animation (left_swipe / right_swipe)
 *   MAIN_MMA_7660.c  —  airbag_trigger: reads accelerometer, trips airbag
 *
 * ── CAN messages sent (ID = 1) ─────────────────────────────────────────
 *   DATA1 = 0x01  → left  indicator pressed
 *   DATA1 = 0x02  → right indicator pressed
 *
 * ── LED mapping (P0.0-P0.7, 8 LEDs) ───────────────────────────────────
 *   Left  mode: LEDs sweep right-to-left  (led_left_step)
 *   Right mode: LEDs sweep left-to-right  (led_right_step)
 *   Off   mode: all LEDs off              (led_off)
 */

#ifndef INDICATOR_H
#define INDICATOR_H

#include "types.h"

/* ── ISR prototypes (called by VIC, must carry __irq attribute) ── */
void eint0_isr(void) __irq;     /* EINT0: left  button ISR              */
void eint2_isr(void) __irq;     /* EINT2: right button ISR              */

/* ── Initialisation ──────────────────────────────────────────────── */
void ext_int_init(void);        /* Configure EINT0/EINT2 + VIC channels */

/* ── LED animation steps (called from MAIN_CAN_RX main loop) ────── */
void led_left_step(void);       /* Advance one step of left  swipe      */
void led_right_step(void);      /* Advance one step of right swipe      */
void led_off(void);             /* Turn all 8 indicator LEDs off        */

/* ── Airbag (called from MAIN_CAN_TX and INDICATOR_GEN loops) ───── */
void airbag_trigger(void);      /* Read accelerometer, deploy if impact  */

#endif /* INDICATOR_H */
