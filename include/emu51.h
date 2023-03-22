/*******************************************************************************
 * Emu51
 * code_51.h:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#ifndef EMU51_EMU51_H
#define EMU51_EMU51_H

#include <stdint.h>
#include <allegro.h>

#define bit0 0x01
#define bit1 0x02
#define bit2 0x04
#define bit3 0x08
#define bit4 0x10
#define bit5 0x20
#define bit6 0x40
#define bit7 0x80

extern uint8_t *ram;
extern uint8_t *prog;

extern FONT *mainf;

extern uint8_t SFR[0x100];                   // internal memory and sfr area 256 bytes
extern uint8_t EXT_RAM[0x10000];             // external memory 64kB
extern uint8_t EXT_PMEM[0x10000];            // external memory 64kB for program
extern uint8_t *Acc;            // 8-bit accumlator
extern uint16_t *DPTR;          // 16-bit register
extern uint8_t *DPH;            // high byte of DPTR
extern uint8_t *DPL;            // low byte of DPTR
extern uint8_t *B;              // B register
extern uint8_t *SP;             // Stack Pointer
extern uint8_t *PSW;            // Program Status Word
extern uint8_t *P0;             // 1st Port
extern uint8_t *P1;             // 2nd Port
extern uint8_t *P2;             // 3rd Port
extern uint8_t *P3;             // 4th port
extern uint8_t *SBUF;           // Serial transmission Buffer
extern uint8_t *IE;             // Int Enable
extern uint8_t *SCON;           // Serial Control
extern uint8_t *TH1;            // Timer1 High
extern uint8_t *TH0;            // Timer0 High
extern uint8_t *TL1;            // Timer1 Low
extern uint8_t *TL0;            // Timer0 Low
extern uint8_t *TMOD;           // Timer Mode
extern uint8_t *TCON;           // Timer Control
extern uint8_t *PCON;           // Power Control
extern uint8_t *R;              // Additional 8 Rx Registers
extern uint16_t PC;                  // Progam Counter

uint8_t check_Bit(uint8_t bit);

#endif /* EMU51_EMU51_H */