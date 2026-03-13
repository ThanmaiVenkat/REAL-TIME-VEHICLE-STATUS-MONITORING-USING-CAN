/**
 * @file    LCD.c
 * @brief   JHD 204A (20x4) LCD driver for LPC2129 — 8-bit parallel mode
 *
 * ── Interface wiring ───────────────────────────────────────────────────
 *   P0.10-P0.17 → DB0-DB7   (8-bit data bus, WRITEBYTE macro)
 *   P0.18       → RS         (0=command, 1=data)
 *   P0.19       → EN         (falling edge latches data)
 *   P0.20       → RW         (always 0 = write)
 *
 * ── CGRAM custom characters (loaded by WriteToCGRAM) ──────────────────
 *   Slot 0 (a): left  arrow ◄     Slot 4 (e): 3-row fill
 *   Slot 1 (b): right arrow ►     Slot 5 (f): 5-row fill
 *   Slot 2 (c): diamond     ◆     Slot 6 (g): full block █  (fuel bar filled)
 *   Slot 3 (d): 1-row fill        Slot 7 (h): border box □  (fuel bar empty)
 */

#include <LPC21xx.h>
#include "delay.h"
#include "types.h"
#include "defines.h"
#include "lcd.h"
#include "lcd_defines.h"

/* ─────────────────────────────────────────────────────────────
 * lcd_init — HD44780 power-on initialisation sequence
 * Follows the datasheet 3-step software reset, then 8-bit/2-line config.
 * ───────────────────────────────────────────────────────────── */
void lcd_init(void)
{
    /* Configure all LCD pins as outputs */
    IODIR0 |= (0xFF << LCD_DATA) | (1 << RS) | (1 << RW) | (1 << EN);

    /* HD44780 power-on reset sequence (datasheet Fig.23) */
    delay_ms(20);           /* wait >15 ms after Vcc rises to 4.5V   */
    lcd_cmd(0x30);
    delay_ms(8);            /* wait >4.1 ms                           */
    lcd_cmd(0x30);
    delay_ms(1);            /* wait >100 µs                           */
    lcd_cmd(0x30);
    delay_ms(1);

    /* Function set: 8-bit bus, 2-line display, 5x8 dot font */
    lcd_cmd(0x38);

    /* Display off */
    lcd_cmd(0x10);

    /* Clear display */
    lcd_cmd(0x01);

    /* Entry mode: increment cursor, no display shift */
    lcd_cmd(0x06);

    /* Display on, cursor off, blink off */
    lcd_cmd(0x0C);

    /* Move cursor to line 1, position 0 */
    lcd_cmd(GOTO_LINE_1_POS_0);
}

/* ─────────────────────────────────────────────────────────────
 * lcd_disp — low-level byte write with EN pulse
 * Called by lcd_cmd and lcd_char after they set RS.
 * ───────────────────────────────────────────────────────────── */
void lcd_disp(u8 val)
{
    IOCLR0 = (1 << RW);                 /* RW = 0: write mode              */
    WRITEBYTE(IOPIN0, LCD_DATA, val);   /* put data on DB0-DB7             */
    IOSET0 = (1 << EN);                 /* EN high                         */
    delay_us(2);                        /* data setup time (min 450 ns)    */
    IOCLR0 = (1 << EN);                 /* EN low → LCD latches on falling */
    delay_ms(2);                        /* command execution time          */
}

/* ─────────────────────────────────────────────────────────────
 * lcd_cmd — send a command byte (RS = 0 = instruction register)
 * ───────────────────────────────────────────────────────────── */
void lcd_cmd(u8 cmd)
{
    IOCLR0 = (1 << RS);     /* RS = 0: instruction register            */
    lcd_disp(cmd);
}

/* ─────────────────────────────────────────────────────────────
 * lcd_char — send one character byte (RS = 1 = data register)
 * ───────────────────────────────────────────────────────────── */
void lcd_char(s8 dat)
{
    IOSET0 = (1 << RS);     /* RS = 1: data register (DDRAM or CGRAM)  */
    lcd_disp(dat);
}

/* ─────────────────────────────────────────────────────────────
 * lcd_str — display a null-terminated string
 * ───────────────────────────────────────────────────────────── */
void lcd_str(s8 *ptr)
{
    while (*ptr)
        lcd_char(*ptr++);
}

/* ─────────────────────────────────────────────────────────────
 * lcd_int — display a signed 32-bit integer as ASCII digits
 * Handles negative numbers; prints "0" for zero.
 * ───────────────────────────────────────────────────────────── */
void lcd_int(s32 num)
{
    s8 a[10];   /* digit buffer (max 10 decimal digits in s32) */
    s8 i = 0;

    if (num == 0)
    {
        lcd_char('0');
        return;
    }

    if (num < 0)
    {
        lcd_char('-');
        num = -num;
    }

    /* Extract digits in reverse order */
    while (num)
    {
        a[i++] = (num % 10) + '0';  /* convert digit to ASCII */
        num /= 10;
    }

    /* Print digits in correct order (most significant first) */
    for (--i; i >= 0; i--)
        lcd_char(a[i]);
}

/* ─────────────────────────────────────────────────────────────
 * lcd_f32 — display a float with nDP decimal places
 * Example: lcd_f32(3.14, 2) → "3.14"
 * ───────────────────────────────────────────────────────────── */
void lcd_f32(f32 num, u32 nDP)
{
    s32 n;
    u32 i;

    if (num < 0)
    {
        lcd_char('-');
        num = -num;
    }

    n = (s32)num;       /* integer part */
    lcd_int(n);
    lcd_char('.');

    /* Extract and display each decimal digit */
    for (i = 0; i <= nDP; i++)
    {
        num = (num - n) * 10.0f;
        n = (s32)num;
        lcd_char((s8)(n + '0'));
    }
}

/* ─────────────────────────────────────────────────────────────
 * WriteToCGRAM — load 8 custom 5x8 characters into CGRAM
 *
 * Each character is 8 bytes (rows), only bits[4:0] used per row.
 * LCD command 0x40 points the address counter to CGRAM slot 0.
 * Slots 0-7 → accessed with lcd_char(0) through lcd_char(7).
 * ───────────────────────────────────────────────────────────── */
void WriteToCGRAM(void)
{
    s8 i;

    /* Slot 0: left arrow ◄ */
    s8 a[] = {0x10, 0x1C, 0x1E, 0x1F, 0x1E, 0x1C, 0x10, 0x00};

    /* Slot 1: right arrow ► */
    s8 b[] = {0x01, 0x07, 0x0F, 0x1F, 0x0F, 0x07, 0x01, 0x00};

    /* Slot 2: diamond ◆ (center arrow) */
    s8 c[] = {0x00, 0x04, 0x0E, 0x1F, 0x0E, 0x04, 0x00, 0x00};

    /* Slot 3: 1-row bottom fill (thin bar level) */
    s8 d[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00};

    /* Slot 4: 3-row fill */
    s8 e[] = {0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x00};

    /* Slot 5: 5-row fill */
    s8 f[] = {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};

    /* Slot 6: full block █ (fuel bar: filled cell) */
    s8 g[] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};

    /* Slot 7: border box □ (fuel bar: empty cell) */
    s8 h[] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00};

    /* Point address counter to CGRAM start (slot 0, row 0) */
    lcd_cmd(0x40);

    /* Write all 8 slots × 8 rows = 64 bytes */
    for (i = 0; i < 8; i++) lcd_char(a[i]);
    for (i = 0; i < 8; i++) lcd_char(b[i]);
    for (i = 0; i < 8; i++) lcd_char(c[i]);
    for (i = 0; i < 8; i++) lcd_char(d[i]);
    for (i = 0; i < 8; i++) lcd_char(e[i]);
    for (i = 0; i < 8; i++) lcd_char(f[i]);
    for (i = 0; i < 8; i++) lcd_char(g[i]);
    for (i = 0; i < 8; i++) lcd_char(h[i]);
}

/* ─────────────────────────────────────────────────────────────
 * lcd_hex — display one byte as two uppercase hex digits
 * Example: lcd_hex(0xAB) → "AB"
 * ───────────────────────────────────────────────────────────── */
void lcd_hex(u8 data)
{
    u8 up  = (data >> 4) & 0x0F;   /* upper nibble */
    u8 low =  data       & 0x0F;   /* lower nibble */

    lcd_disp(up  < 10 ? up  + '0' : up  - 10 + 'A');
    lcd_disp(low < 10 ? low + '0' : low - 10 + 'A');
}

/* ─────────────────────────────────────────────────────────────
 * lcd_hex_data — display 32-bit value as "0xAABBCCDD"
 * ───────────────────────────────────────────────────────────── */
void lcd_hex_data(u32 value)
{
    lcd_str("0x");
    lcd_hex((value >> 24) & 0xFF);
    lcd_hex((value >> 16) & 0xFF);
    lcd_hex((value >>  8) & 0xFF);
    lcd_hex( value        & 0xFF);
}
