/**
 * @file    lcd.h
 * @brief   JHD 204A (20x4) LCD driver public interface for LPC2129
 *
 * All functions use 8-bit parallel write mode (RW tied low).
 * Call lcd_init() once at startup, then WriteToCGRAM() to load
 * the 8 custom characters before using lcd_char(0..7).
 *
 * Coordinate system — use lcd_cmd(DDRAM_addr) to position cursor:
 *   Line 1: 0x80 + col    Line 2: 0xC0 + col
 *   Line 3: 0x94 + col    Line 4: 0xD4 + col
 */

#ifndef LCD_H
#define LCD_H

#include "types.h"

void lcd_init(void);            /* Power-on init sequence (call once)         */
void lcd_disp(u8 val);          /* Low-level: write byte with EN pulse        */
void lcd_cmd(u8 cmd);           /* Send instruction byte  (RS=0)              */
void lcd_char(s8 dat);          /* Send one display character (RS=1)          */
void lcd_str(s8 *ptr);          /* Send null-terminated string                */
void lcd_int(s32 num);          /* Display signed integer as ASCII digits     */
void lcd_f32(f32 num, u32 nDP); /* Display float with nDP decimal places      */
void WriteToCGRAM(void);        /* Load 8 custom characters into CGRAM        */
void lcd_hex(u8 data);          /* Display one byte as 2 hex digits (e.g. AF) */
void lcd_hex_data(u32 value);   /* Display 32-bit value as "0xAABBCCDD"       */

#endif /* LCD_H */
