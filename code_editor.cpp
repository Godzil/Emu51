/*******************************************************************************
 * Emu51
 * code_editor.cpp:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#include <stdio.h>

#include <emu51.h>
#include <code_editor.h>
#include <gui.h>

void code_editor::load_source(char *filename)
{
    FILE *strin;
    int c, i, tp, j = 0;
    strin = NULL;
    strin = fopen(filename, "rb");
    lines = 0;
    if ( strin == NULL )
    {
        ShowMessage("Can't open the file", screen, 200, 200, 200, 60, "Message!");
    }
    else
    {
        //for (c=0;c<65535;c++)
        //{
        //	if (text_edit[c]!=NULL) delete[] text_edit[c];
        //	else break;
        //}

        while ( !feof(strin))
        {
            alloc_new_line();
            for ( i = 0 ; ( i < 256 ) && !feof(strin) ; i++ )
            {
                c = fgetc(strin);
                if ( c == 9 ) //TAB
                {
                    tp = i / tab_size;
                    tp++;
                    tp *= 8;
                    for ( i ; i <= tp ; i++ )
                    {
                        text_edit[j][i] = 32;
                    }
                    i--;
                }
                else
                {
                    if (( c == 10 ) || ( c == 13 ))
                    {
                        if ( c == 13 )
                        {
                            fgetc(strin);
                        }
                        for ( i ; i < 256 ; i++ )
                        {
                            text_edit[j][i] = 10;
                        }
                    }
                    else
                    {
                        text_edit[j][i] = c == -1 ? 10 : c;
                    }
                }
            }
            j++;
        }
        lines = j;
        fclose(strin);
    }
    cp = 0;
    cl = 0;
    scp = 0;
    scl = 0;
    disp();
}

void code_editor::disp(void)
{
    int i, j, cnti, cntj = 0;

    for ( j = cl - scl ; j < cl - scl + _h + 1 && cntj < lines ; j++ )
    {
        cnti = 0;
        bool endf = false;

        for ( i = cp - scp ; i < cp - scp + _w && !endf ; i++ )
        {
            if ( text_edit[j][i] == 10 || text_edit[j][i] == 0 || text_edit[j][i] == 13 )
            {
                endf = true;
            }
            else
            {
                textprintf(surface, mainf, left + cnti * 8, top + cntj * 10, makecol(255, 255, 255), "%c",
                           text_edit[j][i]);
                cnti++;
            }
        }
        for ( i ; i < cp - scp + _w + 1 ; i++ )
        {
            textprintf(surface, mainf, left + cnti * 8, top + cntj * 10, makecol(255, 255, 255), " ");
            cnti++;
        }
        cntj++;
    }
    if ( cntj < _h )
    {
        rectfill(surface, left, top + cntj * 10, surface->w - left, surface->h - left, 0);
    }
}

void code_editor::process(void)
{
    BITMAP *cache;
    int c, i;
    bool exit_ed = false;
    char *filen;
    filen = new char[256];

    cache = create_bitmap(SCREEN_W, SCREEN_H);
    blit(screen, cache, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    clear(surface);
    hline(surface, 2, surface->h - 2, surface->w - 2, makecol(255, 255, 255));
    vline(surface, 2, 2, surface->h - 2, makecol(255, 255, 255));
    vline(surface, surface->w - 2, 2, surface->h - 2, makecol(255, 255, 255));
    for ( c = 2 ; c <= 2 + 10 ; c += 2 )
    {
        hline(surface, 2, c, surface->w - 2, makecol(255, 255, 255));
    }
    textprintf_centre(surface, mainf, surface->w / 2, 4, makecol(255, 255, 255), " Source editor ");
    lines = 0;
    alloc_new_line();
    uint16_t key_p;
    uint16_t key_ch;
    while ( !exit_ed )
    {
        key_p = readkey();
        key_ch = key_p & 0xff;
        key_p = key_p >> 8;

        switch ( key_p )
        {
        case KEY_BACKSPACE:
            if ( cp > 0 )
            {
                clear_cr();
                if ( scp > 0 )
                {
                    scp--;
                }
                cp--;
                backspace();
            }
            else
            {
                if ( cl > 0 )
                {
                    clear_cr();
                    for ( c = 0 ; ( c < 256 ) && ( text_edit[cl - 1][c] != 10 ) ; c++ )
                    {
                    }
                    cp = c;
                    scp = cp;
                    if ( scp >= _w )
                    {
                        scp = _w - 1;
                    }

                    for ( c ; c < 256 ; c++ )
                    {
                        text_edit[cl - 1][c] = text_edit[cl][c - cp];
                        if ( text_edit[cl][c - cp] == 10 )
                        {
                            break;
                        }
                    }
                    delete[]text_edit[cl];
                    for ( i = cl ; i < lines ; i++ )
                    {
                        text_edit[i] = text_edit[i + 1];
                    }
                    lines--;
                    text_edit[lines] = NULL;
                    cl--;
                    if ( scl > 0 )
                    {
                        scl--;
                    }
                }
            }
            break;
        case KEY_DEL:
            backspace();
            break;
        case KEY_ENTER:
            if ( cl < 65536 )
            {
                clear_cr();
                cl++;
                if ( scl < _h )
                {
                    scl++;
                }
                if ( !insert_new_line(cl))
                {
                    ShowMessage("Out of memory", surface, 200, 100, 200, 60, "Error!");
                }
                else
                {
                    for ( c = cp ; c < 256 ; c++ )
                    {
                        text_edit[cl][c - cp] = text_edit[cl - 1][c];
                        text_edit[cl - 1][c] = 10;
                    }
                    cp = 0;
                    scp = 0;
                }

            }
            else
            {
                ShowMessage("Source can't has more then 65536 lines of code!!!", surface, 200, 100, 200, 60, "Error!");
            }
            break;
        case KEY_END:
            clear_cr();
            for ( cp = 0 ; cp < 256 && text_edit[cl][cp] != 10 ; cp++ )
            {
            }
            scp = cp;
            if ( scp >= _w )
            {
                scp = _w - 1;
            }
            break;
        case KEY_HOME:
            clear_cr();
            cp = 0;
            scp = 0;
            break;
        case KEY_LEFT:
            if ( cp > 0 )
            {
                clear_cr();
                cp--;
                if ( scp > 0 )
                {
                    scp--;
                }
            }
            else
            {
                if ( cl > 0 )
                {
                    clear_cr();
                    for ( c = 0 ; ( c < 256 ) && ( text_edit[cl - 1][c] != 10 ) ; c++ )
                    {
                    }
                    cp = c;
                    scp = cp;
                    if ( scp >= _w )
                    {
                        scp = _w - 1;
                    }

                    cl--;
                    if ( scl > 0 )
                    {
                        scl--;
                    }
                }
            }
            break;
        case KEY_RIGHT:
            if ( cp < 256 && text_edit[cl][cp] != 10 )
            {
                clear_cr();
                cp++;
                if ( scp < _w )
                {
                    scp++;
                }
            }
            else
            {
                if ( cl + 1 < lines )
                {
                    clear_cr();
                    cp = 0;
                    scp = cp;
                    if ( scp >= _w )
                    {
                        scp = _w - 1;
                    }
                    cl++;
                    if ( scl < _h )
                    {
                        scl++;
                    }
                }
            }
            break;
        case KEY_UP:
            if ( cl > 0 )
            {
                clear_cr();
                cl--;
                if ( scl > 0 )
                {
                    scl--;
                }
                for ( c = 0 ; c < 256 && text_edit[cl][c] != 10 ; c++ )
                {
                }
                if ( c - 1 < cp )
                {
                    cp = c;
                    scp = cp;
                    if ( scp >= _w )
                    {
                        scp = _w - 1;
                    }
                }
            }
            break;
        case KEY_DOWN:
            if ( cl < 65536 && cl < ( lines - 1 ))
            {
                clear_cr();
                cl++;
                if ( scl < _h )
                {
                    scl++;
                }
                for ( c = 0 ; c < 256 && text_edit[cl][c] != 10 ; c++ )
                {
                }
                if ( c - 1 < cp )
                {
                    cp = c;
                    scp = cp;
                    if ( scp >= _w )
                    {
                        scp = _w - 1;
                    }
                }
            }
            break;
        case KEY_ESC:
            if ( QuestionBox("Are you sure want to EXIT?", screen, 200, 200, 400, 60, "Confirm!", makecol(255, 0, 0)))
            {
                exit_ed = true;
                for ( c = 0 ; c < lines ; c++ )
                {
                    if ( text_edit[c] != NULL )
                    {
                        delete[] text_edit[c];
                    }
                }
                lines = 0;
            }
            else
            {
                rest(500);
                clear_keybuf();
            }
            break;
        case KEY_F3:
            GetText(filen, screen, 100, 200, 600, 60, 256, " Enter source code file name ");
            for ( c = 254 ; filen[c] == ' ' || filen[c] == '\r' ; c-- )
            {
                filen[c] = 0;
            }
            load_source(filen);
            break;
        default:
            if ( key_ch != 0 && cp < 256 )
            {
                insert_char(cl, key_ch);
                clear_cr();
                cp++;
                if ( scp < _w )
                {
                    scp++;
                }
                key_ch = 0;
            }
            break;
        }
        if ( !exit_ed )
        {
            disp();
            draw_cr();
        }
        textprintf(surface, mainf, 1, 1, makecol(255, 255, 255), " p%d l%d sp%d sl%d c%d ln%d", cp, cl, scp, scl,
                   text_edit[cl][cp], lines);

        blit(surface, screen, 0, 0, 0, 0, surface->w, surface->h);

    }

    blit(cache, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    destroy_bitmap(cache);
    delete[] filen;
    return;
}

void code_editor::insert_char(int ln, char ch)
{
    int i;
    for ( i = 256 ; i > cp ; i-- )
    {
        text_edit[cl][i] = text_edit[cl][i - 1];
    }
    text_edit[cl][cp] = ch;
}

void code_editor::backspace(void)
{
    int i;
    for ( i = cp ; i < 256 ; i++ )
    {
        text_edit[cl][i] = text_edit[cl][i + 1];
    }
}

void code_editor::draw_cr(void)
{
    hline(surface, left + 8 * scp, top + 10 * scl + 9, left + 8 * scp + 8, makecol(255, 255, 255));
}

void code_editor::clear_cr(void)
{
    hline(surface, left + 8 * scp, top + 10 * scl + 9, left + 8 * scp + 8, 0);
}

code_editor::code_editor(void)
{
    int i;
    for ( i = 0 ; i < 65536 ; i++ )
    {
        text_edit[i] = NULL;
    }
    lines = 0;
    top = 24;
    left = 11;
    cl = 0;
    cp = 0;
    scl = 0;
    scp = 0;
    surface = create_bitmap(SCREEN_W, SCREEN_H - 100);
    _w = ( surface->w - left * 2 ) / 8 - 1;
    _h = ( surface->h - top - left ) / 10 - 1;
    tab_size = 8;
}

code_editor::~code_editor(void)
{
}

bool code_editor::alloc_new_line(void)
{
    int i;
    if ( lines < 65536 )
    {
        text_edit[lines] = new char[256];
        for ( i = 0 ; i < 256 ; i++ )
        {
            text_edit[lines][i] = 10;
        }
        lines++;
        return text_edit[lines] == NULL ? false : true;
    }
    return false;
}

bool code_editor::insert_new_line(int ln)
{
    int i;
    if ( lines < 65536 )
    {
        for ( i = lines ; i > ln ; i-- )
        {
            text_edit[i] = text_edit[i - 1];
        }
        text_edit[ln] = NULL;
        text_edit[ln] = new char[256];
        for ( i = 0 ; i < 256 ; i++ )
        {
            text_edit[ln][i] = 10;
        }
        lines++;
        if ( text_edit[ln] == NULL )
        {
            for ( i = ln ; i < lines ; i++ )
            {
                text_edit[i] = text_edit[i + 1];
            }
            lines--;
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

void code_editor::delete_last(void)
{
    lines--;
    delete[] text_edit[lines];
    text_edit[lines] = NULL;
}

void code_editor::cut_line(int ln)
{
    int i;
    if ( text_edit[ln] != NULL )
    {
        for ( i = ln ; i < lines ; i++ )
        {
            text_edit[i] = text_edit[i + 1];
        }
        lines--;
    }
}
