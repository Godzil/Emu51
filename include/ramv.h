/*******************************************************************************
 * Emu51
 * ramv.h:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#ifndef EMU51_RAMV_H
#define EMU51_RAMV_H

#include <stdint.h>
#include <allegro.h>

class ramv
{
private:
    void hexoutB(int x, int y, int color, uint8_t numb);

public:
    int frame;
    int left;
    bool changed;
    uint8_t reg[25];
    char reg_label[25][10];
    BITMAP *surface, *buf;

    void draw(uint16_t);

    void blit_it(int, int);

    ramv(BITMAP *);

    ~ramv();
};

#endif /* EMU51_RAMV_H */
