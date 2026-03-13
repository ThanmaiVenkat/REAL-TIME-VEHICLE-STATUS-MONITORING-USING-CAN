/**
 * @file    fuel.h
 * @brief   ADC driver public interface for fuel-level reading (LPC2129)
 *
 * Provides two functions:
 *   adc_init        — configure ADC peripheral and pin-select
 *   read_adc_value  — single-shot blocking conversion on a given channel
 *
 * Returned digital value range: 0 – 1023 (10-bit ADC).
 * Voltage range at AIN pin:     0 – 3.3 V.
 */

#ifndef FUEL_H
#define FUEL_H

#include "types.h"

/**
 * @brief  Initialise the ADC peripheral.
 *         Enables AIN0 (P0.27) and AIN1 (P0.28) via PINSEL1,
 *         sets the ADC clock divider, and powers the ADC on.
 */
void adc_init(void);

/**
 * @brief  Perform one blocking ADC conversion.
 * @param  ch_no  ADC channel number (0 or 1 for AIN0/AIN1).
 * @param  dVal   Output: raw 10-bit digital result (0 – 1023).
 * @param  eAr    Output: equivalent analogue voltage  (0.0 – 3.3 V).
 *
 * Starts the conversion, busy-waits ~3 µs for completion,
 * then extracts the result from ADDR.
 */
void read_adc_value(u32 ch_no, u32 *dVal, f32 *eAr);

#endif /* FUEL_H */
