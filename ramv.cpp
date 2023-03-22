/*******************************************************************************
 * Emu51
 * ramv.cpp:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#include <emu51.h>
#include <ramv.h>
#include <gui.h>

ramv::ramv(BITMAP *tmpBMP) // extram, screen
{
    frame = 2;
    left = 12;
    changed = true;
    buf = tmpBMP;
    surface = create_bitmap(500, 200);
    clear(surface);
}

ramv::~ramv()
{
}

void ramv::blit_it(int x, int y)
{
    blit(surface, buf, 0, 0, x, y, surface->w, surface->h);
}

void ramv::draw(uint16_t cur)
{
    int red = makecol(255, 0, 0);
    int white = makecol(255, 255, 255);
    int lgreen = makecol(128, 255, 128);
    int green = makecol(0, 255, 0);
    int lblue = makecol(0, 128, 255);
    int yellow = makecol(255, 255, 0);
    clear(surface);
    int c2 = 0, c;
    int y;
    y = frame + 20;
    int curcol = white;
    for ( c = cur ; true ; c += 8 )
    {
        if ( !( y < surface->h - 10 ))
        {
            break;
        }
        textprintf(surface, mainf, left, y, lblue, "%4X: ", (uint16_t) ( c ));
        for ( c2 = 0 ; c2 < 8 ; c2++ )
        {
            if ( PC == (uint16_t) ( c + c2 ))
            {
                curcol = red;
                draw_SEG_digit(surface, left + 326 + c2 * 8, y, yellow, 3, ram[(uint16_t) ( c + c2 )]);
            }
            else if ( *DPTR == (uint16_t) ( c + c2 ))
            {
                curcol = lgreen;
                draw_SEG_digit(surface, left + 326 + c2 * 8, y, yellow, 3, ram[(uint16_t) ( c + c2 )]);
            }
            else
            {
                curcol = white;
                draw_SEG_digit(surface, left + 326 + c2 * 8, y, green, 3, ram[(uint16_t) ( c + c2 )]);
            }
            hexoutB(left + 48 + c2 * 24, y, curcol, ram[(uint16_t) ( c + c2 )]);
            textprintf(surface, font, left + 252 + c2 * 8, y, curcol, "%c", ram[(uint16_t) ( c + c2 )]);

        }
        y += 10;
    }
    hline(surface, frame, surface->h - frame, surface->w - frame, white);
    vline(surface, frame, frame, surface->h - frame, white);
    vline(surface, surface->w - frame, frame, surface->h - frame, white);
    for ( c = frame ; c <= frame + 10 ; c += 2 )
    {
        hline(surface, frame, c, surface->w - frame, white);
    }
    textprintf_centre(surface, mainf, surface->w / 2, frame + 2, white, " RAM Monitor ");
}

void ramv::hexoutB(int x, int y, int color, uint8_t numb)
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