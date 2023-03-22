/*******************************************************************************
 * Emu51
 * regs.cpp:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#include <emu51.h>
#include <regs.h>
#include <gui.h>

regs::regs(uint8_t *tmpB, BITMAP *tmpBMP)
{
    red = makecol(255, 0, 0);
    white = makecol(255, 255, 255);
    lblue = makecol(0, 128, 255);
    green = makecol(0, 255, 0);
    lred = makecol(255, 64, 0);
    frame = 2;
    left = 12;
    changed = true;
    sfr = tmpB;
    buf = tmpBMP;
    surface = create_bitmap(400, 300);
    clear(surface);
};

regs::~regs()
{};

void regs::blit_it(int x, int y)
{
    blit(surface, buf, 0, 0, x, y, surface->w, surface->h);
};

void regs::draw()
{
    clear(surface);
    int c2 = 0, c;
    textprintf(surface, mainf, left, frame + 20, lblue, "nr  label    adress  hex dec ascii SEG LED(bin)");
    for ( c = frame + 30 ; c < surface->h - 20 ; c += 10 )
    {
        textprintf(surface, mainf, left, c, white, "%2d: %9s (%2x) =  %2x %3d   %c", c2 + 1, reg_label[c2], reg[c2],
                   sfr[reg[c2]], sfr[reg[c2]], sfr[reg[c2]]);
        draw_SEG_digit(surface, left + 288, c, green, 4, sfr[reg[c2]]);
        draw_LED_bin(surface, left + 314, c + 4, lred, sfr[reg[c2]]);
        c2++;
        if ( c2 == 25 )
        {
            break;
        }
    };
    hline(surface, frame, surface->h - frame, surface->w - frame, white);
    vline(surface, frame, frame, surface->h - frame, white);
    vline(surface, surface->w - frame, frame, surface->h - frame, white);
    for ( c = frame ; c <= frame + 10 ; c += 2 )
    {
        hline(surface, frame, c, surface->w - frame, white);
    }
    textprintf_centre(surface, mainf, surface->w / 2, frame + 2, white, " SFR Registers ");
}

void regs::hexoutB(int x, int y, int color, uint8_t numb)
{
    if ( numb > 0x10 )
    {
        textprintf(surface, mainf, x, y, color, "%2x", numb);
    }
    else
    {
        textprintf(surface, mainf, x, y, color, "%2x", numb);
        textprintf(surface, mainf, x, y, color, "0");
    }
}
