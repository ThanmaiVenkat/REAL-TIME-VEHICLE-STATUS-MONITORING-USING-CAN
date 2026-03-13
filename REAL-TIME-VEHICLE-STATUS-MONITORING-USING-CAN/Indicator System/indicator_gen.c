/**
 * @file    INDICATOR_GEN.c
 * @brief   Indicator TX node — button ISRs + CAN transmit + LED blink (LPC2129)
 *
 * ── Node role ──────────────────────────────────────────────────────────
 * This is the INDICATOR NODE (standalone MCU board with two buttons).
 * Button presses trigger external interrupts (EINT0, EINT2) which:
 *   1. Send a CAN frame (ID=1) to the MAIN NODE and RX NODE.
 *   2. Toggle blink1/blink2 flags used by the main loop for LED blinking.
 *
 * ── Logic: toggle behaviour ────────────────────────────────────────────
 *   Press left  button once  → blink1=1 (left  indicator ON)
 *   Press left  button again → blink1=0 (left  indicator OFF)
 *   Pressing left while right is active → right stops, left starts.
 *   (Only one indicator can be active at a time.)
 *
 * ── CAN frame sent ─────────────────────────────────────────────────────
 *   ID=1, DATA1=0x01 → left  button
 *   ID=1, DATA1=0x02 → right button
 *
 * ── Hardware ───────────────────────────────────────────────────────────
 *   P0.0  → TX LED
 *   EINT0 → left  button (P0.0/P0.1 via PINSEL0)
 *   EINT2 → right button (P0.7     via PINSEL0)
 */

#include <LPC21XX.h>
#include "delay.h"
#include "can.h"
#include "can_defines.h"
#include "types.h"
#include "lcd.h"
#include "lcd_defines.h"
#include "indicator.h"

/* ── Pin definition ────────────────────────────────────────── */
#define TX_LED  0           /* P0.0: toggles on each CAN transmission  */

/* ── Shared blink flags (read by main loop for LED animation) ─ */
volatile u32 blink1 = 0;   /* 1 = left  indicator active               */
volatile u32 blink2 = 0;   /* 1 = right indicator active               */
volatile u32 flag1  = 0;   /* reserved for future use                  */
volatile u32 flag2  = 0;   /* reserved for future use                  */

/* ─────────────────────────────────────────────────────────────
 * eint0_isr — LEFT button interrupt service routine
 * Triggered on falling edge of EINT0 (P0.0/P0.1).
 * ───────────────────────────────────────────────────────────── */
void eint0_isr(void) __irq
{
    CANF txF = {0};

    IODIR0 |= (1 << TX_LED);       /* ensure TX LED pin is output          */

    /* Build and send CAN frame: ID=1, DATA1=0x01 (left button) */
    txF.ID       = 1;
    txF.bfv.RTR  = 0;
    txF.bfv.DLC  = 2;
    txF.DATA1    = 0x01;
    txF.DATA2    = 0;
    can1_tx(txF);

    IOPIN0 ^= (1 << TX_LED);       /* toggle TX LED to show activity       */

    /* Toggle left indicator; cancel right if it was active */
    blink1 = !blink1;
    blink2 = 0;                    /* mutually exclusive: stop right       */

    /* Show direction arrow on LCD line 2 */
    lcd_cmd(GOTO_LINE_2_POS_0);
    lcd_str(">");

    /* Clear external interrupt flag and acknowledge VIC */
    EXTINT  = (1 << 0);
    VICVectAddr = 0;
}

/* ─────────────────────────────────────────────────────────────
 * eint2_isr — RIGHT button interrupt service routine
 * Triggered on falling edge of EINT2 (P0.7).
 * ───────────────────────────────────────────────────────────── */
void eint2_isr(void) __irq
{
    CANF txF = {0};

    IODIR0 |= (1 << TX_LED);

    /* Build and send CAN frame: ID=1, DATA1=0x02 (right button) */
    txF.ID       = 1;
    txF.bfv.RTR  = 0;
    txF.bfv.DLC  = 2;
    txF.DATA1    = 0x02;
    txF.DATA2    = 0;
    can1_tx(txF);

    IOPIN0 ^= (1 << TX_LED);

    /* Toggle right indicator; cancel left if it was active */
    blink2 = !blink2;
    blink1 = 0;                    /* mutually exclusive: stop left        */

    /* Show direction arrow on LCD line 2 */
    lcd_cmd(GOTO_LINE_2_POS_0);
    lcd_str("<");

    /* Clear external interrupt flag and acknowledge VIC */
    EXTINT  = (1 << 2);
    VICVectAddr = 0;
}

/* ─────────────────────────────────────────────────────────────
 * ext_int_init — configure EINT0 and EINT2 with VIC
 * ───────────────────────────────────────────────────────────── */
void ext_int_init(void)
{
    /* Route EINT0 (P0.0/P0.1) and EINT2 (P0.7) via PINSEL0 */
    PINSEL0 |= EINT0_0_1 | EINT2_0_7;

    /* Enable EINT0 and EINT2 in VIC interrupt enable register */
    VICIntEnable = (1 << EINT0_VIC_CHNO) | (1 << EINT2_VIC_CHNO);

    /* Assign EINT0 to VIC slot 0 with enable bit (bit 5) */
    VICVectCntl0 = (1 << 5) | EINT0_VIC_CHNO;
    VICVectAddr0 = (s32)eint0_isr;

    /* Assign EINT2 to VIC slot 1 with enable bit (bit 5) */
    VICVectCntl1 = (1 << 5) | EINT2_VIC_CHNO;
    VICVectAddr1 = (s32)eint2_isr;

    /* Set both interrupts to edge-triggered mode */
    EXTMODE = (1 << 0) | (1 << 2);

    /* Default polarity is falling-edge (EXTPOLAR = 0, no change needed) */
}
