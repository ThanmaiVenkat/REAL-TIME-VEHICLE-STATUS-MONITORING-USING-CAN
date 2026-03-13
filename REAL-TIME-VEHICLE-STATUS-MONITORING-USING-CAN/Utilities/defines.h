/**
 * @file    defines.h
 * @brief   General-purpose bit/byte manipulation macros
 *
 * All macros operate on any integer WORD at bit position SBP.
 * Safe to use on any LPC21xx SFR or ordinary variable.
 *
 * Usage examples:
 *   SETBIT(IODIR0, 5);              // set   bit 5
 *   CLRBIT(IOPIN0, 5);              // clear bit 5
 *   CPLBIT(IOPIN0, 5);              // toggle bit 5
 *   if (READBIT(C1GSR, RBS_BIT))    // test  bit
 *   WRITEBIT(IOPIN0, 5, 1);         // write single bit
 *   WRITEBYTE(IOPIN0, 10, 0xAB);    // write 8-bit field at pos 10
 *   WRITENIBBLE(IOPIN0, 4, 0xF);    // write 4-bit field at pos 4
 *   WRITEHWORD(IOPIN0, 0, 0x1234);  // write 16-bit field at pos 0
 */

#ifndef DEFINES_H
#define DEFINES_H

/* ── Single-bit operations ─────────────────────────────────── */
#define SETBIT(WORD, SBP)       (WORD |=  (1 << (SBP)))        /* Set bit SBP to 1   */
#define CLRBIT(WORD, SBP)       (WORD &= ~(1 << (SBP)))        /* Clear bit SBP to 0 */
#define CPLBIT(WORD, SBP)       (WORD ^=  (1 << (SBP)))        /* Toggle bit SBP     */
#define READBIT(WORD, SBP)      (((WORD) >> (SBP)) & 1)        /* Read bit SBP (0/1) */

/* ── Multi-bit write operations ────────────────────────────── */
/* Each macro: clear the target field, then OR in the new value */
#define WRITEBIT(WORD, SBP, BIT) \
        WORD = ((WORD & ~(1       << (SBP))) | ((BIT)    << (SBP)))

#define WRITEBYTE(WORD, SBP, BYTE_D) \
        WORD = ((WORD & ~(0xFF    << (SBP))) | ((BYTE_D) << (SBP)))

#define WRITENIBBLE(WORD, SBP, NIBBLE) \
        WORD = ((WORD & ~(0xF     << (SBP))) | ((NIBBLE) << (SBP)))

#define WRITEHWORD(WORD, SBP, HWORD) \
        WORD = ((WORD & ~(0xFFFF  << (SBP))) | ((HWORD)  << (SBP)))

#endif /* DEFINES_H */
