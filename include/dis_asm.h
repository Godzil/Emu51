/*******************************************************************************
 * Emu51
 * dis_asm.h:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#ifndef EMU51_DIS_ASM_H
#define EMU51_DIS_ASM_H

#include <stdint.h>
#include <allegro.h>

class dis_asm                     // display code class
{
private:
    void hexoutB(int, int, int, uint8_t);

public:
    uint8_t *ram;                 // pointer to external ram table
    BITMAP *buf, *surface;        // screen buffer, work surface
    int frame;
    int left;
    bool changed;

    dis_asm(uint8_t *, BITMAP *); // ext. ram, draw bitmap, constructor
    void blit_it(int, int);

    void draw(uint16_t);          // x,y,PC (PC is an adr. of register which string is displayed in the middle of the code monitor)
    ~dis_asm();
};

#endif /* EMU51_DIS_ASM_H */
