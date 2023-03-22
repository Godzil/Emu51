/*******************************************************************************
 * Emu51
 * flags.cpp:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#include <emu51.h>
#include <flags.h>
#include <code_51.h>

flags::flags(BITMAP *tmp_buf)
{
    buf = tmp_buf;
    surface = create_bitmap(100, 300);
    frame = 2;
    left = 12;
    changed = false;
}

flags::~flags()
{
}

void flags::draw()
{
    int red = makecol(255, 0, 0);
    int white = makecol(255, 255, 255);
    int lblue = makecol(0, 128, 255);
    clear(surface);
    textprintf(surface, mainf, left, frame + 20, white, "C  : %d", check_C());
    textprintf(surface, mainf, left, frame + 30, white, "AC : %d", check_AC());
    textprintf(surface, mainf, left, frame + 40, white, "OV : %d", check_OV());
    textprintf(surface, mainf, left, frame + 50, white, "P  : %d", check_P());
    textprintf(surface, mainf, left, frame + 60, red, "PC : %4x", PC);
    textprintf(surface, mainf, left, frame + 70, lblue, "SP : %2x", *SP);
    textprintf(surface, mainf, left, frame + 80, lblue, "Acc: %2x", *Acc);
    textprintf(surface, mainf, left, frame + 90, lblue, "B  : %2x", *B);
    textprintf(surface, mainf, left, frame + 100, lblue, "R0 : %2x", R[0]);
    textprintf(surface, mainf, left, frame + 110, lblue, "R1 : %2x", R[1]);
    textprintf(surface, mainf, left, frame + 120, lblue, "R2 : %2x", R[2]);
    textprintf(surface, mainf, left, frame + 130, lblue, "R3 : %2x", R[3]);
    textprintf(surface, mainf, left, frame + 140, lblue, "R4 : %2x", R[4]);
    textprintf(surface, mainf, left, frame + 150, lblue, "R5 : %2x", R[5]);
    textprintf(surface, mainf, left, frame + 160, lblue, "R6 : %2x", R[6]);
    textprintf(surface, mainf, left, frame + 170, lblue, "R7 : %2x", R[7]);
    textprintf(surface, mainf, left, frame + 180, lblue, "DPTR: %4x", *DPTR);
    textprintf(surface, mainf, left, frame + 270, red, "cycles :");
    textprintf(surface, mainf, left, frame + 280, red, "%lld", c_time);

    hline(surface, frame, surface->h - frame, surface->w - frame, white);
    vline(surface, frame, frame, surface->h - frame, white);
    vline(surface, surface->w - frame, frame, surface->h - frame, white);
    for ( int c = frame ; c <= frame + 10 ; c += 2 )
    {
        hline(surface, frame, c, surface->w - frame, white);
    }
    textprintf_centre(surface, mainf, surface->w / 2, frame + 2, white, " RegFlag ");
}

void flags::blit_it(int x, int y)
{
    blit(surface, buf, 0, 0, x, y, surface->w, surface->h);
}