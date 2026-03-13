/**
 * @file    lcd_defines.h
 * @brief   JHD 204A (20×4) LCD pin and command definitions for LPC2129
 *
 * ── Wiring (8-bit parallel interface) ─────────────────────────────────
 *   P0.10-P0.17  →  DB0-DB7  (data bus, 8 bits)
 *   P0.18        →  RS       (0=instruction register, 1=data register)
 *   P0.19        →  EN       (enable pulse, active high)
 *   P0.20        →  RW       (0=write, 1=read — always 0 in this driver)
 *
 * ── DDRAM address map (JHD 204A) ──────────────────────────────────────
 *   Line 1: 0x80 – 0x93  (positions 0-19)
 *   Line 2: 0xC0 – 0xD3  (positions 0-19)
 *   Line 3: 0x94 – 0xA7  (positions 0-19)
 *   Line 4: 0xD4 – 0xE7  (positions 0-19)
 *
 * ── CGRAM ─────────────────────────────────────────────────────────────
 *   8 custom characters stored in WriteToCGRAM() (LCD.c):
 *   Char 0 (a): left  arrow  ◄
 *   Char 1 (b): right arrow  ►
 *   Char 2 (c): diamond      ◆
 *   Char 3 (d): 1-row fill   (thin)
 *   Char 4 (e): 3-row fill
 *   Char 5 (f): 5-row fill
 *   Char 6 (g): full  block  █  ← used for fuel bar filled cell
 *   Char 7 (h): border box   □  ← used for fuel bar empty cell
 */

#ifndef LCD_DEFINES_H
#define LCD_DEFINES_H

/* ── Data bus start bit in IOPIN0 ──────────────────────────── */
#define LCD_DATA    10      /* DB0 = P0.10, DB7 = P0.17           */

/* ── Control pin bit positions in IOPIN0 ───────────────────── */
#define RS          18      /* Register select (P0.18)             */
#define EN          19      /* Enable          (P0.19)             */
#define RW          20      /* Read/Write      (P0.20) — keep low  */

/* ── DDRAM position commands ────────────────────────────────── */
#define GOTO_LINE_1_POS_0   0x80    /* Move cursor to line 1, column 0 */
#define GOTO_LINE_2_POS_0   0xC0    /* Move cursor to line 2, column 0 */
#define GOTO_LINE_3_POS_0   0x94    /* Move cursor to line 3, column 0 */
#define GOTO_LINE_4_POS_0   0xD4    /* Move cursor to line 4, column 0 */

/* ── Common LCD commands ────────────────────────────────────── */
#define LCD_CLR     0x01    /* Clear display and return cursor home */

#endif /* LCD_DEFINES_H */
