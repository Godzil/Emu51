/*******************************************************************************
 * Emu51
 * dis_asm.cpp:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#include <emu51.h>
#include <code_51.h>
#include <dis_asm.h>

dis_asm::dis_asm(uint8_t *tmpB, BITMAP *tmp_buf)
{
    frame = 2;
    left = 12;
    changed = true;
    ram = tmpB;
    buf = tmp_buf;
    surface = create_bitmap(300, 500);
    clear(surface);
}

void dis_asm::hexoutB(int x, int y, int color, uint8_t numb)
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

void dis_asm::draw(uint16_t adress)
{
    int red = makecol(255, 0, 0);
    int white = makecol(255, 255, 255);
    int lblue = makecol(0, 128, 255);
    int color = white;
    int colPC = white;
    int c, cnt;
    clear(surface);
    for ( c = 20 ; c < surface->h - 10 ; c += 10 )
    {
        if ( PC == adress )
        {
            color = red;
        }
        else
        {
            color = white;
        }
        asm51[ram[adress]].make_ds(adress);
        if ( asm51[ram[adress]].code == ram[adress] )
        {
            textprintf(surface, mainf, left, c, color, "%4x:", adress);
            for ( cnt = 0 ; cnt < asm51[ram[adress]].lenght ; cnt++ )
            {
                if ( PC == ( adress + cnt ))
                {
                    colPC = lblue;
                }
                else
                {
                    colPC = color;
                }
                hexoutB(left + 50 + cnt * 24, c, colPC, ram[adress + cnt]);
            }
            textprintf(surface, mainf, left + 144, c, color, "%s", asm51[ram[adress]].display_string);
            adress += asm51[ram[adress]].lenght;
        }
        else
        {
            textprintf(surface, mainf, left, c, color, "%4x: ", adress);
            if ( PC == adress )
            {
                colPC = lblue;
            }
            else
            {
                colPC = color;
            }
            hexoutB(left + 50, c, colPC, ram[adress]);
            textprintf(surface, mainf, left + 144, c, color, "???");
            adress++;
        }
    }
    hline(surface, frame, surface->h - frame, surface->w - frame, white);
    vline(surface, frame, frame, surface->h - frame, white);
    vline(surface, surface->w - frame, frame, surface->h - frame, white);
    for ( c = frame ; c <= frame + 10 ; c += 2 )
    {
        hline(surface, frame, c, surface->w - frame, white);
    }
    textprintf_centre(surface, mainf, surface->w / 2, frame + 2, white, " Disassembler ");
}

void dis_asm::blit_it(int x, int y)
{
    blit(surface, buf, 0, 0, x, y, surface->w, surface->h);
}

dis_asm::~dis_asm(void)
{
}