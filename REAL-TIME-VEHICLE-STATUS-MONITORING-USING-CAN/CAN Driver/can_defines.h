/**
 * @file    can_defines.h
 * @brief   CAN1 peripheral configuration macros for LPC2129
 *
 * Calculates all CAN bit-timing registers at compile time from
 * three user-facing constants: PCLK, BIT_RATE, and QUANTA.
 * Change only those three values to retarget a different baud rate.
 *
 * Timing formula (CAN spec):
 *   Bit time = 1 / BIT_RATE
 *   Bit time = QUANTA * (1 / PCLK) * BRP
 *   TSEG1 + TSEG2 + 1 (sync) = QUANTA
 *   Sample point = (1 + TSEG1) / QUANTA   (target ~70%)
 *
 * @hardware LPC2129, PCLK = 15 MHz
 */

#include "types.h"

/* ── Pin-select value to route CAN1 RD/TD to P0.0 / P0.1 ── */
#define RD1_PIN_0_25    0x15440000      /* PINSEL1 bits for CAN1 RX/TX */

/* ── Clock & baud-rate inputs (change these to reconfigure) ── */
#define PCLK            15000000        /* Peripheral clock  : 15 MHz  */
#define BIT_RATE        125000          /* Target CAN speed  : 125 kbps */
#define QUANTA          15              /* Time quanta per bit          */

/* ── Derived timing values (do not edit below this line) ── */
#define BRP             (PCLK / (BIT_RATE * QUANTA))    /* Baud-rate prescaler        */
#define SAMPLE_POINT    (0.7 * QUANTA)                  /* 70 % sample point          */
#define TSEG1           ((u32)SAMPLE_POINT - 1)         /* Phase segment 1            */
#define TSEG2           (QUANTA - (1 + TSEG1))          /* Phase segment 2            */
#define SJW             ((TSEG2 >= 5) ? 4 : (TSEG2-1)) /* Sync jump width (max 4)    */
#define SAM_BIT         0                               /* Single sample per bit      */

/* Packed value written directly to C1BTR */
#define BTR_LVAL  ((SAM_BIT      << 23) | \
                   ((TSEG2 - 1)  << 20) | \
                   ((TSEG1 - 1)  << 16) | \
                   ((SJW   - 1)  << 14) | \
                   (BRP    - 1))

/* ── C1MOD — mode register bit positions ── */
#define RM_BIT          0       /* Reset mode: 1=reset, 0=operating    */

/* ── C1CMR — command register bit positions ── */
#define TR_BIT          0       /* Transmission request                 */
#define RRB_BIT         2       /* Release receive buffer               */
#define STB1_BIT        5       /* Select TX buffer 1                   */

/* ── C1GSR — global status register bit positions ── */
#define RBS_BIT         0       /* Receive buffer status (1 = msg ready)*/
#define TBS1_BIT        2       /* TX buffer 1 status   (1 = available) */
#define TCS1_BIT        3       /* TX complete status   (1 = complete)  */

/* ── C1RFS — receive frame status bit positions ── */
#define FF_BIT          31      /* Frame format: 0=standard, 1=extended */
#define RTR_BIT         30      /* Remote transmission request           */
#define DLC_BIT         16      /* Data length code (bits 16-19)         */

/* ── AFMR — acceptance filter mode register ── */
#define ACCOFF_BIT      0       /* 1 = acceptance filter off (block all) */
#define ACCBP_BIT       1       /* 1 = bypass filter (accept all)        */

/* ── External interrupt VIC channel numbers ── */
#define EINT0_0_1       0x0000000c  /* PINSEL0 mask for EINT0 on P0.0/P0.1 */
#define EINT0_VIC_CHNO  14          /* VIC channel for EINT0                */

#define EINT2_0_7       0x0000c000  /* PINSEL0 mask for EINT2 on P0.7       */
#define EINT2_VIC_CHNO  16          /* VIC channel for EINT2                */

/* ── Indicator LED direction placeholders (unused — reserved) ── */
#define L_LED           0
#define R_LED           0
