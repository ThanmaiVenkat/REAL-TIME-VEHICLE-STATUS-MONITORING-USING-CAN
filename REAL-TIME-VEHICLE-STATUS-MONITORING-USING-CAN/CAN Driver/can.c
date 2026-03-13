/**
 * @file    CAN.c
 * @brief   CAN1 driver implementation for LPC2129
 *
 * Three functions: init, blocking-TX, non-blocking-RX.
 * All register names (C1MOD, C1BTR, …) come from <LPC21XX.h>.
 * Bit positions are defined in can_defines.h.
 *
 * @note    Acceptance filter is set to BYPASS mode — every frame
 *          on the bus is accepted regardless of ID.
 */

#include <LPC21XX.h>
#include "types.h"
#include "delay.h"
#include "can_defines.h"
#include "can.h"

/* ─────────────────────────────────────────────────────────────
 * can1_init
 * ───────────────────────────────────────────────────────────── */
void can1_init(void)
{
    /* 1. Route P0.0/P0.1 to CAN1 RD/TD via PINSEL1 */
    PINSEL1 |= RD1_PIN_0_25;

    /* 2. Enter reset mode so BTR can be written safely */
    C1MOD |= (1 << RM_BIT);

    /* 3. Configure acceptance filter: bypass (accept all IDs) */
    AFMR &= ~(1 << ACCOFF_BIT);    /* clear ACCOFF — filter enabled    */
    AFMR |=  (1 << ACCBP_BIT);     /* set   ACCBP  — bypass the filter */

    /* 4. Load pre-calculated bit-timing register */
    C1BTR = BTR_LVAL;               /* sets BRP, TSEG1, TSEG2, SJW      */

    /* 5. Leave reset mode — CAN1 is now active */
    C1MOD &= ~(1 << RM_BIT);
}

/* ─────────────────────────────────────────────────────────────
 * can1_tx  —  blocking transmit (with safety timeout)
 * ───────────────────────────────────────────────────────────── */
void can1_tx(CANF txF)
{
    u32 timeout = 50000;

    /* Wait until TX buffer 1 is free (TBS1 = 1) */
    while (((C1GSR >> TBS1_BIT) & 1) == 0)
    {
        if (timeout-- == 0) break;  /* abort if bus is stuck */
    }

    /* Load frame ID into TX identifier register */
    C1TID1 = txF.ID;

    /* Load frame info: RTR flag and data length code */
    C1TFI1 = ((txF.bfv.RTR << RTR_BIT) | (txF.bfv.DLC << DLC_BIT));

    /* Load payload only for data frames (not remote frames) */
    if (txF.bfv.RTR != 1)
    {
        C1TDA1 = txF.DATA1;    /* bytes 0-3 */
        C1TDB1 = txF.DATA2;    /* bytes 4-7 */
    }

    /* Issue transmit request on buffer 1 */
    C1CMR |= ((1 << STB1_BIT) | (1 << TR_BIT));

    /* Wait for transmission complete (TCS1 = 1) */
    timeout = 50000;
    while (((C1GSR >> TCS1_BIT) & 1) == 0)
    {
        if (timeout-- == 0) break;  /* abort if arbitration lost */
    }
}

/* ─────────────────────────────────────────────────────────────
 * can1_rx  —  non-blocking receive
 * Returns 1 and fills *rxF if a frame is waiting, else 0.
 * ───────────────────────────────────────────────────────────── */
u8 can1_rx(CANF *rxF)
{
    /* Check Receive Buffer Status — return immediately if empty */
    if (((C1GSR >> RBS_BIT) & 1) == 0)
        return 0;

    /* Read identifier */
    rxF->ID = C1RID;

    /* Read frame-info flags from receive frame status register */
    rxF->bfv.RTR = ((C1RFS >> RTR_BIT) & 1);
    rxF->bfv.DLC = ((C1RFS >> DLC_BIT) & 0x0F);  /* DLC is 4 bits wide */

    /* Read payload only for data frames */
    if (rxF->bfv.RTR == 0)
    {
        rxF->DATA1 = C1RDA;    /* bytes 0-3 */
        rxF->DATA2 = C1RDB;    /* bytes 4-7 */
    }

    /* Release the receive buffer so the next frame can be stored */
    C1CMR |= (1 << RRB_BIT);

    return 1;
}
