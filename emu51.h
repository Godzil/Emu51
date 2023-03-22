#define bit0 0x01
#define bit1 0x02
#define bit2 0x04
#define bit3 0x08
#define bit4 0x10
#define bit5 0x20
#define bit6 0x40
#define bit7 0x80

typedef unsigned char BYTE;
typedef unsigned short int WORD;

BYTE SFR[0x100];                   // internal memory and sfr area 256 bytes
BYTE EXT_RAM[0x10000];             // external memory 64kB
BYTE *Acc = &SFR[0xe0];            // 8-bit accumlator
WORD *DPTR = (WORD *) &SFR[0x82];  // 16-bit register
BYTE *DPH = &SFR[0x83];            // high byte of DPTR
BYTE *DPL = &SFR[0x82];            // low byte of DPTR
BYTE *B = &SFR[0xf0];              // B register
BYTE *SP = &SFR[0x81];             // Stack Pointer
BYTE *PSW = &SFR[0xd0];            // Program Status Word
BYTE *P0 = &SFR[0x80];             // Port one
BYTE *P1 = &SFR[0x90];             //
BYTE *P2 = &SFR[0xa0];             //
BYTE *P3 = &SFR[0xb0];             //
BYTE *SBUF = &SFR[0x99];           //
BYTE *IE = &SFR[0xa8];             //
BYTE *SCON = &SFR[0x98];           //
BYTE *TH1 = &SFR[0x8d];            //
BYTE *TH0 = &SFR[0x8c];            //
BYTE *TL1 = &SFR[0x8b];            //
BYTE *TL0 = &SFR[0x8a];            //
BYTE *TMOD = &SFR[0x89];           //
BYTE *TCON = &SFR[0x88];           //
BYTE *PCON = &SFR[0x87];           //
BYTE *R = &SFR[0x00];              //
WORD PC = 0x0000;                  //

// initialize emulator
void init_8051(void)
{
    *Acc = 0;
    *DPTR = 0;
    *P1 = 0xff;
    PC = 0x0000;
    *SP = 0x07;
}

BYTE check_C(void)
{
    if ( *PSW & bit7 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

BYTE check_AC(void)
{
    if ( *PSW & bit6 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

BYTE check_OV(void)
{
    if ( *PSW & bit2 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void set_C(void)
{
    *PSW = *PSW | bit7;
}

void set_AC(void)
{
    *PSW = *PSW | bit6;
}

void set_OV(void)
{
    *PSW = *PSW | bit2;
}

void clr_C(void)
{
    *PSW = *PSW & 0x7f;
}

void clr_AC(void)
{
    *PSW = *PSW & 0xbf;
}

void clr_OV(void)
{
    *PSW = *PSW & 0xfb;
}

BYTE check_P(void)
{
    if ( *PSW & bit0 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void set_P(void)
{
    *PSW = *PSW | bit0;
}

void clr_P(void)
{
    *PSW = *PSW & 0xfe;
}

void setB(BYTE bit)
{
    BYTE tmpB;
    BYTE tmpbit;
    tmpbit = bit & 0xf8;
    tmpB = bit & 0x07;
    if ( bit >= 128 )
    {
        // bit set in SFR area
        SFR[tmpB] = SFR[tmpB] | ( 1 << tmpbit );
    }
    else
    {
        // bit set in int. RAM
        tmpB = ( tmpB >> 3 ) + 32;
        SFR[tmpB] = SFR[tmpB] | ( 1 << tmpbit );
    }
}

void clrB(BYTE bit)
{
    BYTE tmpB;
    BYTE tmpbit;
    tmpbit = bit & 0xf8;
    tmpB = bit & 0x07;
    if ( bit >= 128 )
    {
        // bit clear in SFR area
        SFR[tmpB] = SFR[tmpB] & ~( 1 << tmpbit );
    }
    else
    {
        // bit clear in int. RAM
        tmpB = ( tmpB >> 3 ) + 32;
        SFR[tmpB] = SFR[tmpB] & ~( 1 << tmpbit );
    }
}

BYTE checkB(BYTE bit)
{
    BYTE tmpB;
    BYTE tmpbit;
    tmpbit = bit & 0xf8;
    tmpB = bit & 0x07;
    if ( bit >= 128 )
    {
        // bit chack in SFR area
        if ( SFR[tmpB] & ~( 1 << tmpbit ))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        // bit check in int. RAM
        tmpB = ( tmpB >> 3 ) + 32;
        if ( SFR[tmpB] & ~( 1 << tmpbit ))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

BYTE check_RS0(void)
{
    if ( *PSW & bit3 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void set_RS0(void)
{
    *PSW = *PSW | bit3;
}

void clr_RS0(void)
{
    *PSW = *PSW & 0xf7;
}

BYTE check_RS1(void)
{
    if ( *PSW & bit4 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void set_RS1(void)
{
    *PSW = *PSW | bit4;
}

void clr_RS1(void)
{
    *PSW = *PSW & 0xef;
}
