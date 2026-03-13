/**
 * @file    FUEL.c
 * @brief   ADC driver for fuel-level sensing on LPC2129
 *
 * Reads a resistive fuel-sensor voltage on AIN1 (P0.28).
 * Conversion result is 10-bit (0-1023) mapped to 0-3.3 V.
 *
 * Typical use (in FUEL_MAIN.c):
 *   adc_init();
 *   read_adc_value(1, &dVal, &eAr);
 *   percentage = (dVal * 100) / 1023;
 */

#include <LPC21xx.h>
#include "types.h"
#include "delay.h"
#include "fuel_defnes.h"

/* ─────────────────────────────────────────────────────────────
 * adc_init — configure ADC peripheral
 * ───────────────────────────────────────────────────────────── */
void adc_init(void)
{
    /* Clear any previous pin-function bits for pins 27-30 in PINSEL1
     * (PINSEL1 controls P0.16-P0.31; pin 27 = bit 22-23, pin 28 = bit 24-25) */
    PINSEL1 &= ~(0xFF << 22);

    /* Enable AIN0 (P0.27) and AIN1 (P0.28) as analogue inputs */
    PINSEL1 |= (AIN0_0_27) | (AIN1_0_28);

    /* Set ADC clock divider and power on the ADC module */
    ADCR |= (CLK_DIV << CLK_DIV_BITS)  /* ADC clock = PCLK / (CLK_DIV+1) */
          | (1        << PDN_BIT);      /* PDN=1: ADC powered and active   */
}

/* ─────────────────────────────────────────────────────────────
 * read_adc_value — single-shot blocking conversion
 * ───────────────────────────────────────────────────────────── */
void read_adc_value(u32 ch_no, u32 *dVal, f32 *eAr)
{
    /* Select channel: clear bits[7:0] then set the desired channel bit */
    ADCR &= 0xFFFFFF00;             /* clear channel select field        */
    ADCR |= (1 << ch_no)            /* enable selected channel           */
          | (1 << CONV_START_BITS); /* start = 001: convert now          */

    delay_us(3);                    /* wait ≥ 2.44 µs (10-bit conv time) */

    /* Poll DONE bit — wait for conversion to complete */
    while (((ADDR >> DONE_BIT) & 1) == 0);

    /* Stop conversion (clear START bits to avoid repeated conversions) */
    ADCR &= ~(1 << CONV_START_BITS);

    /* Extract 10-bit result from ADDR bits[15:6] */
    *dVal = (ADDR >> RESULT_BITS) & 0x3FF;   /* mask to 10 bits (0-1023) */

    /* Convert to analogue voltage equivalent (0.0 - 3.3 V) */
    *eAr = (*dVal * (3.3f / 1023.0f));
}
