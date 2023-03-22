/*******************************************************************************
 * Emu51
 * code_51.h:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#ifndef EMU51_CODE_51_H
#define EMU51_CODE_51_H

#include <stdint.h>

class code_51
{
public:
    char mnem[6];              // instruction mnemonic (2-4 characters)
    uint8_t code;              // instruction code
    uint8_t lenght;            // bytes which are needed to write this instruction into memory
    uint8_t cycles;            // time unit
    char display_string[20];   // string which is ready for displaying, it's made by make_ds(WORD) method
    char datas[12];            // string which contains datas which will be displayed after mnemonic
    void make_ds(uint16_t);    // make display string
    void process();            // process the instruction
};

extern code_51 asm51[256];
extern unsigned long c_time;

uint8_t check_C(void);
uint8_t check_AC(void);
uint8_t check_OV(void);
uint8_t check_P(void);

void set_C(void);
void clr_C(void);
void set_Bit(uint8_t bit);
void clr_Bit(uint8_t bit);

#endif /* EMU51_CODE_51_H */
