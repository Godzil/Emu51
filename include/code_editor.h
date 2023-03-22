/*******************************************************************************
 * Emu51
 * code_editor.h:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#ifndef EMU51_CODE_EDITOR_H
#define EMU51_CODE_EDITOR_H

#include <allegro.h>

class code_editor
{
private:
    char *text_edit[65536];
    int lines;
    int cl, cp;
    int scl, scp;
    int _w, _h;

public:
    code_editor(void);

    ~code_editor(void);

    bool alloc_new_line(void); // ret. true on success
    bool insert_new_line(int ln);

    void delete_last(void);

    void cut_line(int ln);

    void insert_char(int ln, char ch);

    void replace_char(int ln, char ch);

    void backspace(void);

    void del_ch(int ln);

    void draw_cr(void);

    void clear_cr(void);

    void load_source(char *filename);

    void save_source(char *filename);

    int compile(int *err); // out-num errors, in-table of int's
    // thet represtents lines with errors
    // (automactly allocated)
    int left, top;
    int tab_size;
    BITMAP *surface;

    void process(void);

    void disp(void);
};

#endif /* EMU51_CODE_EDITOR_H */
