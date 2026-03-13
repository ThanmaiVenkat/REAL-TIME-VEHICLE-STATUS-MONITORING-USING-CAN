/**
 * @file    MAIN_CAN_TX.c
 * @brief   Main dashboard node — CAN receive + LCD display + airbag (LPC2129)
 *
 * ── Node role ──────────────────────────────────────────────────────────
 * This is the MAIN NODE (central display board).
 * It receives CAN frames from the FUEL NODE (ID=2) and INDICATOR NODE
 * (ID=1, via blink flags from INDICATOR_GEN ISRs), and presents a
 * clean real-time dashboard on the JHD 204A 20x4 LCD.
 *
 * ── LCD layout (20 columns × 4 lines) ─────────────────────────────────
 *
 *   Pos: 0         1
 *        01234567890123456789
 *       ┌────────────────────┐
 *  L1:  │  VEHICLE DASHBOARD │  ← static title (written once)
 *  L2:  │FUEL [██████████]75%│  ← fuel bar + % (updates on CAN ID=2)
 *  L3:  │IND  <<<   ◆   >>> │  ← indicator arrows (blink flags)
 *  L4:  │AIRBAG  STATUS: OK! │  ← airbag status (updates on impact)
 *       └────────────────────┘
 *
 * ── CGRAM character slots (loaded by WriteToCGRAM in LCD.c) ───────────
 *   Char 0 = left  arrow ◄   Char 1 = right arrow ►
 *   Char 2 = diamond     ◆   Char 6 = full block  █  (fuel bar filled)
 *   Char 7 = border box  □   (fuel bar empty cell)
 *
 * ── CAN messages received ──────────────────────────────────────────────
 *   ID=1: indicator command   (DATA1: 0x01=left, 0x02=right)
 *         → updates blink1/blink2 via ISRs in INDICATOR_GEN.c
 *   ID=2: fuel percentage     (DATA2: 0-100)
 *         → updates fuel bar and percentage on LCD line 2
 *
 * ── Peripherals initialised ────────────────────────────────────────────
 *   CAN1, EINT0/EINT2 (indicator buttons), I2C (MMA7660),
 *   LCD (JHD 204A 20x4), MMA7660FC (airbag accelerometer)
 */

#include <LPC21XX.h>
#include "delay.h"
#include "can.h"
#include "can_defines.h"
#include "types.h"
#include "lcd.h"
#include "lcd_defines.h"
#include "indicator.h"
#include "i2c.h"
#include "mma_7660.h"

/* ── Shared blink flags set by INDICATOR_GEN ISRs ────────── */
extern volatile u32 blink1;     /* 1 = left  indicator active           */
extern volatile u32 blink2;     /* 1 = right indicator active           */

/* ── Global state ─────────────────────────────────────────── */
static CANF rxF;                /* CAN receive buffer                   */
static u8   fuel      = 0;     /* Last received fuel percentage (0-100) */
static u8   ind_left  = 0;     /* Current left  indicator display state */
static u8   ind_right = 0;     /* Current right indicator display state */

/* ═══════════════════════════════════════════════════════════════
 * DISPLAY FUNCTIONS
 * All display functions write ONLY to their own LCD positions.
 * Static labels are written once; value fields update each frame.
 * ═══════════════════════════════════════════════════════════════ */

/* ─────────────────────────────────────────────────────────────
 * display_static_labels — write all fixed text (call once only)
 * ───────────────────────────────────────────────────────────── */
static void display_static_labels(void)
{
    /* Line 1: title */
    lcd_cmd(0x80);
    lcd_str("  VEHICLE DASHBOARD ");   /* 20 chars exactly */

    /* Line 2: fuel label and closing bracket */
    lcd_cmd(0xC0 + 0);
    lcd_str("FUEL [");                 /* pos 0-5                         */
    lcd_cmd(0xC0 + 16);
    lcd_char(']');                     /* pos 16                          */

    /* Line 3: indicator label and centre diamond */
    lcd_cmd(0x94 + 0);
    lcd_str("IND ");                   /* pos 0-3                         */
    lcd_cmd(0x94 + 9);
    lcd_char(2);                       /* pos 9: CGRAM 2 = diamond ◆      */

    /* Line 4: airbag label and static status text */
    lcd_cmd(0xD4 + 0);
    lcd_str("AIRBAG  STATUS:     "); /* pos 0-19 (value filled next)    */
    lcd_cmd(0xD4 + 16);
    lcd_str(" OK!");                   /* default: system is safe         */
}

/* ─────────────────────────────────────────────────────────────
 * display_fuel_bar — update line 2 bar blocks and percentage
 *
 * Bar occupies positions 6-15 (10 cells).
 * Each cell = 10% of tank. Filled = CGRAM 6, empty = CGRAM 7.
 * Percentage is right-aligned in positions 17-19 (always 3 chars).
 * ───────────────────────────────────────────────────────────── */
static void display_fuel_bar(u8 f)
{
    u8 filled, i;

    if (f > 100) f = 100;

    filled = f / 10;            /* 0-10 filled blocks                  */

    /* Write 10 bar cells at pos 6-15 */
    lcd_cmd(0xC0 + 6);
    for (i = 0; i < 10; i++)
        lcd_char(i < filled ? 6 : 7); /* 6=full █, 7=empty □            */

    /* Write percentage at pos 17-19 (fixed width, always 3 chars) */
    lcd_cmd(0xC0 + 17);
    if (f < 100) lcd_char(' ');  /* pad hundreds digit                  */
    if (f < 10)  lcd_char(' ');  /* pad tens    digit                   */
    lcd_int(f);
    lcd_char('%');
}

/* ─────────────────────────────────────────────────────────────
 * display_indicators — update line 3 left/right arrow positions
 *
 * Left  arrows: pos 4-6  (CGRAM 0 = ◄ or space)
 * Right arrows: pos 12-14 (CGRAM 1 = ► or space)
 * ───────────────────────────────────────────────────────────── */
static void display_indicators(u8 left_on, u8 right_on)
{
    u8 i;

    /* Left arrows at pos 4-6 */
    lcd_cmd(0x94 + 4);
    for (i = 0; i < 3; i++)
        lcd_char(left_on ? 0 : ' ');   /* CGRAM 0 = left arrow ◄         */

    /* Right arrows at pos 12-14 */
    lcd_cmd(0x94 + 12);
    for (i = 0; i < 3; i++)
        lcd_char(right_on ? 1 : ' ');  /* CGRAM 1 = right arrow ►        */
}

/* ─────────────────────────────────────────────────────────────
 * display_airbag — update airbag status text on line 4 pos 16-19
 * ───────────────────────────────────────────────────────────── */
static void display_airbag(u8 triggered)
{
    lcd_cmd(0xD4 + 16);
    if (triggered)
        lcd_str("BOOM");   /* 4 chars — impact detected               */
    else
        lcd_str(" OK!");   /* 4 chars — system safe                   */
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN — dashboard node entry point
 * ═══════════════════════════════════════════════════════════════ */
int main(void)
{
    /* ── Initialise all peripherals ── */
    can1_init();            /* CAN1 bus at 125 kbps                    */
    ext_int_init();         /* EINT0/EINT2 for indicator buttons        */
    i2c_init();             /* I2C for MMA7660 accelerometer           */
    lcd_init();             /* LCD power-on reset sequence             */
    mma7660_init();         /* Accelerometer: standby → active         */
    WriteToCGRAM();         /* Load 8 custom chars into LCD CGRAM      */

    /* ── Draw static labels once — never rewritten in the loop ── */
    display_static_labels();

    /* ── Show initial values ── */
    display_fuel_bar(0);            /* 0% until first CAN frame arrives */
    display_indicators(0, 0);       /* both indicators off               */
    display_airbag(0);              /* system OK                         */

    /* ════════════════════════════════════════════════════════
     * Main loop
     * Each iteration:
     *   1. Check accelerometer for airbag condition
     *   2. Poll CAN bus for new fuel data
     *   3. Blink the active indicator arrow
     * ════════════════════════════════════════════════════════ */
    while (1)
    {
        /* ── 1. Airbag: read accelerometer, update line 4 if triggered ── */
        airbag_trigger();

        /* ── 2. CAN receive: update fuel bar and percentage ── */
        if (can1_rx(&rxF))
        {
            if (rxF.ID == 2)
            {
                /* Fuel frame from FUEL NODE */
                fuel = (u8)rxF.DATA2;
                if (fuel > 100) fuel = 100;
                display_fuel_bar(fuel);
            }
            /* ID=1 (indicator) frames are handled by EINT ISRs
             * which set blink1/blink2 — no action needed here */
        }

        /* ── 3. Indicator blink: toggle arrows every 400 ms ── */
        if (blink1)
        {
            /* Left indicator active — show arrows */
            if (!ind_left)
            {
                ind_left = 1;
                display_indicators(1, 0);
            }
            delay_ms(400);

            /* Flash off */
            display_indicators(0, 0);
            delay_ms(400);
        }
        else if (blink2)
        {
            /* Right indicator active — show arrows */
            if (!ind_right)
            {
                ind_right = 1;
                display_indicators(0, 1);
            }
            delay_ms(400);

            /* Flash off */
            display_indicators(0, 0);
            delay_ms(400);
        }
        else
        {
            /* No indicator active — clear arrows and reset state flags */
            if (ind_left || ind_right)
            {
                display_indicators(0, 0);
                ind_left  = 0;
                ind_right = 0;
            }
            delay_ms(200);  /* short delay to keep CAN polling responsive */
        }
    }
}
