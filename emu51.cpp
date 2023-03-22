#include <stdio.h>
#include <stdint.h>

#define USE_CONSOLE

#include <allegro.h>
#include <string.h>
#include <stdlib.h>

#include <code_51.h>
#include <code_editor.h>
#include <dis_asm.h>
#include <flags.h>
#include <gui.h>
#include <ramv.h>
#include <regs.h>

#include <emu51.h>

FONT *mainf;
int white, red, lred, green, blue;

uint8_t SFR[0x100];                   // internal memory and sfr area 256 bytes
uint8_t EXT_RAM[0x10000];             // external memory 64kB
uint8_t EXT_PMEM[0x10000];            // external memory 64kB for program
uint8_t *Acc = &SFR[0xe0];            // 8-bit accumlator
uint16_t *DPTR = (uint16_t *) &SFR[0x82];  // 16-bit register
uint8_t *DPH = &SFR[0x83];            // high byte of DPTR
uint8_t *DPL = &SFR[0x82];            // low byte of DPTR
uint8_t *B = &SFR[0xf0];              // B register
uint8_t *SP = &SFR[0x81];             // Stack Pointer
uint8_t *PSW = &SFR[0xd0];            // Program Status Word
uint8_t *P0 = &SFR[0x80];             // 1st Port
uint8_t *P1 = &SFR[0x90];             // 2nd Port
uint8_t *P2 = &SFR[0xa0];             // 3rd Port
uint8_t *P3 = &SFR[0xb0];             // 4th port
uint8_t *SBUF = &SFR[0x99];           // Serial transmission Buffer
uint8_t *IE = &SFR[0xa8];             // Int Enable
uint8_t *SCON = &SFR[0x98];           // Serial Control
uint8_t *TH1 = &SFR[0x8d];            // Timer1 High
uint8_t *TH0 = &SFR[0x8c];            // Timer0 High
uint8_t *TL1 = &SFR[0x8b];            // Timer1 Low
uint8_t *TL0 = &SFR[0x8a];            // Timer0 Low
uint8_t *TMOD = &SFR[0x89];           // Timer Mode
uint8_t *TCON = &SFR[0x88];           // Timer Control
uint8_t *PCON = &SFR[0x87];           // Power Control
uint8_t *R = &SFR[0x00];              // Additional 8 Rx Registers
uint16_t PC = 0x0000;                  // Progam Counter

uint8_t *ram = EXT_RAM;
uint8_t *prog = EXT_PMEM;

// initialize the emulator
void init_8051(void)
{
    int i;
    c_time = 0;                    // cycles counter
    for ( i = 0 ; i < 256 ; i++ )
    {
        SFR[i] = 0xA5;             // all regs in internal RAM & SFR
    }
    *P0 = 0xff;                    //
    *P1 = 0xff;                    //
    *P2 = 0xff;                    //   Some SFR registers
    *P3 = 0xff;                    //   and Program Counter
    PC = 0x0000;                   //
    *SP = 0x07;                    //
}

// 8051 assembler macros
int get_lenght(uint8_t code)
{
    if ( asm51[code].code == code )
    {
        return asm51[code].lenght;
    }
    else
    {
        return 1;
    }
}

/*bool vret (void)
{
  if (inportb(0x3da) & 8) return true;
  else return false;

}*/

void wait_for_retrace(void)
{
    /*while (!vret())
  {
  }*/
    //vsync();
}

int load_code_table(code_51 *tmp_tab)
{
    FILE *strio = NULL;
    char _mnem[6];
    int _code;
    int _lenght;
    int _cycles;
    char _datas[12];
    strio = fopen("list51.txt", "r");
    if ( strio )
    {
        for ( int c = 0 ; c < 256 ; c++ )
        {
            fscanf(strio, "%x", &_code);
            fscanf(strio, "%s", _mnem);
            fscanf(strio, "%s", _datas);
            fscanf(strio, "%d", &_lenght);
            fscanf(strio, "%d", &_cycles);
            if ( _mnem[0] == 'E' && _mnem[1] == 'N' && _mnem[2] == 'D' )
            {
                break;
            }
            tmp_tab[_code].code = (uint8_t) _code;
            strcpy(tmp_tab[_code].mnem, _mnem);
            strcpy(tmp_tab[_code].datas, _datas);
            tmp_tab[_code].lenght = (uint8_t) _lenght;
            tmp_tab[_code].cycles = (uint8_t) _cycles;
        }
        fclose(strio);
    }
    else
    {
        ShowMessage("Can't load list51.txt file", screen, 100, 100, 200, 60, "Error!");
    }
    return 0;
}

int load_ram(char *filename, uint8_t *ram)
{
    FILE *strin = NULL;
    if ( *filename != 0 )
    {
        strin = fopen(filename, "rb");
    }
    if ( strin == NULL )
    {
        ShowMessage("Can't open the file", screen, 250, 100, 200, 100, "Error!");
    }
    else
    {
        uint16_t c = 0;
        for ( c ; !feof(strin) ; c++ )
        {
            ram[c] = fgetc(strin);
        }

        fclose(strin);
    }

    return 0;
}

int load_hex(char *filename, uint8_t *ram)
{
    FILE *strin;
    int Adr = 0;
    int Cnt;
    //int to_mem;
    int i;
    char line[50];
    char word[5];
    char byte[3];

    byte[2] = word[4] = 0;
    strin = NULL;

    if ( *filename != 0 )
    {
        strin = fopen(filename, "r");
    }

    if ( strin == NULL )
    {
        ShowMessage("Can't open the file", screen, 250, 100, 200, 100, "Error!");
        return -1;
    }
    else
    {
        while ( !feof(strin))
        {
            fscanf(strin, "%s", line);
            //fprintf (out,"%s\n", line);
            byte[0] = line[1];
            byte[1] = line[2];
            sscanf(byte, "%x", &Cnt);
            word[0] = line[3];
            word[1] = line[4];
            word[2] = line[5];
            word[3] = line[6];
            sscanf(word, "%x", &Adr);
            byte[0] = line[7];
            byte[1] = line[8];
            sscanf(byte, "%x", &i);
            if ( i == 1 )
            {
                break;
            }
            else
            {
                byte[2] = 0;
                for ( i = 9 ; i < 9 + Cnt * 2 ; i += 2 )
                {
                    byte[0] = line[i];
                    byte[1] = line[i + 1];

                    sscanf(byte, "%x", ( ram + Adr ));

                    Adr++;
                }
            }
        }
        fclose(strin);
        //fclose (out);
    }
    return 0;
}

void ChangeMonitedSFRRegister(regs *regwnd)
{
    char lab[10];
    char ln[3];
    int line_num = 0;
    int reg_adr = 0;
    GetText(ln, screen, 50, 50, 300, 70, 3, "Enter line number (dec)");
    sscanf(ln, "%d", &line_num);
    if ( line_num < 1 || line_num > 25 )
    {
        return;
    }
    GetText(lab, screen, 50, 80, 300, 70, 10, "Enter reg. label (ascii)");
    GetText(ln, screen, 50, 100, 300, 70, 3, "Enter register number (hex)");
    sscanf(ln, "%x", &reg_adr);
    line_num--;
    strcpy(regwnd->reg_label[line_num], lab);
    regwnd->reg[line_num] = (uint8_t) reg_adr;
    return;
}

void reset51(void)
{
    if ( QuestionBox("Are you sure want to reset 8051", screen, 250, 240, 300, 60, "Confirm!", makecol(255, 255, 255)))
    {
        init_8051();
    }
    return;
}

uint16_t ViewAt(void)
{
    char hex4[6];
    int tmp_adr;
    GetText(hex4, screen, 50, 80, 300, 70, 5, "Enter new view point (hex)");
    if ( hex4[0] == 32 )
    {
        return 0;
    }
    sscanf(hex4, "%x", &tmp_adr);
    return (uint16_t) tmp_adr;
}

void ChangePC(void)
{
    char hex4[5];
    int tmpPC = 0;
    GetText(hex4, screen, 50, 80, 300, 70, 5, "Enter new PC (hex)");
    if ( hex4[0] == 32 )
    {
        return;
    }
    sscanf(hex4, "%x", &tmpPC);
    PC = (uint16_t) tmpPC;
    return;
}

void ChangeRegValue_SFR(void)
{
    char hex2[3];
    char hex2_a[3];
    int tmp_adr;
    int tmp_dat;
    GetText(hex2_a, screen, 50, 80, 300, 70, 3, "Enter adress from SFR (hex)");
    if ( hex2_a[0] == 32 )
    {
        return;
    }
    GetText(hex2, screen, 60, 90, 300, 70, 3, "Enter byte (hex)");
    sscanf(hex2_a, "%x", &tmp_adr);
    sscanf(hex2, "%x", &tmp_dat);
    SFR[(uint8_t) tmp_adr] = (uint8_t) tmp_dat;
    return;
}

void ChangeRegValue_RAM(void)
{
    char hex2[3];
    char hex4_a[5];
    int tmp_adr;
    int tmp_dat;
    GetText(hex4_a, screen, 50, 80, 400, 70, 5, "Enter adress from External Memory (hex)");
    if ( hex4_a[0] == 32 )
    {
        return;
    }
    GetText(hex2, screen, 60, 90, 300, 70, 3, "Enter byte (hex)");
    sscanf(hex4_a, "%x", &tmp_adr);
    sscanf(hex2, "%x", &tmp_dat);
    prog[(uint16_t) tmp_adr] = (uint8_t) tmp_dat;
    return;
}

void LoadRAM(void)
{
    char filename[40];
    GetText(filename, screen, 30, 80, 700, 70, 40, "Load *.bin file");
    for ( int c = 38 ; filename[c] == ' ' || filename[c] == '\r' ; c-- )
    {
        filename[c] = 0;
    }
    load_ram(filename, prog);
    return;
}

void LoadHEX(void)
{
    char filename[40];
    GetText(filename, screen, 30, 80, 700, 70, 40, "Load *.hex file");
    for ( int c = 38 ; filename[c] == ' ' || filename[c] == '\r' ; c-- )
    {
        filename[c] = 0;
    }
    load_hex(filename, prog);
    return;
}

void LoadSrc(void)
{
    char cmd[256];
    char filename[256];
    char comp_msg[512];
    FILE *msgin;
    int c = 0;

    GetText(filename, screen, 30, 80, 700, 70, 40, "Load and compile source file (*.*)");
    sprintf(cmd, "asm51.exe %s > out.tmp", filename);

    system(cmd);

    msgin = NULL;

    msgin = fopen("out.tmp", "rb");

    if ( msgin == NULL )
    {
        ShowMessage("Can't open the file: out.tmp", screen, 250, 100, 200, 100, "Error!");
    }
    else
    {
        for ( c ; c <= 0xffff ; c++ )
        {
            comp_msg[c] = fgetc(msgin);
            if ( feof(msgin))
            {
                break;
            }
        }
        fclose(msgin);
    }

    ShowMsgEx(comp_msg, screen, 100, 80, 600, 140, "Assembling status", makecol(255, 255, 255));

    for ( c = 512 ; c > 0 ; c-- )
    {
        if ( comp_msg[c] == ',' && comp_msg[c + 2] == '0' )
        {
            change_ext(filename);
            if ( load_hex(filename, prog) == 0 )
            {
                ShowMessage("Compiled file is loaded.", screen, 250, 100, 300, 80, "Message!");
            }
            break;
        }
    }
    return;
}

void EditSrc(void)
{
    code_editor ed1;
    ed1.process();
    return;
}

void ChMaxI(int *maxi)
{
    char dec6_a[7];
    GetText(dec6_a, screen, 50, 80, 400, 70, 7, "Enter number of instructions per frame");
    if ( dec6_a[0] == 32 )
    {
        return;
    }
    sscanf(dec6_a, "%d", maxi);
    return;
}

void ChMemMode(void)
{
    if ( QuestionBox("Do you want to use memory in mixed mode?", screen, 100, 100, 400, 60, "Qusetion?", white))
    {
        ram = EXT_PMEM;
    }
    else
    {
        ram = EXT_RAM;
    }
}

void FillExt(void)
{
    char h4beg[5], h4end[5], h2f[3];
    int from, to, i, ic = 1, fill;
    GetText(h4beg, screen, 50, 80, 400, 70, 5, "From adress... (16bit hex)");
    GetText(h4end, screen, 50, 80, 400, 70, 5, "...to adress (16bit hex)");
    GetText(h2f, screen, 50, 80, 400, 70, 3, "with byte (8bit hex)");
    sscanf(h4beg, "%x", &from);
    sscanf(h4end, "%x", &to);
    sscanf(h2f, "%x", &fill);
    if ( from > to )
    {
        ic = -1;
    }
    for ( i = from ; i != to ; i += ic )
    {
        ram[i] = fill;
    }
    ram[to] = fill;
    return;
}

void main_init(void)
{
    load_code_table(asm51);
    //load_ram ("lekcja3.bin",ram);
    //load_ram ("avt.bin",ram);

    init_8051();
    allegro_init();
    install_timer();
    install_keyboard();
    //install_mouse();
    set_color_depth(8);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 600, 0, 0);
    //set_gfx_mode (GFX_AUTODETECT, 800, 600,0,0);

    set_palette(desktop_palette);
    //show_mouse (screen);
    RGB col;
    col.r = 0;
    col.g = 0;
    col.b = 0;
    set_color(0, &col);
    col.r = 63;
    col.g = 15;
    col.b = 0;
    set_color(1, &col);
    col.r = 20;
    col.g = 0;
    col.b = 0;
    set_color(2, &col);
    col.r = 255;
    col.g = 255;
    col.b = 255;
    set_color(255, &col);
    DATAFILE *dataf;
    dataf = load_datafile("data.dat");
    mainf = (FONT *) dataf[0].dat;
}

int main(void)
{
    main_init();

    white = makecol(255, 255, 255);
    red = makecol(255, 0, 0);
    lred = makecol(255, 64, 0);
    green = makecol(0, 255, 0);
    blue = makecol(0, 0, 255);

    dis_asm disasm(prog, screen);
    regs SFRregs(SFR, screen);
    flags FlagWnd(screen);
    ramv RMon(screen);
    strcpy(SFRregs.reg_label[0], "Delay AVT");
    SFRregs.reg[0] = 0x75;
    strcpy(SFRregs.reg_label[1], "Key AVT  ");
    SFRregs.reg[1] = 0x76;
    strcpy(SFRregs.reg_label[2], "DPH      ");
    SFRregs.reg[2] = 0x83;
    strcpy(SFRregs.reg_label[3], "DPL      ");
    SFRregs.reg[3] = 0x82;
    strcpy(SFRregs.reg_label[4], "PSW      ");
    SFRregs.reg[4] = 0xd0;
    strcpy(SFRregs.reg_label[5], "P0       ");
    SFRregs.reg[5] = 0x80;
    strcpy(SFRregs.reg_label[6], "P1       ");
    SFRregs.reg[6] = 0x90;
    strcpy(SFRregs.reg_label[7], "P2       ");
    SFRregs.reg[7] = 0xa0;
    strcpy(SFRregs.reg_label[8], "P3       ");
    SFRregs.reg[8] = 0xb0;
    strcpy(SFRregs.reg_label[9], "TH1      ");
    SFRregs.reg[9] = 0x8d;
    strcpy(SFRregs.reg_label[10], "TL1      ");
    SFRregs.reg[10] = 0x8b;
    strcpy(SFRregs.reg_label[11], "TH0      ");
    SFRregs.reg[11] = 0x8c;
    strcpy(SFRregs.reg_label[12], "TL0      ");
    SFRregs.reg[12] = 0x8a;
    strcpy(SFRregs.reg_label[13], "IE       ");
    SFRregs.reg[13] = 0xa8;
    strcpy(SFRregs.reg_label[14], "10       ");
    SFRregs.reg[14] = 0x12;
    strcpy(SFRregs.reg_label[15], "9        ");
    SFRregs.reg[15] = 0x11;
    strcpy(SFRregs.reg_label[16], "8        ");
    SFRregs.reg[16] = 0x10;
    strcpy(SFRregs.reg_label[17], "7        ");
    SFRregs.reg[17] = 0x0f;
    strcpy(SFRregs.reg_label[18], "6        ");
    SFRregs.reg[18] = 0x0e;
    strcpy(SFRregs.reg_label[19], "5        ");
    SFRregs.reg[19] = 0x0d;
    strcpy(SFRregs.reg_label[20], "4        ");
    SFRregs.reg[20] = 0x0c;
    strcpy(SFRregs.reg_label[21], "3        ");
    SFRregs.reg[21] = 0x0b;
    strcpy(SFRregs.reg_label[22], "2        ");
    SFRregs.reg[22] = 0x0a;
    strcpy(SFRregs.reg_label[23], "1        ");
    SFRregs.reg[23] = 0x09;
    strcpy(SFRregs.reg_label[24], "0        ");
    SFRregs.reg[24] = 0x08;
    PC = 0x0000;
    uint16_t aadr = 0x0000;
    uint16_t madr = 0x0000;
    bool manual_scrolling = true;
    bool step = true;
    bool into = false;
    bool over = false;
    unsigned long cnt = 0;
    int break_point = -1;
    char break_str[5];
    //load_ram("avt.bin",ram);
    int i = 0, maxi = 13000;
    while ( !( key[KEY_X] && key[KEY_ALT] ))
    {
        rect(screen, 2, 2, 798, 18, white);
        // key shortcuts disp
        rect(screen, 2, 521, 798, 598, white);
        textprintf(screen, mainf, 128, 528, white,
                   "F1     F2     F3     F4     F5     F6     F7     F8     F9     F10    F11    F12");
        textprintf(screen, mainf, 128, 548, white,
                   "Help   ChSFR  ManS   Debug  DView  WrPC   TInto  SOver  SrcEd  FillE  Speed  Reset");
        textprintf(screen, mainf, 128, 568, white,
                   "WrRAM  WrSFR  AutoS  Run    MView         LdBIN  LdHEX  LdSrc  BrkPt  MemMod      ");
        hline(screen, 2, 540, 798, white);
        textprintf(screen, mainf, 6, 548, white, "Function key :");
        hline(screen, 2, 560, 798, white);
        textprintf(screen, mainf, 6, 568, white, "Shift+key    :");
        hline(screen, 2, 580, 798, white);
        textprintf(screen, mainf, 6, 587, white, "Alt+X: Exit  ");
        textprintf(screen, mainf, 6, 7, white, "emu51 %s", VERSION);
        textout(screen, mainf, "AVT-2250 Segment display:", 490, 6, white);
        if ( ram == prog )
        {
            rect(screen, 680, 568, 680 + 6, 575, red);
        }


        cnt++;
        if ( manual_scrolling )
        {
            disasm.draw(aadr);
        }
        else
        {
            disasm.draw(PC);
        }
        disasm.blit_it(0, 20);
        RMon.draw(madr);
        RMon.blit_it(300, 320);
        SFRregs.draw();
        SFRregs.blit_it(400, 20);
        FlagWnd.draw();
        FlagWnd.blit_it(300, 20);

        if ( !step )
        {
            for ( i = 0 ; i < maxi ; i++ )
            {
                if ( PC == break_point )
                {
                    step = true;
                    break;
                }
                asm51[prog[PC]].process();
            }
        }
        else
        {
            if ( over )
            {
                asm51[prog[PC]].process();
                over = false;
            }
            if ( into )
            {
                asm51[prog[PC]].process();
                into = false;
            }
        }

        for ( i = 0 ; i <= 7 ; i++ )
        {
            // for compatibility with avt2250
            draw_SEG_digit(screen, 700 + 8 * i, 6, lred, 4, SFR[i + 0x78]);
        }
        if ( key[KEY_LCONTROL] )
        {
            if ( key[KEY_0_PAD] )
            {
                SFR[0x76] = 0x30;
            }
            if ( key[KEY_1_PAD] )
            {
                SFR[0x76] = 0x31;
            }
            if ( key[KEY_2_PAD] )
            {
                SFR[0x76] = 0x32;
            }
            if ( key[KEY_3_PAD] )
            {
                SFR[0x76] = 0x33;
            }
            if ( key[KEY_4_PAD] )
            {
                SFR[0x76] = 0x34;
            }
            if ( key[KEY_5_PAD] )
            {
                SFR[0x76] = 0x35;
            }
            if ( key[KEY_6_PAD] )
            {
                SFR[0x76] = 0x36;
            }
            if ( key[KEY_7_PAD] )
            {
                SFR[0x76] = 0x37;
            }
            if ( key[KEY_8_PAD] )
            {
                SFR[0x76] = 0x38;
            }
            if ( key[KEY_9_PAD] )
            {
                SFR[0x76] = 0x39;
            }
            if ( key[KEY_INSERT] )
            {
                SFR[0x76] = 0x41;
            }
            if ( key[KEY_HOME] )
            {
                SFR[0x76] = 0x42;
            }
            if ( key[KEY_PGUP] )
            {
                SFR[0x76] = 0x43;
            }
            if ( key[KEY_DEL] )
            {
                SFR[0x76] = 0x44;
            }
            if ( key[KEY_END] )
            {
                SFR[0x76] = 0x45;
            }
            if ( key[KEY_PGDN] )
            {
                SFR[0x76] = 0x46;
            }
            if ( key[KEY_ENTER_PAD] )
            {
                SFR[0x76] = 0x0d;
            }
        }
        else
        {
            if ( key[KEY_UP] )
            {
                aadr -= 3;
            }
            if ( key[KEY_DOWN] )
            {
                aadr += get_lenght(prog[aadr]);
                aadr += get_lenght(prog[aadr]);
            }
            if ( key[KEY_PGUP] && !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ))
            {
                madr -= 16;
            }
            if ( key[KEY_PGDN] && !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ))
            {
                madr += 16;
            }

            if ( key[KEY_PGUP] && ( key[KEY_LSHIFT] || key[KEY_RSHIFT] ))
            {
                madr -= 1;
            }
            if ( key[KEY_PGDN] && ( key[KEY_LSHIFT] || key[KEY_RSHIFT] ))
            {
                madr += 1;
            }

            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F2] )
            {
                ChangeMonitedSFRRegister(&SFRregs);
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F2] )
            {
                ChangeRegValue_SFR();
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F1] )
            {
                ChangeRegValue_RAM();
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F3] )
            {
                manual_scrolling = true;
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F3] )
            {
                manual_scrolling = false;
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F4] )
            {
                step = true;
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F4] )
            {
                step = false;
            }
            if ( step )
            {
                if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F7] )
                {
                    into = true;
                }
                while ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F7] )
                {
                }
                if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F8] )
                {
                    over = true;
                }
                while ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F8] )
                {
                }
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F5] )
            {
                aadr = ViewAt();
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F5] )
            {
                madr = ViewAt();
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F6] )
            {
                ChangePC();
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F7] )
            {
                LoadRAM();
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F8] )
            {
                LoadHEX();
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F9] )
            {
                LoadSrc();
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F9] )
            {
                EditSrc();
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F12] )
            {
                reset51();
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F11] )
            {
                ChMaxI(&maxi);
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F11] )
            {
                ChMemMode();
            }
            if ( !( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F10] )
            {
                FillExt();
            }
            if (( key[KEY_LSHIFT] || key[KEY_RSHIFT] ) && key[KEY_F10] )
            {
                GetText(break_str, screen, 50, 80, 400, 70, 5, "Enter break piont adress");
                sscanf(break_str, "%x", &break_point);
            }
        }
    }
    return 0;
}
//END_OF_MAIN();

