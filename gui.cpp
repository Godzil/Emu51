/*******************************************************************************
 * Emu51
 * gui.cpp:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>

#include <emu51.h>
#include <gui.h>

// Returns state of a single bit which is on 'pos' position in tmp_b byte
bool checkBit(uint8_t tmp_b, int pos)
{
    if ( tmp_b & ( 1 << pos ))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void draw_SEG_digit(BITMAP *buf, int x, int y, int color, int size, uint8_t segm_code)
{
    int darkc, normc;
    normc = color;
    darkc = makecol(getr(normc) / 4, getg(normc) / 4, getb(normc) / 4);

    if ( checkBit(segm_code, 1))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    vline(buf, x + size, y, y + size, color);

    if ( checkBit(segm_code, 2))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    vline(buf, x + size, y + size, y + 2 * size, color);

    if ( checkBit(segm_code, 4))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    vline(buf, x, y + 2 * size, y + size, color);

    if ( checkBit(segm_code, 5))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    vline(buf, x, y + size, y, color);

    if ( checkBit(segm_code, 6))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    hline(buf, x, y + size, x + size, color);

    if ( checkBit(segm_code, 0))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    hline(buf, x, y, x + size, color);

    if ( checkBit(segm_code, 3))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    hline(buf, x + size, y + 2 * size, x, color);

    if ( checkBit(segm_code, 7))
    {
        color = normc;
    }
    else
    {
        color = darkc;
    }
    putpixel(buf, x + size + 2, y + 2 * size, color);
}

void draw_LED_bin(BITMAP *buf, int x, int y, int color, uint8_t code)
{
    int darkc, normc;
    int i;
    normc = color;
    darkc = makecol(getr(normc) / 4, getg(normc) / 4, getb(normc) / 4);
    for ( i = 0 ; i < 8 ; i++ )
    {
        if ( checkBit(code, 7 - i))
        {
            color = normc;
        }
        else
        {
            color = darkc;
        }
        circlefill(buf, x + 7 * i, y, 2, color);
    }
}

void change_char(char *string, char ch, int pos)
{
    string[pos - 1] = ch;
}

void insert_char(char *string, char ch, int pos, int lenght)
{
    for ( int c = lenght ; c > pos ; c-- )
    {
        string[c - 1] = string[c - 2];
    }
    string[pos - 1] = ch;
}

void cut_char(char *string, int pos, int lenght)
{
    for ( int c = pos ; c < lenght ; c++ )
    {
        string[c - 1] = string[c];
    }
    string[lenght - 1] = (char) ( 32 );
}

void change_ext(char *s)
{
    int c;
    for ( c = 254 ; c >= 0 ; c-- )
    {
        if ( s[c] == '.' )
        {
            s[c + 1] = 'h';
            s[c + 2] = 'e';
            s[c + 3] = 'x';
            break;
        }
    }
    return;
}

void GetText(char *text, BITMAP *buf, int x, int y, int w, int h, int lenght, char *title)
{
    int c;
    BITMAP *surface = create_bitmap(w, h);
    BITMAP *cache = create_bitmap(w, h);
    blit(buf, cache, x, y, 0, 0, w, h);
    clear(surface);
    hline(surface, 2, surface->h - 2, surface->w - 2, makecol(255, 255, 255));
    vline(surface, 2, 2, surface->h - 2, makecol(255, 255, 255));
    vline(surface, surface->w - 2, 2, surface->h - 2, makecol(255, 255, 255));

    for ( c = 2 ; c <= 2 + 10 ; c += 2 )
    {
        hline(surface, 2, c, surface->w - 2, makecol(255, 255, 255));
    }
    textprintf_centre(surface, mainf, surface->w / 2, 4, makecol(255, 255, 255), " %s ", title);
    text[0] = 32;

    for ( c = 1 ; c <= lenght - 1 ; c++ )
    {
        text[c] = 32;
    }
    text[lenght - 1] = 0;
    clear_keybuf();
    int pos = 1;
    uint16_t key_p;
    uint16_t key_ch;
    bool flag;

    while ( key[KEY_ENTER] )
    {
    }

    while ( !key[KEY_ENTER] )
    {
        textprintf(surface, mainf, 15, surface->h / 2, makecol(255, 255, 255), "%s", text);
        hline(surface, 15 + ( pos - 1 ) * 8, surface->h / 2 + 8, 15 + pos * 8, makecol(255, 255, 255));
        blit(surface, buf, 0, 0, x, y, w, h);
        hline(surface, 15 + ( pos - 1 ) * 8, surface->h / 2 + 8, 15 + pos * 8, makecol(0, 0, 0));
        key_p = readkey();
        key_ch = key_p & 0xff;
        key_p = key_p >> 8;
        flag = false;
        switch ( key_p )
        {
        case KEY_BACKSPACE:
            if ( pos > 1 )
            {
                pos--;
                cut_char(text, pos, lenght);
                text[lenght - 2] = 32;
                text[lenght - 1] = 0;
            }
            flag = true;
            break;
        case KEY_DEL:
            cut_char(text, pos, lenght);
            text[lenght - 2] = 32;
            text[lenght - 1] = 0;
            flag = true;
            break;
        case KEY_LEFT:
            if ( pos > 1 )
            { pos--; }
            flag = true;
            break;
        case KEY_RIGHT:
            if ( pos < lenght )
            { pos++; }
            flag = true;
            break;
        }
        if ( !key[KEY_ENTER] && key_ch != 0 && pos < lenght && !flag )
        {
            insert_char(text, key_ch, pos, lenght);
            pos++;
            text[lenght - 1] = 0;
        }
    }

    text[lenght - 1] = 0;
    blit(cache, buf, 0, 0, x, y, w, h);
    destroy_bitmap(cache);
    destroy_bitmap(surface);
}

void ShowMessage(char *text, BITMAP *buf, int x, int y, int w, int h, char *title)
{
    BITMAP *surface = create_bitmap(w, h);
    BITMAP *cache = create_bitmap(w, h);
    blit(buf, cache, x, y, 0, 0, w, h);
    clear(surface);
    hline(surface, 2, surface->h - 2, surface->w - 2, makecol(255, 255, 255));
    vline(surface, 2, 2, surface->h - 2, makecol(255, 255, 255));
    vline(surface, surface->w - 2, 2, surface->h - 2, makecol(255, 255, 255));
    for ( int c = 2 ; c <= 2 + 10 ; c += 2 )
    {
        hline(surface, 2, c, surface->w - 2, makecol(255, 255, 255));
    }
    textprintf_centre(surface, mainf, surface->w / 2, 4, makecol(255, 255, 255), " %s ", title);
    textprintf(surface, mainf, 15, surface->h / 2, makecol(255, 255, 255), "%s", text);
    textprintf_centre(surface, mainf, surface->w / 2, surface->h - 16, makecol(255, 0, 0), "Press ENTER");
    blit(surface, buf, 0, 0, x, y, w, h);

    while ( key[KEY_ENTER] )
    {
    }

    while ( !key[KEY_ENTER] )
    {
    }

    blit(cache, buf, 0, 0, x, y, w, h);
    destroy_bitmap(cache);
    destroy_bitmap(surface);
}

bool QuestionBox(char *text, BITMAP *buf, int x, int y, int w, int h, char *title, int _color)
{
    BITMAP *surface = create_bitmap(w, h);
    BITMAP *cache = create_bitmap(w, h);
    int len, c, line = 0, pos = 0;
    blit(buf, cache, x, y, 0, 0, w, h);
    clear(surface);
    hline(surface, 2, surface->h - 2, surface->w - 2, makecol(255, 255, 255));
    vline(surface, 2, 2, surface->h - 2, makecol(255, 255, 255));
    vline(surface, surface->w - 2, 2, surface->h - 2, makecol(255, 255, 255));
    for ( c = 2 ; c <= 2 + 10 ; c += 2 )
    {
        hline(surface, 2, c, surface->w - 2, makecol(255, 255, 255));
    }
    textprintf_centre(surface, mainf, surface->w / 2, 4, makecol(255, 255, 255), " %s ", title);
    textprintf_centre(surface, mainf, surface->w / 2, surface->h - 16, makecol(255, 0, 0),
                      "Press ENTER (OK)  or  ESC (Cancel)");
    len = (int) strlen(text);
    for ( c = 0 ; c < len ; c++ )
    {
        if ( text[c] == 10 )
        {
            line++;
            pos = 0;
        }
        else
        {
            if ( text[c] != 13 )
            {
                if ( text[c] > 126 || text[c] < 32 )
                { break; }
                textprintf(surface, mainf, 15 + pos * 8, 20 + line * 10, _color, "%c", text[c]);
                pos++;
            }
        }
    }
    blit(surface, buf, 0, 0, x, y, w, h);

    while ( key[KEY_ENTER] || key[KEY_ESC] )
    {
    }

    while ( !( key[KEY_ENTER] || key[KEY_ESC] ))
    {
    }

    if ( key[KEY_ENTER] )
    {
        return true;
    }
    else
    {
        return false;
    }

    blit(cache, buf, 0, 0, x, y, w, h);
    destroy_bitmap(cache);
    destroy_bitmap(surface);
}

void ShowMsgEx(char *text, BITMAP *buf, int x, int y, int w, int h, char *title, int _color)
{
    BITMAP *surface = create_bitmap(w, h);
    BITMAP *cache = create_bitmap(w, h);
    int len, c, line = 0, pos = 0;
    blit(buf, cache, x, y, 0, 0, w, h);
    clear(surface);
    hline(surface, 2, surface->h - 2, surface->w - 2, makecol(255, 255, 255));
    vline(surface, 2, 2, surface->h - 2, makecol(255, 255, 255));
    vline(surface, surface->w - 2, 2, surface->h - 2, makecol(255, 255, 255));

    for ( c = 2 ; c <= 2 + 10 ; c += 2 )
    {
        hline(surface, 2, c, surface->w - 2, makecol(255, 255, 255));
    }
    textprintf_centre(surface, mainf, surface->w / 2, 4, makecol(255, 255, 255), " %s ", title);
    textprintf_centre(surface, mainf, surface->w / 2, surface->h - 16, makecol(255, 0, 0), "Press ENTER");
    len = (int) strlen(text);

    for ( c = 0 ; c < len ; c++ )
    {
        if ( text[c] == 10 )
        {
            line++;
            pos = 0;
        }
        else
        {
            if ( text[c] != 13 )
            {
                if ( text[c] > 126 || text[c] < 32 )
                { break; }
                textprintf(surface, mainf, 15 + pos * 8, 20 + line * 10, _color, "%c", text[c]);
                pos++;
            }
        }
    }
    blit(surface, buf, 0, 0, x, y, w, h);

    while ( key[KEY_ENTER] )
    {
    }

    while ( !key[KEY_ENTER] )
    {
    }

    blit(cache, buf, 0, 0, x, y, w, h);
    destroy_bitmap(cache);
    destroy_bitmap(surface);
}
