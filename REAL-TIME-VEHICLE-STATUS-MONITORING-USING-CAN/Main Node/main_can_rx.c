/**
 * @file    MAIN_CAN_RX.c
 * @brief   Indicator RX node — receives CAN frames and drives LED sweep (LPC2129)
 *
 * ── Node role ──────────────────────────────────────────────────────────
 * This is the INDICATOR RX NODE (standalone MCU board with 8 LEDs).
 * It listens on the CAN bus for frames with ID=1 and drives a sweep
 * animation on P0.0-P0.7 (8 LEDs) accordingly.
 *
 * ── State machine ──────────────────────────────────────────────────────
 *   Receive DATA1=0x01 → if already LEFT  : switch to OFF
 *                        else             : switch to LEFT
 *   Receive DATA1=0x02 → if already RIGHT : switch to OFF
 *                        else             : switch to RIGHT
 *
 *   MODE_LEFT  → call led_left_step()  each loop iteration
 *   MODE_RIGHT → call led_right_step() each loop iteration
 *   MODE_OFF   → call led_off()        each loop iteration
 *
 * ── Hardware ───────────────────────────────────────────────────────────
 *   P0.0-P0.7 → 8 indicator LEDs (driven by INDICATOR.c)
 *   P0.18     → activity LED (toggles on each received CAN frame)
 */

#include <LPC21XX.h>
#include "delay.h"
#include "can.h"
#include "types.h"
#include "indicator.h"

/* ── Activity LED ─────────────────────────────────────────── */
#define LED     18      /* P0.18: toggles when a CAN frame arrives */

/* ── Indicator state machine ──────────────────────────────── */
typedef enum
{
    MODE_OFF   = 0,     /* no indicator active — LEDs all off       */
    MODE_LEFT,          /* left  indicator — LED sweep left         */
    MODE_RIGHT          /* right indicator — LED sweep right        */
} LED_MODE;

static LED_MODE current_mode = MODE_OFF;

/* ── CAN receive buffer ───────────────────────────────────── */
static CANF rxF;

/* ─────────────────────────────────────────────────────────────
 * main — indicator RX node entry point
 * ───────────────────────────────────────────────────────────── */
int main(void)
{
    /* Initialise CAN1 peripheral */
    can1_init();

    /* Configure activity LED as output */
    IODIR0 |= (1 << LED);

    /* ── Main loop: poll CAN, update state, drive LEDs ──── */
    while (1)
    {
        /* ── Step 1: check for new CAN frame ── */
        if (can1_rx(&rxF))
        {
            /* Toggle activity LED to show frame received */
            IOPIN0 ^= (1 << LED);

            /* Only process frames with indicator ID (ID = 1) */
            if (rxF.ID == 1)
            {
                if (rxF.DATA1 == 0x01)
                {
                    /* Left button pressed:
                     * If already in LEFT mode → toggle off
                     * Otherwise              → switch to LEFT */
                    current_mode = (current_mode == MODE_LEFT)
                                   ? MODE_OFF
                                   : MODE_LEFT;
                }
                else if (rxF.DATA1 == 0x02)
                {
                    /* Right button pressed:
                     * If already in RIGHT mode → toggle off
                     * Otherwise               → switch to RIGHT */
                    current_mode = (current_mode == MODE_RIGHT)
                                   ? MODE_OFF
                                   : MODE_RIGHT;
                }
            }
        }

        /* ── Step 2: advance LED animation based on current mode ── */
        switch (current_mode)
        {
            case MODE_LEFT:
                led_left_step();    /* advance one step left  (100 ms) */
                break;

            case MODE_RIGHT:
                led_right_step();   /* advance one step right (100 ms) */
                break;

            case MODE_OFF:
            default:
                led_off();          /* all LEDs off, pos reset to 0    */
                break;
        }
    }
}
