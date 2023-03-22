/*******************************************************************************
 * Emu51
 * regs.h:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#ifndef EMU51_REGS_H
#define EMU51_REGS_H

#include <stdint.h>
#include <allegro.h>

class regs
{
private:
    void hexoutB(int x, int y, int color, uint8_t numb);

public:
    int frame;
    int left;
    bool changed;
    uint8_t *sfr;
    uint8_t reg[25];
    char reg_label[25][10];
    BITMAP *surface, *buf;

    void draw();

    void blit_it(int, int);

    int red;
    int white;
    int lblue;
    int green;
    int lred;

    regs(uint8_t *, BITMAP *);

    ~regs();
};

#endif /* EMU51_REGS_H */
