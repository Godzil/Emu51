/*******************************************************************************
 * Emu51
 * code_51.cpp:
 * Created by mlt on 22/03/23.
 ******************************************************************************/

#include <emu51.h>
#include <code_51.h>

#include <stdio.h>

unsigned long c_time = 0;
code_51 asm51[256];

// 1 or 0
uint8_t check_C(void)
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

// 1 or 0
uint8_t check_AC(void)
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

// 1 or 0
uint8_t check_OV(void)
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

// C <- 1
void set_C(void)
{
    *PSW = *PSW | bit7;
}

// AC <- 1
void set_AC(void)
{
    *PSW = *PSW | bit6;
}

// OV <- 1
void set_OV(void)
{
    *PSW = *PSW | bit2;
}

// C <- 0
void clr_C(void)
{
    *PSW = *PSW & 0x7f;
}

// AC <- 0
void clr_AC(void)
{
    *PSW = *PSW & 0xbf;
}

// OV <- 0
void clr_OV(void)
{
    *PSW = *PSW & 0xfb;
}

// 1 or 0
uint8_t check_P(void)
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

// P <- 1
void set_P(void)
{
    *PSW = *PSW | bit0;
}

// P <- 0
void clr_P(void)
{
    *PSW = *PSW & 0xfe;
}

// (bit) <- 1
void set_Bit(uint8_t bit)
{
    uint8_t tmpB;
    uint8_t tmpbit;
    tmpbit = bit & 0x07;
    tmpB = bit & 0xf8;
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

// (bit) <- 0
void clr_Bit(uint8_t bit)
{
    uint8_t tmpB;
    uint8_t tmpbit;
    tmpbit = bit & 0x07;
    tmpB = bit & 0xf8;
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

// returns (bit) 1 or 0
uint8_t check_Bit(uint8_t bit)
{
    uint8_t tmpB;
    uint8_t tmpbit;
    tmpbit = bit & 0x07;
    tmpB = bit & 0xf8;
    if ( bit >= 128 )
    {
        // bit check in SFR area
        if ( SFR[tmpB] & 1 << tmpbit )
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
        if ( SFR[tmpB] & 1 << tmpbit )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

uint8_t check_RS0(void)
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

uint8_t check_RS1(void)
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

void _add(uint8_t &a, uint8_t &b)
{
    int tmp1, tmp2;
    (( 0xf & a ) + ( 0xf & b )) > 0xf ? set_AC() : clr_AC();
    tmp1 = (( 0x7f & a ) + ( 0x7f & b )) > 0x7f ? 1 : 0;
    ( tmp2 = a + b ) > 0xff ? ( set_C(), tmp1 ? clr_OV() : set_OV()) : ( clr_C(), tmp1 ? set_OV() : clr_OV());
    a = (uint8_t) tmp2 & 0xff;
}

void _addc(uint8_t &a, uint8_t &b)
{
    int tmp1, tmp2, c;
    c = check_C();
    (( 0xf & a ) + ( 0xf & b + c )) > 0xf ? set_AC() : clr_AC();
    tmp1 = (( 0x7f & a ) + ( 0x7f & b ) + c ) > 0x7f ? 1 : 0;
    ( tmp2 = a + b + c ) > 0xff ? ( set_C(), tmp1 ? clr_OV() : set_OV()) : ( clr_C(), tmp1 ? set_OV() : clr_OV());
    a = (uint8_t) tmp2 & 0xff;
}

void _subb(uint8_t &a, uint8_t &b)
{
    int tmp1, tmp2, c;
    c = check_C();
    ((char) ( 0xf & a ) - (char) ( 0xf & b ) - c ) < 0 ? set_AC() : clr_AC();
    tmp1 = ((char) ( 0x7f & a ) - (char) ( 0x7f & b ) - c ) < 0 ? 1 : 0;
    (char) ( tmp2 = a - b - c ) < 0 ? ( set_C(), tmp1 ? clr_OV() : set_OV()) : ( clr_C(), tmp1 ? set_OV() : clr_OV());
    a = (uint8_t) tmp2 & 0xff;
}


void code_51::make_ds(uint16_t ram_pl)
{
    int c, i;
    char tmp[3], tmpW[5];
    display_string[0] = mnem[0];
    display_string[1] = mnem[1];
    display_string[2] = ( mnem[2] >= 0x61 && mnem[2] <= 0x7a ) ? mnem[2] : (char) 32;
    display_string[3] = ( mnem[3] >= 0x61 && mnem[3] <= 0x7a ) ? mnem[3] : (char) 32;
    display_string[4] = ( mnem[4] >= 0x61 && mnem[4] <= 0x7a ) ? mnem[4] : (char) 32;
    display_string[5] = (char) 32;

    for ( c = 0 ; c <= 10 ; c++ )
    {
        if ( datas[c] != '%' )
        {
            if ( datas[c] == 'O' && datas[c + 1] == '%' )
            {
                if ( datas[c + 2] == '1' )
                {
                    sprintf(tmpW, "%4x", (signed char) prog[ram_pl + 1] + ram_pl + 2);
                    for ( i = 0 ; i < 4 ; i++ )
                    {
                        display_string[c + 6 + i] = ( tmpW[i] == ' ' ) ? '0' : tmpW[i];
                    }
                }
                if ( datas[c + 2] == '2' )
                {
                    sprintf(tmpW, "%4x", (signed char) prog[ram_pl + 2] + ram_pl + 3);
                    for ( i = 0 ; i < 4 ; i++ )
                    {
                        display_string[c + 6 + i] = ( tmpW[i] == ' ' ) ? '0' : tmpW[i];
                    }
                }
                c += 3;

            }
            else
            {
                display_string[c + 6] = datas[c];
            }
        }
        else
        {
            if ( datas[c + 1] == '1' )
            {
                sprintf(tmp, "%2x", prog[ram_pl + 1]);
                for ( i = 0 ; i < 2 ; i++ )
                {
                    display_string[c + 6 + i] = tmp[i] == ' ' ? '0' : tmp[i];
                }
            }
            if ( datas[c + 1] == '2' )
            {
                sprintf(tmp, "%2x", prog[ram_pl + 2]);
                for ( i = 0 ; i < 2 ; i++ )
                {
                    display_string[c + 6 + i] = tmp[i] == ' ' ? '0' : tmp[i];
                }
            }
            c++;
        }
    }
}

void code_51::process(void)
{
    uint8_t tmpB;
    uint16_t tmpW;
    switch ( code )
    {
    case 0x00:  //nop                      00
        PC++;
        break;
    case 0x74:  // mov a,#(byte)           74 xx
        PC++;
        *Acc = prog[PC];
        PC++;
        break;
    case 0x75:  // mov (adress),#(byte)    75 xx yy
        PC++;
        SFR[prog[PC]] = prog[PC + 1];
        PC += 2;
        break;
    case 0x02:  // ljmp 16bit_adres        02 xx yy
        PC++;
        PC = (uint16_t) prog[PC] << 8 | prog[PC + 1];
        break;
    case 0x80:  // sjmp offset             80 oo
        PC++;
        PC += (signed char) prog[PC] + 1;
        break;
    case 0x85:  // mov (adress1),(ardess2) 85 yy xx
        PC++;
        SFR[prog[PC + 1]] = SFR[prog[PC]];
        PC += 2;
        break;
    case 0xe5:  // mov A,(adress)
        PC++;
        *Acc = SFR[prog[PC]];
        PC++;
        break;
    case 0x92:  // mov (bit),C
        PC++;
        if ( check_C())
        {
            set_Bit(prog[PC]);
        }
        else
        {
            clr_Bit(prog[PC]);
        }
        PC++;
        break;
    case 0x04:  // inc a
        ( *Acc )++;
        PC++;
        break;
    case 0x05:  // inc (adress)
        PC++;
        SFR[prog[PC]]++;
        PC++;
        break;
    case 0xa3:  // inc DPTR
        *DPTR = *DPTR + 1;
        PC++;
        break;
    case 0x06: //inc @R0
        SFR[R[0]]++;
        PC++;
        break;
    case 0x07: //inc @R1
        SFR[R[1]]++;
        PC++;
        break;
    case 0x08: //inc R0
        R[0]++;
        PC++;
        break;
    case 0x09: //inc R1
        R[1]++;
        PC++;
        break;
    case 0x0a: //inc R2
        R[2]++;
        PC++;
        break;
    case 0x0b: //inc R3
        R[3]++;
        PC++;
        break;
    case 0x0c: //inc R4
        R[4]++;
        PC++;
        break;
    case 0x0d: //inc R5
        R[5]++;
        PC++;
        break;
    case 0x0e: //inc R6
        R[6]++;
        PC++;
        break;
    case 0x0f: //inc R7
        R[7]++;
        PC++;
        break;
    case 0x14: //dec A
        *Acc = *Acc - 1;
        PC++;
        break;
    case 0x15: //dec (adress)
        PC++;
        SFR[prog[PC]]--;
        PC++;
        break;
    case 0x16: //dec @R0
        SFR[R[0]]--;
        PC++;
        break;
    case 0x17: //dec @R1
        SFR[R[1]]--;
        PC++;
        break;
    case 0x18: // dec R0
        R[0]--;
        PC++;
        break;
    case 0x19: // dec R1
        R[1]--;
        PC++;
        break;
    case 0x1a: // dec R2
        R[2]--;
        PC++;
        break;
    case 0x1b: // dec R3
        R[3]--;
        PC++;
        break;
    case 0x1c: // dec R4
        R[4]--;
        PC++;
        break;
    case 0x1d: // dec R5
        R[5]--;
        PC++;
        break;
    case 0x1e: // dec R6
        R[6]--;
        PC++;
        break;
    case 0x1f: // dec R7
        R[7]--;
        PC++;
        break;
    case 0x84:  // div AB
        clr_C();
        if ( *B != 0 )
        {
            tmpB = *Acc;
            *Acc = tmpB / *B;
            *B = tmpB % *B;
        }
        else
        {
            set_OV();
        }
        PC++;
        break;
    case 0xa4:  // mul AB
        tmpW = (uint16_t) *Acc * (uint16_t) *B;
        if ( tmpW > 255 )
        {
            set_OV();
        }
        else
        {
            clr_OV();
        }
        *B = (uint8_t) tmpW >> 8;
        *Acc = (uint8_t) ( tmpW & 0xff );
        PC++;
        break;
    case 0x23:  // rl A
        tmpB = *Acc;
        *Acc = tmpB << 1 | tmpB >> 7;
        PC++;
        break;
    case 0x03:  // rr A
        tmpB = *Acc;
        *Acc = tmpB >> 1 | tmpB << 7;
        PC++;
        break;
    case 0x33:  // rlc A
        tmpB = *Acc;
        *Acc = tmpB << 1 | check_C();
        if ( tmpB & 0x80 )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        PC++;
        break;
    case 0x13:  // rrc A
        tmpB = *Acc;
        *Acc = tmpB >> 1 | check_C() << 7;
        if ( tmpB & 0x01 )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        PC++;
        break;
    case 0x90:  // mov DPTR,#(16-bit data)
        PC++;
        *DPH = prog[PC];
        PC++;
        *DPL = prog[PC];
        PC++;
        break;
    case 0xf5:  // mov (adress),A
        PC++;
        SFR[prog[PC]] = *Acc;
        PC++;
        break;
        // ADD (full functional)
    case 0x24:  // add A,#(data)
        PC++;
        _add(*Acc, prog[PC]);
        PC++;
        break;
    case 0x25:  // add A,(adres)
        PC++;
        _add(*Acc, SFR[prog[PC]]);
        PC++;
        break;
    case 0x26:  // add A,@R0
        _add(*Acc, SFR[R[0]]);
        PC++;
        break;
    case 0x27:  // add A,@R1
        _add(*Acc, SFR[R[1]]);
        PC++;
        break;
    case 0x28: //add A,R0
        _add(*Acc, R[0]);
        PC++;
        break;
    case 0x29: //add A,R1
        _add(*Acc, R[1]);
        PC++;
        break;
    case 0x2a: //add A,R2
        _add(*Acc, R[2]);
        PC++;
        break;
    case 0x2b: //add A,R3
        _add(*Acc, R[3]);
        PC++;
        break;
    case 0x2c: //add A,R4
        _add(*Acc, R[4]);
        PC++;
        break;
    case 0x2d: //add A,R5
        _add(*Acc, R[5]);
        PC++;
        break;
    case 0x2e: //add A,R6
        _add(*Acc, R[6]);
        PC++;
        break;
    case 0x2f: //add A,R7
        _add(*Acc, R[7]);
        PC++;
        break;
        // ADDC
    case 0x34:  // addc A,#(data)
        PC++;
        _addc(*Acc, prog[PC]);
        PC++;
        break;
    case 0x35:  // addc A,(adres)
        PC++;
        _addc(*Acc, SFR[prog[PC]]);
        PC++;
        break;
    case 0x36:  // addc A,@R0
        _addc(*Acc, SFR[R[0]]);
        PC++;
        break;
    case 0x37:  // addc A,@R1
        _addc(*Acc, SFR[R[1]]);
        PC++;
        break;
    case 0x38: //addc A,R0
        _addc(*Acc, R[0]);
        PC++;
        break;
    case 0x39: //addc A,R1
        _addc(*Acc, R[1]);
        PC++;
        break;
    case 0x3a: //addc A,R2
        _addc(*Acc, R[2]);
        PC++;
        break;
    case 0x3b: //addc A,R3
        _addc(*Acc, R[3]);
        PC++;
        break;
    case 0x3c: //addc A,R4
        _addc(*Acc, R[4]);
        PC++;
        break;
    case 0x3d: //addc A,R5
        _addc(*Acc, R[5]);
        PC++;
        break;
    case 0x3e: //addc A,R6
        _addc(*Acc, R[6]);
        PC++;
        break;
    case 0x3f: //addc A,R7
        _addc(*Acc, R[7]);
        PC++;
        break;
    case 0xe0:  // movx A,@DPTR
        *Acc = ram[*DPTR];
        PC++;
        break;
    case 0xf0:  // movx @DPTR,A
        ram[*DPTR] = *Acc;
        PC++;
        break;
    case 0xc0:  // push (adress)
        *SP = *SP + 1;
        PC++;
        SFR[*SP] = SFR[prog[PC]];
        PC++;
        break;
    case 0xd0:  // pop (adress)
        PC++;
        SFR[prog[PC]] = SFR[*SP];
        *SP = *SP - 1;
        PC++;
        break;
    case 0xc3:  // clr C
        clr_C();
        PC++;
        break;
    case 0xd3:  // setb C
        set_C();
        PC++;
        break;
    case 0xd2:  // setb (bit)
        PC++;
        set_Bit(prog[PC]);
        PC++;
        break;
    case 0xc2:  // clr (bit)
        PC++;
        clr_Bit(prog[PC]);
        PC++;
        break;
    case 0x62:  // xrl (adress),A
        PC++;
        SFR[prog[PC]] = SFR[prog[PC]] ^ ( *Acc );
        PC++;
        break;
    case 0x63:  // xrl (adress),#(data)
        PC++;
        SFR[prog[PC]] ^= prog[PC + 1];
        PC += 2;
        break;
    case 0x64:  // xrl A,#(data)
        PC++;
        *Acc = *Acc ^ prog[PC];
        PC++;
        break;
    case 0x65:  // xrl A,(adress)
        PC++;
        *Acc = *Acc ^ SFR[prog[PC]];
        PC++;
        break;
    case 0x66:  // xrl A,@R0
        *Acc = *Acc ^ SFR[R[0]];
        PC++;
        break;
    case 0x67:  // xrl A,@R1
        *Acc = *Acc ^ SFR[R[1]];
        PC++;
        break;
    case 0x68:  // xrl A,R0
        *Acc = ( *Acc ) ^ R[0];
        PC++;
        break;
    case 0x69:  // xrl A,R1
        *Acc = ( *Acc ) ^ R[1];
        PC++;
        break;
    case 0x6a:  // xrl A,R2
        *Acc = ( *Acc ) ^ R[2];
        PC++;
        break;
    case 0x6b:  // xrl A,R3
        *Acc = ( *Acc ) ^ R[3];
        PC++;
        break;
    case 0x6c:  // xrl A,R4
        *Acc = ( *Acc ) ^ R[4];
        PC++;
        break;
    case 0x6d:  // xrl A,R5
        *Acc = ( *Acc ) ^ R[5];
        PC++;
        break;
    case 0x6e:  // xrl A,R6
        *Acc = ( *Acc ) ^ R[6];
        PC++;
        break;
    case 0x6f:  // xrl A,R7
        *Acc = ( *Acc ) ^ R[7];
        PC++;
        break;
    case 0xc5:  // xch A,(adress)
        PC++;
        tmpB = *Acc;
        *Acc = SFR[prog[PC]];
        SFR[prog[PC]] = tmpB;
        PC++;
        break;
    case 0xc6:  // xch A,@R0
        tmpB = *Acc;
        *Acc = SFR[R[0]];
        SFR[R[0]] = tmpB;
        PC++;
        break;
    case 0xc7:  // xch A,@R1
        tmpB = *Acc;
        *Acc = SFR[R[1]];
        SFR[R[1]] = tmpB;
        PC++;
        break;
    case 0xc8:  // xch A,R0
        tmpB = *Acc;
        *Acc = R[0];
        R[0] = tmpB;
        PC++;
        break;
    case 0xc9:  // xch A,R1
        tmpB = *Acc;
        *Acc = R[1];
        R[1] = tmpB;
        PC++;
        break;
    case 0xca:  // xch A,R2
        tmpB = *Acc;
        *Acc = R[2];
        R[2] = tmpB;
        PC++;
        break;
    case 0xcb:  // xch A,R3
        tmpB = *Acc;
        *Acc = R[3];
        R[3] = tmpB;
        PC++;
        break;
    case 0xcc:  // xch A,R4
        tmpB = *Acc;
        *Acc = R[4];
        R[4] = tmpB;
        PC++;
        break;
    case 0xcd:  // xch A,R5
        tmpB = *Acc;
        *Acc = R[5];
        R[5] = tmpB;
        PC++;
        break;
    case 0xce:  // xch A,R6
        tmpB = *Acc;
        *Acc = R[6];
        R[6] = tmpB;
        PC++;
        break;
    case 0xcf:  // xch A,R7
        tmpB = *Acc;
        *Acc = R[7];
        R[7] = tmpB;
        PC++;
        break;
    case 0x12:  // lcall 16bit_adres        02 xx yy
        tmpW = PC + 3;
        PC++;
        PC = (uint16_t) prog[PC] << 8 | prog[PC + 1];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0x22: // ret
        PC = (uint16_t) SFR[*SP] << 8 | (uint16_t) SFR[*SP - 1];
        *SP = *SP - 2;
        break;
    case 0x70: // jnz (offset)
        PC++;
        if ( *Acc != 0 )
        {
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC++;
        }
        break;
    case 0x60: // jz (offset)
        PC++;
        if ( *Acc == 0 )
        {
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC++;
        }
        break;
    case 0x40:  // jc (offset)
        PC++;
        if ( check_C())
        {
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC++;
        }
        break;
    case 0x50:  // jnc (offset)
        PC++;
        if ( !check_C())
        {
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC++;
        }
        break;
    case 0xc4:  // swap A
        tmpB = *Acc;
        tmpB = ( tmpB << 4 ) & 0xf0;
        *Acc = ( *Acc >> 4 ) & 0x0f;
        *Acc = *Acc | tmpB;
        PC++;
        break;
    case 0x93:  // movc A,@DPTR+A
        *Acc = ram[*Acc + *DPTR];
        PC++;
        break;
    case 0x83:  // movc A,@PC+A
        *Acc = ram[*Acc + PC];
        PC++;
        break;
    case 0xe6:  // mov A,@R0
        *Acc = SFR[R[0]];
        PC++;
        break;
    case 0xe7:  // mov A,@R1
        *Acc = SFR[R[1]];
        PC++;
        break;
    case 0xf6:  // mov @R0,A
        SFR[R[0]] = *Acc;
        PC++;
        break;
    case 0xf7:  // mov @R1,a
        SFR[R[1]] = *Acc;
        PC++;
        break;
    case 0x78:  // mov R0,#(data)
        PC++;
        R[0] = prog[PC];
        PC++;
        break;
    case 0x79: // mov R1,#(data)
        PC++;
        R[1] = prog[PC];
        PC++;
        break;
    case 0x7a:  // mov R2,#(data)
        PC++;
        R[2] = prog[PC];
        PC++;
        break;
    case 0x7b:  // mov R3,#(data)
        PC++;
        R[3] = prog[PC];
        PC++;
        break;
    case 0x7c:   // mov R4,#(data)
        PC++;
        R[4] = prog[PC];
        PC++;
        break;
    case 0x7d:  // mov R5,#(data)
        PC++;
        R[5] = prog[PC];
        PC++;
        break;
    case 0x7e:  // mov R6,#(data)
        PC++;
        R[6] = prog[PC];
        PC++;
        break;
    case 0x7f:   // mov R7,#(data)
        PC++;
        R[7] = prog[PC];
        PC++;
        break;
    case 0xa8:   // mov R0,(adress)
        PC++;
        R[0] = SFR[prog[PC]];
        PC++;
        break;
    case 0xa9:  // mov R1,(adress)
        PC++;
        R[1] = SFR[prog[PC]];
        PC++;
        break;
    case 0xaa:  // mov R2,(adress)
        PC++;
        R[2] = SFR[prog[PC]];
        PC++;
        break;
    case 0xab:  // mov R3,(adress)
        PC++;
        R[3] = SFR[prog[PC]];
        PC++;
        break;
    case 0xac:  // mov R4,(adress)
        PC++;
        R[4] = SFR[prog[PC]];
        PC++;
        break;
    case 0xad:  // mov R5,(adress)
        PC++;
        R[5] = SFR[prog[PC]];
        PC++;
        break;
    case 0xae:  // mov R6,(adress)
        PC++;
        R[6] = SFR[prog[PC]];
        PC++;
        break;
    case 0xaf: // mov R7,(adress)
        PC++;
        R[7] = SFR[prog[PC]];
        PC++;
        break;
    case 0x88:   // mov (adress),R0
        PC++;
        SFR[prog[PC]] = R[0];
        PC++;
        break;
    case 0x89:   // mov (adress),R1
        PC++;
        SFR[prog[PC]] = R[1];
        PC++;
        break;
    case 0x8a:   // mov (adress),R2
        PC++;
        SFR[prog[PC]] = R[2];
        PC++;
        break;
    case 0x8b:   // mov (adress),R3
        PC++;
        SFR[prog[PC]] = R[3];
        PC++;
        break;
    case 0x8c:   // mov (adress),R4
        PC++;
        SFR[prog[PC]] = R[4];
        PC++;
        break;
    case 0x8d:   // mov (adress),R5
        PC++;
        SFR[prog[PC]] = R[5];
        PC++;
        break;
    case 0x8e:   // mov (adress),R6
        PC++;
        SFR[prog[PC]] = R[6];
        PC++;
        break;
    case 0x8f:   // mov (adress),R7
        PC++;
        SFR[prog[PC]] = R[7];
        PC++;
        break;
    case 0xf8:  // mov R0,A
        PC++;
        R[0] = *Acc;
        break;
    case 0xf9:  // mov R1,A
        PC++;
        R[1] = *Acc;
        break;
    case 0xfa:  // mov R2,A
        PC++;
        R[2] = *Acc;
        break;
    case 0xfb:  // mov R3,A
        PC++;
        R[3] = *Acc;
        break;
    case 0xfc:  // mov R4,A
        PC++;
        R[4] = *Acc;
        break;
    case 0xfd:  // mov R5,A
        PC++;
        R[5] = *Acc;
        break;
    case 0xfe:  // mov R6,A
        PC++;
        R[6] = *Acc;
        break;
    case 0xff:  // mov R7,A
        PC++;
        R[7] = *Acc;
        break;
    case 0xe8:  // mov A,R0
        PC++;
        *Acc = R[0];
        break;
    case 0xe9:  // mov A,R1
        PC++;
        *Acc = R[1];
        break;
    case 0xea:  // mov A,R2
        PC++;
        *Acc = R[2];
        break;
    case 0xeb:  // mov A,R3
        PC++;
        *Acc = R[3];
        break;
    case 0xec:  // mov A,R4
        PC++;
        *Acc = R[4];
        break;
    case 0xed:  // mov A,R5
        PC++;
        *Acc = R[5];
        break;
    case 0xee:  // mov A,R6
        PC++;
        *Acc = R[6];
        break;
    case 0xef:  // mov A,R7
        PC++;
        *Acc = R[7];
        break;
    case 0xe4: //clr A
        PC++;
        *Acc = 0;
        break;
    case 0x01: // ajmp 00xx
        PC++;
        PC = (uint16_t) prog[PC];
        break;
    case 0x21: // ajmp 01xx
        PC++;
        PC = 0x0100 | (uint16_t) prog[PC];
        break;
    case 0x41: // ajmp 02xx
        PC++;
        PC = 0x0200 | (uint16_t) prog[PC];
        break;
    case 0x61: // ajmp 03xx
        PC++;
        PC = 0x0300 | (uint16_t) prog[PC];
        break;
    case 0x81: // ajmp 04xx
        PC++;
        PC = 0x0400 | (uint16_t) prog[PC];
        break;
    case 0xa1: // ajmp 05xx
        PC++;
        PC = 0x0500 | (uint16_t) prog[PC];
        break;
    case 0xc1: // ajmp 06xx
        PC++;
        PC = 0x0600 | (uint16_t) prog[PC];
        break;
    case 0xe1: // ajmp 07xx
        PC++;
        PC = 0x0700 | (uint16_t) prog[PC];
        break;
    case 0x11: // acall 00xx
        tmpW = PC + 2;
        PC++;
        PC = (uint16_t) prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0x31: // acall 01xx
        tmpW = PC + 2;
        PC++;
        PC = 0x0100 | prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0x51: // acall 02xx
        tmpW = PC + 2;
        PC++;
        PC = 0x0200 | prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0x71: // acall 03xx
        tmpW = PC + 2;
        PC++;
        PC = 0x0300 | prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0x91: // acall 04xx
        tmpW = PC + 2;
        PC++;
        PC = 0x0400 | prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0xb1: // acall 05xx
        tmpW = PC + 2;
        PC++;
        PC = 0x0500 | prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0xd1: // acall 06xx
        tmpW = PC + 2;
        PC++;
        PC = 0x0600 | prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0xf1: // acall 07xx
        tmpW = PC + 2;
        PC++;
        PC = 0x0700 | prog[PC];
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW & 0x00ff );
        *SP = *SP + 1;
        SFR[*SP] = (uint8_t) ( tmpW >> 8 );
        break;
    case 0xd8: // djnz R0,(offset)
        PC++;
        R[0]--;
        if ( R[0] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xd9: // djnz R1,(offset)
        PC++;
        R[1]--;
        if ( R[1] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xda: // djnz R2,(offset)
        PC++;
        R[2]--;
        if ( R[2] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xdb: // djnz R3,(offset)
        PC++;
        R[3]--;
        if ( R[3] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xdc: // djnz R4,(offset)
        PC++;
        R[4]--;
        if ( R[4] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xdd: // djnz R5,(offset)
        PC++;
        R[5]--;
        if ( R[5] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xde: // djnz R6,(offset)
        PC++;
        R[6]--;
        if ( R[6] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xdf: // djnz R7,(offset)
        PC++;
        R[7]--;
        if ( R[7] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0xd5: // djnz (adress),(offset)
        PC++;
        tmpB = prog[PC];
        PC++;
        SFR[tmpB]--;
        if ( SFR[tmpB] != 0 )
        {
            PC += (signed char) prog[PC];
        }
        PC++;
        break;
    case 0x43: // orl (adress),#(data)
        PC++;
        SFR[prog[PC]] = SFR[prog[PC]] | prog[PC + 1];
        PC += 2;
        break;
    case 0x44: // orl A,#(data)
        PC++;
        *Acc = prog[PC] | *Acc;
        PC++;
        break;
    case 0x45: // orl A,(adress)
        PC++;
        *Acc = SFR[prog[PC]] | *Acc;
        PC++;
        break;
    case 0x42: // orl (adress),A
        PC++;
        SFR[prog[PC]] = SFR[prog[PC]] | *Acc;
        PC++;
        break;
    case 0x48: // orl A,R0
        *Acc = *Acc | R[0];
        PC++;
        break;
    case 0x49: // orl A,R1
        *Acc = *Acc | R[1];
        PC++;
        break;
    case 0x4a: // orl A,R2
        *Acc = *Acc | R[2];
        PC++;
        break;
    case 0x4b: // orl A,R3
        *Acc = *Acc | R[3];
        PC++;
        break;
    case 0x4c: // orl A,R4
        *Acc = *Acc | R[4];
        PC++;
        break;
    case 0x4d: // orl A,R5
        *Acc = *Acc | R[5];
        PC++;
        break;
    case 0x4e: // orl A,R6
        *Acc = *Acc | R[6];
        PC++;
        break;
    case 0x4f: // orl A,R7
        *Acc = *Acc | R[7];
        PC++;
        break;
    case 0x46: // orl A,@R0
        *Acc = *Acc | SFR[R[0]];
        PC++;
        break;
    case 0x47: // orl A,@R1
        *Acc = *Acc | SFR[R[1]];
        PC++;
        break;
    case 0x54:  // anl A,#(data)
        PC++;
        *Acc = *Acc & prog[PC];
        PC++;
        break;
    case 0x55: // anl A,(adress)
        PC++;
        *Acc = SFR[prog[PC]] & *Acc;
        PC++;
        break;
    case 0x52: // anl (adress),A
        PC++;
        SFR[prog[PC]] = SFR[prog[PC]] & *Acc;
        PC++;
        break;
    case 0x53: // anl (adress),#(data)
        PC++;
        SFR[prog[PC]] = SFR[prog[PC]] & prog[PC + 1];
        PC++;
        break;
    case 0x58: // anl A,R0
        *Acc = *Acc & R[0];
        PC++;
        break;
    case 0x59: // anl A,R1
        *Acc = *Acc & R[1];
        PC++;
        break;
    case 0x5a: // anl A,R2
        *Acc = *Acc & R[2];
        PC++;
        break;
    case 0x5b: // anl A,R3
        *Acc = *Acc & R[3];
        PC++;
        break;
    case 0x5c: // anl A,R4
        *Acc = *Acc & R[4];
        PC++;
        break;
    case 0x5d: // anl A,R5
        *Acc = *Acc & R[5];
        PC++;
        break;
    case 0x5e: // anl A,R6
        *Acc = *Acc & R[6];
        PC++;
        break;
    case 0x5f: // anl A,R7
        *Acc = *Acc & R[7];
        PC++;
        break;
    case 0x56: // anl A,@R0
        *Acc = *Acc & SFR[R[0]];
        PC++;
        break;
    case 0x57: // anl A,@R1
        *Acc = *Acc & SFR[R[1]];
        PC++;
        break;
    case 0xa2: // mov C,(bit)
        PC++;
        if ( check_Bit(prog[PC]))
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        PC++;
        break;
    case 0xb2: // cpl (bit)
        PC++;
        if ( check_Bit(prog[PC]))
        {
            clr_Bit(prog[PC]);
        }
        else
        {
            set_Bit(prog[PC]);
        }
        PC++;
        break;
    case 0xb3: // cpl C
        PC++;
        if ( check_C())
        {
            clr_C();
        }
        else
        {
            set_C();
        }
        break;
    case 0x72: // orl C,(bit)
        PC++;
        if ( check_C() || check_Bit(prog[PC]))
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        PC++;
        break;
    case 0xa0: // orl C,/(bit)
        PC++;
        if ( check_C() || !check_Bit(prog[PC]))
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        PC++;
        break;
    case 0x82: // anl C,(bit)
        PC++;
        if ( check_C() && check_Bit(prog[PC]))
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        PC++;
        break;
    case 0xb0: // anl C,/(bit)
        PC++;
        if ( check_C() && !check_Bit(prog[PC]))
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        PC++;
        break;
    case 0xf4: // cpl A
        *Acc = *Acc ^ 0xff;
        PC++;
        break;
    case 0xa6: // mov @R0,(adress)
        PC++;
        SFR[R[0]] = SFR[prog[PC]];
        PC++;
        break;
    case 0xa7: // mov @R1,(adress)
        PC++;
        SFR[R[1]] = SFR[prog[PC]];
        PC++;
        break;
    case 0x76: // mov @R0,#(data)
        PC++;
        SFR[R[0]] = prog[PC];
        PC++;
        break;
    case 0x77: // mov @R1,#(data)
        PC++;
        SFR[R[1]] = prog[PC];
        PC++;
        break;
    case 0x86: // mov (adress),@R0
        PC++;
        SFR[prog[PC]] = SFR[R[0]];
        PC++;
        break;
    case 0x87: // mov (adress),@R1
        PC++;
        SFR[prog[PC]] = SFR[R[1]];
        PC++;
        break;
    case 0x73: // jmp @A+DPTR
        PC = (uint16_t) ( *Acc + *DPTR );
        break;
    case 0xb4: // cjne A,#(byte),offset
        PC++;
        if ( *Acc < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( *Acc != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xb5: // cjne A,(adress),offset
        PC++;
        if ( *Acc < SFR[prog[PC]] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( *Acc != SFR[prog[PC]] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xb6: // cjne @R0,#(byte),offset
        PC++;
        if ( SFR[R[0]] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( SFR[R[0]] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xb7: // cjne @R1,#(byte),offset
        PC++;
        if ( SFR[R[1]] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( SFR[R[1]] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xb8: // cjne R0,#(byte),offset
        PC++;
        if ( R[0] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[0] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xb9: // cjne R1,#(byte),offset
        PC++;
        if ( R[1] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[1] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xba: // cjne R2,#(byte),offset
        PC++;
        if ( R[2] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[2] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xbb: // cjne R3,#(byte),offset
        PC++;
        if ( R[3] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[3] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xbc: // cjne R4,#(byte),offset
        PC++;
        if ( R[4] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[4] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xbd: // cjne R5,#(byte),offset
        PC++;
        if ( R[5] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[5] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xbe: // cjne R6,#(byte),offset
        PC++;
        if ( R[6] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[6] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0xbf: // cjne R7,#(byte),offset
        PC++;
        if ( R[7] < prog[PC] )
        {
            set_C();
        }
        else
        {
            clr_C();
        }
        if ( R[7] != prog[PC] )
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0x32: // reti
        PC = (uint16_t) SFR[*SP] << 8 | (uint16_t) SFR[*SP - 1];
        *SP = *SP - 2;
        break;
    case 0x30: //jnb (bit),(offset)
        PC++;
        if ( !check_Bit(prog[PC]))
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0x20: //jb (bit),(offset)
        PC++;
        if ( check_Bit(prog[PC]))
        {
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0x10: //jbc (bit),(offset)
        PC++;
        if ( check_Bit(prog[PC]))
        {
            clr_Bit(prog[PC]);
            PC++;
            PC += (signed char) prog[PC] + 1;
        }
        else
        {
            PC += 2;
        }
        break;
    case 0x94: // subb A,#(data)
        PC++;
        _subb(*Acc, prog[PC]); //*Acc=*Acc-prog[PC];
        PC++;
        break;
    case 0x95: // subb A,(adress)
        PC++;
        _subb(*Acc, SFR[prog[PC]]); //*Acc=*Acc-SFR[prog[PC]];
        PC++;
        break;
    case 0x96: // subb A,@R0
        PC++;
        _subb(*Acc, SFR[R[0]]); //*Acc=*Acc-SFR[R[0]];
        PC++;
        break;
    case 0x97: // subb A,@R1
        PC++;
        _subb(*Acc, SFR[R[1]]); //*Acc=*Acc-SFR[R[1]];
        PC++;
        break;
    case 0x98: // subb A,R0
        PC++;
        _subb(*Acc, R[0]); //*Acc=*Acc-R[0];
        PC++;
        break;
    case 0x99: // subb A,R1
        PC++;
        _subb(*Acc, R[1]); //*Acc=*Acc-R[1];
        PC++;
        break;
    case 0x9a: // subb A,R2
        PC++;
        _subb(*Acc, R[2]); //*Acc=*Acc-R[2];
        PC++;
        break;
    case 0x9b: // subb A,R3
        PC++;
        _subb(*Acc, R[3]); //*Acc=*Acc-R[3];
        PC++;
        break;
    case 0x9c: // subb A,R4
        PC++;
        _subb(*Acc, R[4]); //*Acc=*Acc-R[4];
        PC++;
        break;
    case 0x9d: // subb A,R5
        PC++;
        _subb(*Acc, R[5]); //*Acc=*Acc-R[5];
        PC++;
        break;
    case 0x9e: // subb A,R6
        PC++;
        _subb(*Acc, R[6]); //*Acc=*Acc-R[6];
        PC++;
        break;
    case 0x9f: // subb A,R7
        PC++;
        _subb(*Acc, R[7]); //*Acc=*Acc-R[7];
        PC++;
        break;
    case 0xd4:  // da A
        PC++;
        tmpB = *Acc;
        *Acc &= 0xf;
        tmpB >>= 4;
        if (( *Acc > 9 ) || check_AC())
        {
            *Acc += 9;
        }
        if (( tmpB > 9 ) || check_C())
        {
            tmpB += 9;
        }
        *Acc |= tmpB << 4;
        break;
    case 0xd6:  // xchd A,@R0
        PC++;
        tmpB = SFR[R[0]] & 0xf;
        SFR[R[0]] &= 0xf;
        SFR[R[0]] |= ( *Acc ) & 0xf;
        ( *Acc ) &= 0xf;
        ( *Acc ) |= tmpB;
        break;
    case 0xd7:  // xchd A,@R1
        PC++;
        tmpB = SFR[R[1]] & 0xf;
        SFR[R[1]] &= 0xf;
        SFR[R[1]] |= ( *Acc ) & 0xf;
        ( *Acc ) &= 0xf;
        ( *Acc ) |= tmpB;
        break;
    case 0xe2:  // movx A,@R0
        *Acc = ram[R[0]];
        PC++;
        break;
    case 0xe3:  // movx A,@R1
        *Acc = ram[R[1]];
        PC++;
        break;
    case 0xf2:  // movx @R0,A
        ram[R[0]] = *Acc;
        PC++;
        break;
    case 0xf3:  // movx @R1,A
        ram[R[1]] = *Acc;
        PC++;
        break;
    }

    uint8_t cnt = 0;
    // count cycles
    c_time += cycles;

    // count bits in Acc
    for ( uint8_t c = 0 ; c <= 7 ; c++ )
    {
        if (( 1 << c ) & *Acc )
        {
            cnt++;
        }
    }
    // parrity check
    if ( cnt % 2 ) // if cnt parity isn't true
    {
        clr_P();
    }
    else
    {
        // if cnt parity is true
        set_P();
    }
    // setup Rx bank
    tmpB = check_RS1() << 1 | check_RS0();
    R = &SFR[tmpB << 3];

    uint8_t modeT0, modeT1;

    // timers & interrupts
    modeT0 = *TMOD & 0x03;
    modeT1 = ( *TMOD & 0x30 ) >> 4;
    // Timer 0
    clr_Bit(0x8d); // clr bit TF0 - T0 overflow
    if ( check_Bit(0x8c)) // if TR0
    {
        switch ( modeT0 )
        {
        case 0:
            break;
        case 1:
            if ((int) ( *TL0 ) + (int) cycles <= 0xff )
            {
                *TL0 += cycles;
            }
            else
            {
                *TL0 = (uint8_t) ((int) *TL0 - (int) 0x100 + cycles );
                if ((int) ( *TH0 ) < 0xff )
                {
                    *TH0 += 1;
                }
                else
                {
                    *TH0 = 0;
                    set_Bit(0x8d); // set bit TF0 - T0 overflow
                }
            }
            break;
        case 2:
            if ((int) ( *TL0 ) + (int) cycles <= 0xff )
            {
                *TL0 += cycles;
            }
            else
            {
                *TL0 = *TH0;
                set_Bit(0x8d); // set bit TF0 - T0 overflow
            }
            break;
        case 3:
            break;
        }
    }
    clr_Bit(0x8f); // clr bit TF1 - T1 overflow
    if ( check_Bit(0x8e)) // if TR1
    {
        switch ( modeT1 )
        {
        case 0:
            break;
        case 1:
            if ((int) ( *TL1 ) + (int) cycles <= 0xff )
            {
                *TL1 += cycles;
            }
            else
            {
                *TL1 = (uint8_t) ((int) *TL1 - (int) 0x100 + cycles );
                if ((int) ( *TH1 ) < 0xff )
                {
                    *TH1 += 1;
                }
                else
                {
                    *TH1 = 0;
                    set_Bit(0x8f); // set bit TF0 - T0 overflow
                }
            }
            break;
        case 2:
            if ((int) ( *TL1 ) + (int) cycles <= 0xff )
            {
                *TL1 += cycles;
            }
            else
            {
                *TL1 = *TH1;
                set_Bit(0x8f); // set bit TF0 - T0 overflow
            }
            break;
        case 3:
            break;
        }
    }
    // Interrupts
    if ( check_Bit(0xaf)) //if EA is 1
    {
        // T0 int
        if ( check_Bit(0x8d) && check_Bit(0xa9)) // if TF0 and ET0
        {
            *SP = *SP + 1;
            SFR[*SP] = (uint8_t) ( PC & 0x00ff );
            *SP = *SP + 1;
            SFR[*SP] = (uint8_t) ( PC >> 8 );
            PC = 0x0000b;
        }
        // T1 int
        if ( check_Bit(0x8f) && check_Bit(0xab)) // if TF1 and ET1
        {
            *SP = *SP + 1;
            SFR[*SP] = (uint8_t) ( PC & 0x00ff );
            *SP = *SP + 1;
            SFR[*SP] = (uint8_t) ( PC >> 8 );
            PC = 0x001b;
        }
    }
}