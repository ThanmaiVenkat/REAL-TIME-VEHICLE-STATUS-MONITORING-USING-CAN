/**
 * @file    can.h
 * @brief   CAN1 driver public interface for LPC2129
 *
 * Exposes a single frame type (CANF) and three functions:
 *   - can1_init : configure baud rate and acceptance filter
 *   - can1_tx   : blocking transmit with timeout
 *   - can1_rx   : non-blocking receive (returns 0 if no frame)
 *
 * CAN frame ID usage in this project:
 *   ID = 1  →  indicator command  (DATA1: 0x01=left, 0x02=right)
 *   ID = 2  →  fuel percentage    (DATA2: 0-100)
 */

#include "types.h"

/**
 * @brief  CAN frame container
 *
 * Maps directly onto the LPC2129 TX/RX buffer registers.
 * DATA1 holds bytes 0-3, DATA2 holds bytes 4-7 (little-endian).
 */
typedef struct can_frame
{
    u32 ID;                 /* 11-bit standard frame identifier        */

    struct bitfield
    {
        u32 RTR : 1;        /* 1 = remote frame, 0 = data frame        */
        u32 DLC : 4;        /* Data length code: number of data bytes  */
        u32 FF  : 1;        /* Frame format: 0 = standard (11-bit ID)  */
    } bfv;

    u32 DATA1;              /* Bytes 0-3 of payload                    */
    u32 DATA2;              /* Bytes 4-7 of payload                    */

} CANF;

/* ── Function prototypes ─────────────────────────────────── */

/**
 * @brief  Initialise CAN1 peripheral.
 *         Configures pin-select, acceptance filter (bypass = accept all),
 *         and loads the pre-calculated bit-timing value BTR_LVAL.
 */
void can1_init(void);

/**
 * @brief  Transmit one CAN frame (blocking with timeout).
 * @param  txF  Frame to send (passed by value).
 *
 * Waits up to 50 000 iterations for the TX buffer to become free,
 * then loads the frame and issues a transmission request.
 * Waits again for TCS (transmission complete) before returning.
 */
void can1_tx(CANF txF);

/**
 * @brief  Receive one CAN frame (non-blocking).
 * @param  rxF  Pointer to frame struct to be filled.
 * @return 1 if a frame was read, 0 if receive buffer was empty.
 *
 * Caller must poll this function; it does NOT block.
 * After reading, releases the hardware receive buffer (RRB).
 */
u8 can1_rx(CANF *rxF);
