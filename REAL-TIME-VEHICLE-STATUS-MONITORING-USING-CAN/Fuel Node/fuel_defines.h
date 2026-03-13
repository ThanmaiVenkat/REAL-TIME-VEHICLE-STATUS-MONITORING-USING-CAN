/**
 * @file    fuel_defnes.h
 * @brief   ADC configuration macros for fuel-level sensing (LPC2129)
 *
 * The fuel sensor output is read on ADC channel 1 (AIN1, pin P0.28).
 * ADC clock must be ≤ 4.5 MHz per LPC21xx datasheet.
 *
 * Clock chain:
 *   FOSC = 12 MHz  →  CCLK = 60 MHz (PLL ×5)  →  PCLK = 15 MHz (÷4)
 *   ADC_CLK = 3 MHz  →  CLK_DIV = (15 / 3) - 1 = 4
 */

#ifndef FUEL_DEFNES_H
#define FUEL_DEFNES_H

/* ── System clock chain ────────────────────────────────────── */
#define FOSC        12000000            /* Crystal oscillator  : 12 MHz */
#define CCLK        (FOSC * 5)         /* CPU clock (PLL ×5)  : 60 MHz */
#define PCLK        (CCLK / 4)         /* Peripheral clock (÷4):15 MHz */

/* ── ADC clock ─────────────────────────────────────────────── */
#define ADC_CLK     3000000            /* Target ADC clock : 3 MHz (≤4.5 MHz) */
#define CLK_DIV     ((PCLK / ADC_CLK) - 1)  /* CLKDIV field value = 4     */

/* ── ADCR register bit positions ──────────────────────────── */
#define CLK_DIV_BITS    8   /* Bits 8-15  : ADC clock divider            */
#define PDN_BIT         21  /* Bit  21    : 1 = ADC powered on           */
#define CONV_START_BITS 24  /* Bits 24-26 : 001 = start conversion now   */

/* ── ADDR register bit positions ──────────────────────────── */
#define RESULT_BITS     6   /* Bits 6-15  : 10-bit conversion result      */
#define DONE_BIT        31  /* Bit  31    : 1 = conversion complete        */

/* ── PINSEL1 masks to enable ADC pins ─────────────────────── */
#define AIN0_0_27   0x00400000  /* P0.27 → AIN0 (reserved / spare)       */
#define AIN1_0_28   0x01000000  /* P0.28 → AIN1 (fuel sensor input)       */

#endif /* FUEL_DEFNES_H */
