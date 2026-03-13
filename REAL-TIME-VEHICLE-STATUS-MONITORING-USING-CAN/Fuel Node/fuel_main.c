/**
 * @file    FUEL_MAIN.c
 * @brief   Fuel sensor node — reads ADC and transmits via CAN (LPC2129)
 *
 * ── Node role ──────────────────────────────────────────────────────────
 * This is the FUEL NODE (standalone MCU board).
 * It samples AIN1 (fuel sensor), converts to a 0-100 % value,
 * and transmits a CAN frame every 300 ms to the MAIN NODE.
 *
 * ── CAN frame sent ─────────────────────────────────────────────────────
 *   ID    = 2
 *   DLC   = 8
 *   DATA2 = fuel percentage (0-100)
 *
 * ── Hardware ───────────────────────────────────────────────────────────
 *   P0.0  → TX LED (toggles each CAN transmission)
 *   P0.28 → AIN1  (fuel sensor analogue input, 0-3.3 V)
 */

#include <LPC21XX.h>
#include "delay.h"
#include "can.h"
#include "can_defines.h"
#include "types.h"
#include "fuel.h"

/* ── Pin definitions ──────────────────────────────────────── */
#define TX_LED      0       /* P0.0: blinks on every CAN transmission */

/* ── Globals ───────────────────────────────────────────────── */
static u32 dVal       = 0;  /* Raw ADC result  (0-1023)              */
static f32 eAr        = 0;  /* Analogue equivalent voltage (0-3.3 V) */
static u8  percentage = 0;  /* Fuel level percent (0-100)            */

/* ─────────────────────────────────────────────────────────────
 * main — fuel node entry point
 * ───────────────────────────────────────────────────────────── */
int main(void)
{
    CANF txF = {0};

    /* Configure TX LED pin as output */
    IODIR0 |= (1 << TX_LED);

    /* Initialise peripherals */
    can1_init();
    adc_init();

    /* Pre-fill constant frame fields (never change in this node) */
    txF.ID       = 2;       /* Fuel node always uses CAN ID 2 */
    txF.bfv.RTR  = 0;       /* Data frame (not remote)        */
    txF.bfv.DLC  = 8;       /* 8 data bytes                   */

    /* ── Main loop: sample → calculate → transmit ─────────── */
    while (1)
    {
        /* 1. Read ADC channel 1 (fuel sensor on AIN1 / P0.28) */
        read_adc_value(1, &dVal, &eAr);

        /* 2. Convert raw 10-bit ADC value to percentage (0-100) */
        percentage = (u8)((dVal * 100UL) / 1023UL);

        /* 3. Clamp to valid range (defensive — formula never exceeds 100) */
        if (percentage > 100) percentage = 100;

        /* 4. Pack percentage into DATA2 of the CAN frame
         *    DATA1 is unused by this node (left as 0) */
        txF.DATA1 = 0;
        txF.DATA2 = percentage;

        /* 5. Transmit the frame */
        can1_tx(txF);

        /* 6. Toggle LED to show activity */
        IOPIN0 ^= (1 << TX_LED);

        /* 7. Wait 300 ms before next sample (≈3 Hz update rate) */
        delay_ms(300);
    }
}
