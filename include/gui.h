/*******************************************************************************
 * Emu51
 * gui.h:
 ******************************************************************************/

#ifndef EMU51_GUI_H
#define EMU51_GUI_H

#include <stdint.h>
#include <allegro.h>

bool checkBit(uint8_t tmp_b, int pos);
void draw_SEG_digit(BITMAP *buf, int x, int y, int color, int size, uint8_t segm_code);
void draw_LED_bin(BITMAP *buf, int x, int y, int color, uint8_t code);
void change_char(char *string, char ch, int pos);
void insert_char(char *string, char ch, int pos, int lenght);
void cut_char(char *string, int pos, int lenght);
void change_ext(char *s);
void GetText(char *text, BITMAP *buf, int x, int y, int w, int h, int lenght, char *title);
void ShowMessage(char *text, BITMAP *buf, int x, int y, int w, int h, char *title);
bool QuestionBox(char *text, BITMAP *buf, int x, int y, int w, int h, char *title, int _color);
void ShowMsgEx(char *text, BITMAP *buf, int x, int y, int w, int h, char *title, int _color);

#endif   /* EMU51_GUI_H */
