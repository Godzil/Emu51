/*******************************************************************************
 * Emu51
 * flags.h:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#ifndef EMU51_FLAGS_H
#define EMU51_FLAGS_H

#include <stdint.h>
#include <allegro.h>

class flags
{
private:
    void hexoutB(int x, int y, int color, uint8_t numb);

public:
    int frame;
    int left;
    bool changed;
    uint8_t *sfr;
    BITMAP *surface, *buf;

    void draw();

    void blit_it(int, int);

    flags(BITMAP *);

    ~flags();
};

#endif /* EMU51_FLAGS_H */
