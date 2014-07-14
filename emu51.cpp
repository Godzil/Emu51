typedef unsigned char BYTE;
typedef unsigned short int WORD;

#include <stdio.h>

#define USE_CONSOLE

#include "allegro.h"
#include <string.h>
#include <stdlib.h>

FONT *mainf;
#include "gui.h"

#define bit0 0x01
#define bit1 0x02
#define bit2 0x04
#define bit3 0x08
#define bit4 0x10
#define bit5 0x20
#define bit6 0x40
#define bit7 0x80

unsigned long c_time=0;

BYTE SFR[0x100];                   // internal memory and sfr area 256 bytes
BYTE EXT_RAM[0x10000];             // external memory 64kB
BYTE *Acc  = &SFR[0xe0];              // 8-bit accumlator
WORD *DPTR = (WORD*) &SFR[0x82];      // 16-bit register
BYTE *DPH  = &SFR[0x83];              // high byte of DPTR
BYTE *DPL  = &SFR[0x82];              // low byte of DPTR
BYTE *B    = &SFR[0xf0];              // B register
BYTE *SP   = &SFR[0x81];              // Stack Pointer
BYTE *PSW  = &SFR[0xd0];              // Program Status Word
BYTE *P0   = &SFR[0x80];              // 1st Port
BYTE *P1   = &SFR[0x90];              // 2nd Port
BYTE *P2   = &SFR[0xa0];              // 3rd Port
BYTE *P3   = &SFR[0xb0];              // 4th port
BYTE *SBUF = &SFR[0x99];              // Serial transmission Buffer
BYTE *IE   = &SFR[0xa8];              // Int Enable
BYTE *SCON = &SFR[0x98];              // Serial Control
BYTE *TH1  = &SFR[0x8d];              // Timer1 High
BYTE *TH0  = &SFR[0x8c];              // Timer0 High
BYTE *TL1  = &SFR[0x8b];              // Timer1 Low
BYTE *TL0  = &SFR[0x8a];              // Timer0 Low
BYTE *TMOD = &SFR[0x89];              // Timer Mode
BYTE *TCON = &SFR[0x88];              // Timer Control
BYTE *PCON = &SFR[0x87];              // Power Control
BYTE *R    = &SFR[0x00];              // Additional 8 Rx Registers
WORD PC=0x0000;                       // Progam Counter

void init_8051 (void)              // initialize the emulator
{
  int i;
  c_time=0;                        // cycles counter
  for (i=0;i<256;i++)
  {
    SFR[i]=0xA5;                      // all regs in internal RAM & SFR
  }
  *P0=0xff;                        //
  *P1=0xff;                        //  
  *P2=0xff;                        //   Some SFR registers
  *P3=0xff;                        //   and Program Counter
  PC=0x0000;                       //  
  *SP=0x07;                        //
}

BYTE check_C (void)                // 1 or 0
{
  if (*PSW & bit7) return 1;
  else return 0;
}
BYTE check_AC (void)               // 1 or 0
{
  if (*PSW & bit6) return 1;
  else return 0;
}
BYTE check_OV (void)               // 1 or 0
{
  if (*PSW & bit2) return 1;
  else return 0;
}
void set_C (void)                  // C <- 1
{
  *PSW=*PSW | bit7;
}
void set_AC (void)                 // AC <- 1
{
  *PSW=*PSW | bit6;
}
void set_OV (void)                 // OV <- 1
{
  *PSW=*PSW | bit2;
}
void clr_C (void)                  // C <- 0
{
  *PSW=*PSW & 0x7f;
}
void clr_AC (void)                 // AC <- 0
{
  *PSW=*PSW & 0xbf;
}
void clr_OV (void)                 // OV <- 0
{
  *PSW=*PSW & 0xfb;
}
BYTE check_P(void)                 // 1 or 0
{
  if (*PSW & bit0) return 1;
  else return 0;
}
void set_P (void)                  // P <- 1
{
  *PSW=*PSW | bit0;
}
void clr_P (void)                  // P <- 0
{
  *PSW=*PSW & 0xfe;
}
void setB (BYTE bit)               // (bit) <- 1
{
  BYTE tmpB;
  BYTE tmpbit;
  tmpbit=bit & 0x07;
  tmpB=bit & 0xf8;
  if (bit>=128)
  {         // bit set in SFR area
    SFR[tmpB]=SFR[tmpB] | (1<<tmpbit);
  } else {  // bit set in int. RAM
    tmpB=(tmpB>>3)+32;
    SFR[tmpB]=SFR[tmpB] | (1<<tmpbit);
  }
}
void clrB (BYTE bit)               // (bit) <- 0
{
  BYTE tmpB;
  BYTE tmpbit;
  tmpbit=bit & 0x07;
  tmpB=bit & 0xf8;
  if (bit>=128)
  {         // bit clear in SFR area
    SFR[tmpB]=SFR[tmpB] & ~(1<<tmpbit);
  } else {  // bit clear in int. RAM
    tmpB=(tmpB>>3)+32;
    SFR[tmpB]=SFR[tmpB] & ~(1<<tmpbit);
  }
}
BYTE checkB (BYTE bit)       // returns (bit) 1 or 0
{
  BYTE tmpB;
  BYTE tmpbit;
  tmpbit=bit & 0x07;
  tmpB=bit & 0xf8;
  if (bit>=128)
  {         // bit check in SFR area
    if ( SFR[tmpB] & 1<<tmpbit)
    {
      return 1;
    } else {
      return 0;
    }
  } else {  // bit check in int. RAM
    tmpB=(tmpB>>3)+32;
    if (SFR[tmpB] & 1<<tmpbit)
    {
      return 1;
    } else {
      return 0;
    }
  }
}
BYTE check_RS0 (void)
{
  if (*PSW & bit3) return 1;
  else return 0;
}
void set_RS0 (void)
{
  *PSW=*PSW | bit3;
}
void clr_RS0 (void)
{
  *PSW=*PSW & 0xf7;
}
BYTE check_RS1 (void)
{
  if (*PSW & bit4) return 1;
  else return 0;
}
void set_RS1 (void)
{
  *PSW=*PSW | bit4;
}
void clr_RS1 (void)
{
  *PSW=*PSW & 0xef;
}

// 8051 assebmler macros

inline void _add (BYTE &a, BYTE &b)
{
  int tmp1,tmp2;  
  ((0xf & a) + (0xf & b)) > 0xf ? set_AC() : clr_AC();
  tmp1 = ((0x7f & a) + (0x7f & b)) > 0x7f ? 1 : 0;
  (tmp2=a + b)  > 0xff ? (set_C(), tmp1 ? clr_OV() : set_OV()) :
                         (clr_C(), tmp1 ? set_OV() : clr_OV()) ;
  a=(BYTE)tmp2 & 0xff;
}

inline void _addc (BYTE &a, BYTE &b)
{
  int tmp1,tmp2,c;  
  c=check_C();  
  ((0xf & a) + (0xf & b + c )) > 0xf ? set_AC() : clr_AC();
  tmp1 = ( (0x7f & a) + (0x7f & b) + c ) > 0x7f ? 1 : 0;
  (tmp2=a + b + c)  > 0xff ? (set_C(), tmp1 ? clr_OV() : set_OV()) :
                         (clr_C(), tmp1 ? set_OV() : clr_OV()) ;
  a=(BYTE)tmp2 & 0xff;
}


class code_51
{
public:
        char mnem[6];              // instruction mnemonic (2-4 characters)
        BYTE code;                 // instruction code
        BYTE lenght;               // bytes which are needed to write this
                                   // instruction into memory
        BYTE cycles;               // time unit
        char display_string[20];   // string which is ready for displaying,
                                   // it's made by make_ds(WORD) method
        char datas[12];            // string which contains datas which
                                   // will be displayed after mnemonic
        void make_ds (WORD);       // make display string
        void process ();           // process the instruction
};

void code_51::make_ds (WORD ram_pl)
{
        int c,i;
		char tmp[3],tmpW[5];
        display_string[0]=mnem[0];
        display_string[1]=mnem[1];
		display_string[2]=(mnem[2]>=0x61 && mnem[2]<=0x7a) ? mnem[2] : (char)32;
		display_string[3]=(mnem[3]>=0x61 && mnem[3]<=0x7a) ? mnem[3] : (char)32;
		display_string[4]=(mnem[4]>=0x61 && mnem[4]<=0x7a) ? mnem[4] : (char)32;
		display_string[5]=(char)32;
		
        for (c=0;c<=10;c++)
        {
          if (datas[c]!='%')
          {
			  if (datas[c]=='O' && datas[c+1]=='%')
			  {
					if (datas[c+2]=='1')
					{
						sprintf(tmpW,"%4x",(signed char)EXT_RAM[ram_pl+1]+ram_pl+2);
						for (i=0;i<4;i++) display_string[c+6+i]=(tmpW[i]==' ') ? '0' : tmpW[i];
					}
					if (datas[c+2]=='2')
					{
						sprintf(tmpW,"%4x",(signed char)EXT_RAM[ram_pl+2]+ram_pl+3);
						for (i=0;i<4;i++) display_string[c+6+i]=(tmpW[i]==' ') ? '0' : tmpW[i];
					}
					c+=3;

			  } else display_string[c+6]=datas[c];
          } else
          {
             if (datas[c+1]=='1')
             {
               sprintf(tmp,"%2x",EXT_RAM[ram_pl+1]);
               for (i=0;i<2;i++) display_string[c+6+i]= tmp[i]==' ' ? '0' : tmp[i];
             }
             if (datas[c+1]=='2')
             {
               sprintf(tmp,"%2x",EXT_RAM[ram_pl+2]);
			   for (i=0;i<2;i++) display_string[c+6+i]= tmp[i]==' ' ? '0' : tmp[i];
             }
             c++;
          }
        }
}

void code_51::process (void)
{
  BYTE tmpB;
  WORD tmpW;
  switch (code)
  {
        case 0x00:  //nop                      00
          PC++;
        break;
        case 0x74:  // mov a,#(byte)           74 xx
          PC++;
          *Acc=EXT_RAM[PC];
          PC++;
        break;
        case 0x75:  // mov (adress),#(byte)    75 xx yy
          PC++;
          SFR[EXT_RAM[PC]]=EXT_RAM[PC+1];
          PC+=2;
        break;
		case 0x02:  // ljmp 16bit_adres        02 xx yy
          PC++;
          PC=(WORD)EXT_RAM[PC]<<8 | EXT_RAM[PC+1];
        break;
        case 0x80:  // sjmp offset             80 oo
          PC++;
          PC+=(signed char)EXT_RAM[PC]+1;
        break;
        case 0x85:  // mov (adress1),(ardess2) 85 yy xx
          PC++;
          SFR[EXT_RAM[PC+1]]=SFR[EXT_RAM[PC]];
          PC+=2;
        break;
        case 0xe5:  // mov A,(adress)
          PC++;
          *Acc=SFR[EXT_RAM[PC]];
          PC++;
        break;
		case 0x92:  // mov (bit),C
		  PC++;
		  if (check_C()) setB(EXT_RAM[PC]);
		  else clrB(EXT_RAM[PC]);
		  PC++;
		break;
		/* 2003-12-04 : Added */
		case 0xA2:  // mov C,(bit)
		  PC++;
		  if (checkB(EXT_RAM[PC])) set_C();
		  else clr_C();
		  PC++;
		break;
		/* End Added */
        case 0x04:  // inc a
          SFR[0xe0]++;
          PC++;
        break;
        case 0x05:  // inc (adress)
          PC++;
          SFR[EXT_RAM[PC]]++;
          PC++;
        break;
        case 0xa3:  // inc DPTR
          *DPTR=*DPTR+1;
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
          *Acc=*Acc-1;
          PC++;
        break;
        case 0x15: //dec (adress)
          PC++;
          SFR[EXT_RAM[PC]]--;
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
          if (*B!=0)
          {
            tmpB=*Acc;
            *Acc=tmpB / *B;
            *B=tmpB % *B;
          } else set_OV();
          PC++;
        break;
        case 0xa4:  // mul AB
          tmpW=(WORD)*Acc * (WORD)*B;
          if (tmpW>255) set_OV();
          else clr_OV();
          *B=(BYTE) tmpW>>8;
          *Acc=(BYTE) (tmpW&0xff);
          PC++;
        break;
        case 0x23:  // rl A
          tmpB=*Acc;
          *Acc=tmpB<<1 | tmpB>>7;
          PC++;
        break;
        case 0x03:  // rr A
          tmpB=*Acc;
          *Acc=tmpB>>1 | tmpB<<7;
          PC++;
        break;
        case 0x33:  // rlc A
          tmpB=*Acc;
          *Acc=tmpB<<1 | check_C();
          if (tmpB&0x80) set_C();
          else clr_C();
          PC++;
        break;
        case 0x13:  // rrc A
          tmpB=*Acc;
          *Acc=tmpB>>1 | check_C()<<7;
          if (tmpB&0x01) set_C();
          else clr_C();
          PC++;
        break;
        case 0x90:  // mov DPTR,#(16-bit data)
          PC++;
          *DPH=EXT_RAM[PC];
          PC++;
          *DPL=EXT_RAM[PC];
          PC++;
        break;
        case 0xf5:  // mov (adress),A
          PC++;
          SFR[EXT_RAM[PC]]=*Acc;
          PC++;
        break;
        // ADD (full functional)
        case 0x24:  // add A,#(data)
          PC++;
          _add(*Acc, EXT_RAM[PC]);
          PC++;
        break;
        case 0x25:  // add A,(adres)
          PC++;
          _add(*Acc, SFR[EXT_RAM[PC]]);
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
          _addc(*Acc, EXT_RAM[PC]);
          PC++;
        break;
        case 0x35:  // addc A,(adres)
          PC++;
          _addc(*Acc, SFR[EXT_RAM[PC]]);
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
          *Acc=EXT_RAM[*DPTR];
          PC++;
        break;
        case 0xf0:  // movx @DPTR,A
          EXT_RAM[*DPTR]=*Acc;
          PC++;
        break;
        case 0xc0:  // push (adress)
          *SP=*SP+1;
          PC++;
          SFR[*SP]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xd0:  // pop (adress)
          PC++;
          SFR[EXT_RAM[PC]]=SFR[*SP];
          *SP=*SP-1;
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
          setB(EXT_RAM[PC]);
          PC++;
        break;
        case 0xc2:  // clr (bit)
          PC++;
          clrB(EXT_RAM[PC]);
          PC++;
        break;
        case 0x64:  // xrl A,#(data)
          PC++;
          *Acc=*Acc^EXT_RAM[PC];
          PC++;
        break;
		case 0x65:  // xrl A,(adress)
          PC++;
          *Acc=*Acc^SFR[EXT_RAM[PC]];
          PC++;
        break;
		case 0x63:  // xrl (adress),#(data)
          PC++;
          SFR[EXT_RAM[PC]] ^= EXT_RAM[PC+1];
          PC+=2;
        break;
		case 0xc5:  // xch A,(adress)
		  PC++;
		  tmpB=*Acc;
		  *Acc=SFR[EXT_RAM[PC]];
		  SFR[EXT_RAM[PC]]=tmpB;
		  PC++;
		break;
		case 0xc8:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[0];
		  R[0]=tmpB;
		  PC++;
		break;
		case 0xc9:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[1];
		  R[1]=tmpB;
		  PC++;
		break;
		case 0xca:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[2];
		  R[2]=tmpB;
		  PC++;
		break;
		case 0xcb:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[3];
		  R[3]=tmpB;
		  PC++;
		break;
		case 0xcc:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[4];
		  R[4]=tmpB;
		  PC++;
		break;
		case 0xcd:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[5];
		  R[5]=tmpB;
		  PC++;
		break;
		case 0xce:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[6];
		  R[6]=tmpB;
		  PC++;
		break;
		case 0xcf:  // xch A,R0
		  tmpB=*Acc;
		  *Acc=R[7];
		  R[7]=tmpB;
		  PC++;
		break;
        case 0x12:  // lcall 16bit_adres        02 xx yy
          tmpW=PC+3;
          PC++;
          PC=(WORD)EXT_RAM[PC]<<8 | EXT_RAM[PC+1];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0x22: // ret
          PC=(WORD)SFR[*SP]<<8 | (WORD)SFR[*SP-1];
          *SP=*SP-2;
        break;
        case 0x70: // jnz (offset)
          PC++;
          if (*Acc!=0) PC+=(signed char)EXT_RAM[PC]+1;
          else PC++;
        break;
        case 0x60: // jz (offset)
          PC++;
          if (*Acc==0) PC+=(signed char)EXT_RAM[PC]+1;
          else PC++;
        break;
        case 0x40:  // jc (offset)
          PC++;
          if (check_C()) PC+=(signed char)EXT_RAM[PC]+1;
          else PC++;
        break;
        case 0x50:  // jnc (offset)
          PC++;
          if (!check_C()) PC+=(signed char)EXT_RAM[PC]+1;
          else PC++;
        break;
        case 0xc4:  // swap A
          tmpB=*Acc;
          tmpB=(tmpB<<4) & 0xf0;
          *Acc=(*Acc>>4) & 0x0f;
          *Acc=*Acc | tmpB;
          PC++;
        break;
        case 0x93:  // movc A,@DPTR+A
          *Acc=EXT_RAM[*Acc+*DPTR];
          PC++;
        break;
        case 0x83:  // movc A,@PC+A
          *Acc=EXT_RAM[*Acc+PC];
          PC++;
        break;
        case 0xe6:  // mov A,@R0
          *Acc=SFR[R[0]];
          PC++;
        break;
        case 0xe7:  // mov A,@R1
          *Acc=SFR[R[1]];
          PC++;
        break;
        case 0xf6:  // mov @R0,A
          SFR[R[0]]=*Acc;
          PC++;
        break;
        case 0xf7:  // mov @R1,a
          SFR[R[1]]=*Acc;
          PC++;
        break;
        case 0x78:  // mov R0,#(data)
          PC++;
          R[0]=EXT_RAM[PC];
          PC++;
        break;
        case 0x79: // mov R1,#(data)
          PC++;
          R[1]=EXT_RAM[PC];
          PC++;
        break;
        case 0x7a:  // mov R2,#(data)
          PC++;
          R[2]=EXT_RAM[PC];
          PC++;
        break;
        case 0x7b:  // mov R3,#(data)
          PC++;
          R[3]=EXT_RAM[PC];
          PC++;
        break;
        case 0x7c:   // mov R4,#(data)
          PC++;
          R[4]=EXT_RAM[PC];
          PC++;
        break;
        case 0x7d:  // mov R5,#(data)
          PC++;
          R[5]=EXT_RAM[PC];
          PC++;
        break;
        case 0x7e:  // mov R6,#(data)
          PC++;
          R[6]=EXT_RAM[PC];
          PC++;
        break;
        case 0x7f:   // mov R7,#(data)
          PC++;
          R[7]=EXT_RAM[PC];
          PC++;
        break;
        case 0xa8:   // mov R0,(adress)
          PC++;
          R[0]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xa9:  // mov R1,(adress)
          PC++;
          R[1]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xaa:  // mov R2,(adress)
          PC++;
          R[2]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xab:  // mov R3,(adress)
          PC++;
          R[3]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xac:  // mov R4,(adress)
          PC++;
          R[4]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xad:  // mov R5,(adress)
          PC++;
          R[5]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xae:  // mov R6,(adress)
          PC++;
          R[6]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xaf: // mov R7,(adress)
          PC++;
          R[7]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0x88:   // mov (adress),R0
          PC++;
          SFR[EXT_RAM[PC]]=R[0];
          PC++;
        break;
        case 0x89:   // mov (adress),R1
          PC++;
          SFR[EXT_RAM[PC]]=R[1];
          PC++;
        break;
        case 0x8a:   // mov (adress),R2
          PC++;
          SFR[EXT_RAM[PC]]=R[2];
          PC++;
        break;
        case 0x8b:   // mov (adress),R3
          PC++;
          SFR[EXT_RAM[PC]]=R[3];
          PC++;
        break;
        case 0x8c:   // mov (adress),R4
          PC++;
          SFR[EXT_RAM[PC]]=R[4];
          PC++;
        break;
        case 0x8d:   // mov (adress),R5
          PC++;
          SFR[EXT_RAM[PC]]=R[5];
          PC++;
        break;
        case 0x8e:   // mov (adress),R6
          PC++;
          SFR[EXT_RAM[PC]]=R[6];
          PC++;
        break;
        case 0x8f:   // mov (adress),R7
          PC++;
          SFR[EXT_RAM[PC]]=R[7];
          PC++;
        break;
        case 0xf8:  // mov R0,A
          PC++;
          R[0]=*Acc;
        break;
        case 0xf9:  // mov R1,A
          PC++;
          R[1]=*Acc;
        break;
        case 0xfa:  // mov R2,A
          PC++;
          R[2]=*Acc;
        break;
        case 0xfb:  // mov R3,A
          PC++;
          R[3]=*Acc;
        break;
        case 0xfc:  // mov R4,A
          PC++;
          R[4]=*Acc;
        break;
        case 0xfd:  // mov R5,A
          PC++;
          R[5]=*Acc;
        break;
        case 0xfe:  // mov R6,A
          PC++;
          R[6]=*Acc;
        break;
        case 0xff:  // mov R7,A
          PC++;
          R[7]=*Acc;
        break;
        case 0xe8:  // mov A,R0
          PC++;
          *Acc=R[0];
        break;
        case 0xe9:  // mov A,R1
          PC++;
          *Acc=R[1];
        break;
        case 0xea:  // mov A,R2
          PC++;
          *Acc=R[2];
        break;
        case 0xeb:  // mov A,R3
          PC++;
          *Acc=R[3];
        break;
        case 0xec:  // mov A,R4
          PC++;
          *Acc=R[4];
        break;
        case 0xed:  // mov A,R5
          PC++;
          *Acc=R[5];
        break;
        case 0xee:  // mov A,R6
          PC++;
          *Acc=R[6];
        break;
        case 0xef:  // mov A,R7
          PC++;
          *Acc=R[7];
        break;
        case  0xe4: //clr A
          PC++;
          *Acc=0;
        break;
        case 0x01: // ajmp 00xx
          PC++;
          PC=(WORD)EXT_RAM[PC];
        break;
        case 0x21: // ajmp 01xx
          PC++;
          PC=0x0100 | (WORD)EXT_RAM[PC];
        break;
        case 0x41: // ajmp 02xx
          PC++;
          PC=0x0200 | (WORD)EXT_RAM[PC];
        break;
        case 0x61: // ajmp 03xx
          PC++;
          PC=0x0300 | (WORD)EXT_RAM[PC];
        break;
        case 0x81: // ajmp 04xx
          PC++;
          PC=0x0400 | (WORD)EXT_RAM[PC];
        break;
        case 0xa1: // ajmp 05xx
          PC++;
          PC=0x0500 | (WORD)EXT_RAM[PC];
        break;
        case 0xc1: // ajmp 06xx
          PC++;
          PC=0x0600 | (WORD)EXT_RAM[PC];
        break;
        case 0xe1: // ajmp 07xx
          PC++;
          PC=0x0700 | (WORD)EXT_RAM[PC];
        break;
        case 0x11: // acall 00xx
          tmpW=PC+2;
          PC++;
          PC=(WORD) EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0x31: // acall 01xx
          tmpW=PC+2;
          PC++;
          PC=0x0100 | EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0x51: // acall 02xx
          tmpW=PC+2;
          PC++;
          PC=0x0200 | EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0x71: // acall 03xx
          tmpW=PC+2;
          PC++;
          PC=0x0300 | EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0x91: // acall 04xx
          tmpW=PC+2;
          PC++;
          PC=0x0400 | EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0xb1: // acall 05xx
          tmpW=PC+2;
          PC++;
          PC=0x0500 | EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0xd1: // acall 06xx
          tmpW=PC+2;
          PC++;
          PC=0x0600 | EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0xf1: // acall 07xx
          tmpW=PC+2;
          PC++;
          PC=0x0700 | EXT_RAM[PC];
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (tmpW>>8);
        break;
        case 0xd8: // djnz R0,(offset)
          PC++;
          R[0]--;
          if (R[0]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xd9: // djnz R1,(offset)
          PC++;
          R[1]--;
          if (R[1]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xda: // djnz R2,(offset)
          PC++;
          R[2]--;
          if (R[2]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xdb: // djnz R3,(offset)
          PC++;
          R[3]--;
          if (R[3]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xdc: // djnz R4,(offset)
          PC++;
          R[4]--;
          if (R[4]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xdd: // djnz R5,(offset)
          PC++;
          R[5]--;
          if (R[5]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xde: // djnz R6,(offset)
          PC++;
          R[6]--;
          if (R[6]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xdf: // djnz R7,(offset)
          PC++;
          R[7]--;
          if (R[7]!=0) PC+=(signed char)EXT_RAM[PC];
          PC++;
        break;
        case 0xd5: // djnz (adress),(offset)
          PC++;
          tmpB=EXT_RAM[PC];
          PC++;
          SFR[tmpB]--;
          if (SFR[tmpB]!=0) PC+=(signed char)EXT_RAM[PC];
		  PC++;
        break;
        case 0x43: // orl (adress),#(data)
          PC++;
          SFR[EXT_RAM[PC]]=SFR[EXT_RAM[PC]] | EXT_RAM[PC+1];
          PC+=2;
        break;
        case 0x44: // orl A,#(data)
          PC++;
          *Acc=EXT_RAM[PC] | *Acc;
          PC++;
        break;
        case 0x45: // orl A,(adress)
          PC++;
          *Acc=SFR[EXT_RAM[PC]] | *Acc;
          PC++;
        break;
        case 0x42: // orl (adress),A
          PC++;
          SFR[EXT_RAM[PC]] = SFR[EXT_RAM[PC]] | *Acc;
          PC++;
        break;
        case 0x48: // orl A,R0
          *Acc=*Acc | R[0];
          PC++;
        break;
        case 0x49: // orl A,R1
          *Acc=*Acc | R[1];
          PC++;
        break;
        case 0x4a: // orl A,R2
          *Acc=*Acc | R[2];
          PC++;
        break;
        case 0x4b: // orl A,R3
          *Acc=*Acc | R[3];
          PC++;
        break;
        case 0x4c: // orl A,R4
          *Acc=*Acc | R[4];
          PC++;
        break;
        case 0x4d: // orl A,R5
          *Acc=*Acc | R[5];
          PC++;
        break;
        case 0x4e: // orl A,R6
          *Acc=*Acc | R[6];
          PC++;
        break;
        case 0x4f: // orl A,R7
          *Acc=*Acc | R[7];
          PC++;
        break;
        case 0x46: // orl A,@R0
          *Acc=*Acc | SFR[R[0]];
          PC++;
        break;
        case 0x47: // orl A,@R1
          *Acc=*Acc | SFR[R[1]];
          PC++;
        break;
        case 0x54:  // anl A,#(data)
          PC++;
          *Acc=*Acc & EXT_RAM[PC];
          PC++;
        break;
        case 0x55: // anl A,(adress)
          PC++;
          *Acc=SFR[EXT_RAM[PC]] & *Acc;
          PC++;
        break;
        case 0x52: // anl (adress),A
          PC++;
          SFR[EXT_RAM[PC]] = SFR[EXT_RAM[PC]] & *Acc;
          PC++;
        break;
        case 0x53: // anl (adress),#(data)
          PC++;
          SFR[EXT_RAM[PC]] = SFR[EXT_RAM[PC]] & EXT_RAM[PC+1];
          PC++;
		  PC++;
        break;
        case 0x58: // anl A,R0
          *Acc=*Acc & R[0];
          PC++;
        break;
        case 0x59: // anl A,R1
          *Acc=*Acc & R[1];
          PC++;
        break;
        case 0x5a: // anl A,R2
          *Acc=*Acc & R[2];
          PC++;
        break;
        case 0x5b: // anl A,R3
          *Acc=*Acc & R[3];
          PC++;
        break;
        case 0x5c: // anl A,R4
          *Acc=*Acc & R[4];
          PC++;
        break;
        case 0x5d: // anl A,R5
          *Acc=*Acc & R[5];
          PC++;
        break;
        case 0x5e: // anl A,R6
          *Acc=*Acc & R[6];
          PC++;
        break;
        case 0x5f: // anl A,R7
          *Acc=*Acc & R[7];
          PC++;
        break;
        case 0x56: // anl A,@R0
          *Acc=*Acc & SFR[R[0]];
          PC++;
        break;
        case 0x57: // anl A,@R1
          *Acc=*Acc & SFR[R[1]];
          PC++;
        break;
		case 0x82: // anl C,(bit)
		  PC++;
		  if (check_C() && checkB(EXT_RAM[PC])) set_C();
		  else clr_C();
		  PC++;
		break;
		case 0xb0: // anl C,/(bit)
		  PC++;
		  if (check_C() && !checkB(EXT_RAM[PC])) set_C();
		  else clr_C();
		  PC++;
		break;
        case 0xf4: // cpl A
          *Acc=*Acc^0xff;
          PC++;
        break;
		/* 2003-12-04 : Added */
		case 0xB3: // cpl C
          if (check_C()) clr_C();
		  else	set_C();
          PC++;
        break;
		/* End Added */
        case 0xa6: // mov @R0,(adress)
          PC++;
          SFR[R[0]]=SFR[EXT_RAM[PC]];
          PC++;
        break;
        case 0xa7: // mov @R1,(adress)
          PC++;
          SFR[R[1]]=SFR[EXT_RAM[PC]];
          PC++;
        break;
				case 0x76: // mov @R0,#(data)
          PC++;
          SFR[R[0]]=EXT_RAM[PC];
          PC++;
        break;
        case 0x77: // mov @R1,#(data)
          PC++;
          SFR[R[1]]=EXT_RAM[PC];
          PC++;
        break;
        case 0x86: // mov (adress),@R0
          PC++;
          SFR[EXT_RAM[PC]]=SFR[R[0]];
          PC++;
        break;
        case 0x87: // mov (adress),@R1
          PC++;
          SFR[EXT_RAM[PC]]=SFR[R[1]];
          PC++;
        break;
        case 0x73: // jmp @A+DPTR
          PC=(WORD)(*Acc + *DPTR);
        break;
        case 0xb4: // cjne A,#(byte),offset
          PC++;
          if (*Acc < EXT_RAM[PC]) set_C();
          else clr_C();
          if (*Acc!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
        case 0xb5: // cjne A,(adress),offset
          PC++;
          if (*Acc < SFR[EXT_RAM[PC]]) set_C();
          else clr_C();
          if (*Acc!=SFR[EXT_RAM[PC]])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xb8: // cjne R0,#(byte),offset
          PC++;
          if (R[0] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[0]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xb9: // cjne R1,#(byte),offset
          PC++;
          if (R[1] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[1]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xba: // cjne R2,#(byte),offset
          PC++;
          if (R[2] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[2]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xbb: // cjne R3,#(byte),offset
          PC++;
          if (R[3] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[3]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xbc: // cjne R4,#(byte),offset
          PC++;
          if (R[4] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[4]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xbd: // cjne R5,#(byte),offset
          PC++;
          if (R[5] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[5]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xbe: // cjne R6,#(byte),offset
          PC++;
          if (R[6] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[6]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
		case 0xbf: // cjne R7,#(byte),offset
          PC++;
          if (R[7] < EXT_RAM[PC]) set_C();
          else clr_C();
          if (R[7]!=EXT_RAM[PC])
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else
          {
            PC+=2;
          }
        break;
        case 0x32: // reti
          PC=(WORD)SFR[*SP]<<8 | (WORD)SFR[*SP-1];
          *SP=*SP-2;
        break;
        case 0x30: //jnb (bit),(offset)
          PC++;
          if (!checkB(EXT_RAM[PC]))
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else PC+=2;
        break;
        case 0x20: //jb (bit),(offset)
          PC++;
          if (checkB(EXT_RAM[PC]))
          {
            PC++;
            PC+=(signed char)EXT_RAM[PC]+1;
          } else PC+=2;
        break;
        case 0x94: // subb A,#(data)
          PC++;
          *Acc=*Acc-EXT_RAM[PC];
          PC++;
        break;
        case 0x95: // subb A,(adress)
          PC++;
          *Acc=*Acc-SFR[EXT_RAM[PC]];
          PC++;
        break;
		case 0x98: // subb A,R0
          PC++;
          *Acc=*Acc-R[0];
          PC++;
        break;
		case 0x99: // subb A,R1
          PC++;
          *Acc=*Acc-R[1];
          PC++;
        break;
		case 0x9a: // subb A,R2
          PC++;
          *Acc=*Acc-R[2];
          PC++;
        break;
		case 0x9b: // subb A,R3
          PC++;
          *Acc=*Acc-R[3];
          PC++;
        break;
		case 0x9c: // subb A,R4
          PC++;
          *Acc=*Acc-R[4];
          PC++;
        break;
		case 0x9d: // subb A,R5
          PC++;
          *Acc=*Acc-R[5];
          PC++;
        break;
		case 0x9e: // subb A,R6
          PC++;
          *Acc=*Acc-R[6];
          PC++;
        break;
		case 0x9f: // subb A,R7
          PC++;
          *Acc=*Acc-R[7];
          PC++;
        break;
  }

  BYTE cnt=0;
	// count cycles
  c_time+=cycles;

	// count bits in Acc
  for (BYTE c=0;c<=7;c++)
  {
    if ((1<<c) & *Acc) cnt++;
  }
	// parrity check
  if (cnt%2) // if cnt parity isn't true
  {
    clr_P();
  } else {   // if cnt parity is true
    set_P();
  }
	// setup Rx bank
  tmpB = check_RS1()<<1 | check_RS0();
  R=&SFR[tmpB<<3];

  BYTE modeT0,modeT1;

  // timers & interrupts
  modeT0= *TMOD & 0x03;
  modeT1= (*TMOD & 0x30)>>4;
  // Timer 0
  clrB(0x8d); // clr bit TF0 - T0 overflow
  if(checkB(0x8c)) // if TR0
  {
    switch (modeT0)
    {
      case 0:
      break;
      case 1:
           if ((int)(*TL0)+(int)cycles <= 0xff) *TL0+=cycles;
           else
           {
             *TL0=(BYTE)((int)*TL0-(int)0x100+cycles);
             if ((int)(*TH0)<0xff) *TH0+=1;
             else
             {
               *TH0=0;
               setB(0x8d); // set bit TF0 - T0 overflow
             }
           }
      break;
      case 2:
           if ((int)(*TL0)+(int)cycles <= 0xff) *TL0+=cycles;
           else
           {
             *TL0=*TH0;
             setB(0x8d); // set bit TF0 - T0 overflow
           }
      break;
      case 3:
      break;
    }
  }
	clrB(0x8f); // clr bit TF1 - T1 overflow
  if(checkB(0x8e)) // if TR1
  {
    switch (modeT1)
    {
      case 0:
      break;
      case 1:
           if ((int)(*TL1)+(int)cycles <= 0xff) 
		   {
			   *TL1+=cycles;
		   }
           else
           {
             *TL1=(BYTE)((int)*TL1-(int)0x100+cycles);
             if ((int)(*TH1)<0xff) 
			 {
				 *TH1+=1;
			 }
             else
             {
               *TH1=0;
               setB(0x8f); // set bit TF0 - T0 overflow
             }
           }
      break;
      case 2:
           if ((int)(*TL1)+(int)cycles <= 0xff) 
		   {
			   *TL1+=cycles;
		   }
           else
           {
             *TL1=*TH1;
             setB(0x8f); // set bit TF0 - T0 overflow
           }
      break;
      case 3:
      break;
    }
  }
  // Interrupts
  if (checkB(0xaf)) //if EA is 1
  {
    // T0 int
    if (checkB(0x8d) && checkB(0xa9)) // if TF0 and ET0
    {
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (PC&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (PC>>8);
          PC=0x0000b;
    }
		// T1 int
    if (checkB(0x8f) && checkB(0xab)) // if TF1 and ET1
    {
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (PC&0x00ff);
          *SP=*SP+1;
          SFR[*SP]=(BYTE) (PC>>8);
          PC=0x001b;
    }
  }
}

code_51 asm51[256];

class dis_asm                     // display code class
{
  private:
        void hexoutB (int,int,int,BYTE);
  public:
        BYTE *ram;                // pointer to external ram table
        BITMAP *buf, *surface;    // screen buffer, work surface
        int frame;
        int left;
        bool changed;
        dis_asm (BYTE*, BITMAP*); // ext. ram, draw bitmap, constructor
        void blit_it (int, int);
        void draw (WORD);              // x,y,PC (PC is an adr. of register which
                                  // string is displayed in the middle
                                  // of the code monitor)
        ~dis_asm ();
};

class regs
{
  private:
         void hexoutB(int,int,int color,BYTE numb); //x,y,color,byte
  public:
         int frame;
         int left;
         bool changed;
         BYTE *sfr;
         BYTE reg[25];
         char reg_label[25][10];
         BITMAP *surface, *buf;
         void draw ();
         void blit_it (int, int);
         int red;
         int white;
         int lblue;
         int green;
         int lred;
  
         regs (BYTE*, BITMAP*);
         ~regs ();
};

regs::regs (BYTE *tmpB, BITMAP *tmpBMP)
{
  red   = makecol(255,0,0);
  white = makecol(255,255,255);
  lblue = makecol(0,128,255);
  green = makecol(0,255,0);
  lred = makecol(255,64,0);
  frame=2;
  left=12;
  changed=true;
  sfr=tmpB;
  buf=tmpBMP;
  surface=create_bitmap(400,300);
  clear (surface);
};

regs::~regs ()
{};

void regs::blit_it (int x, int y)
{
  blit(surface,buf,0,0,x,y,surface->w,surface->h);
};

void regs::draw ()
{
  clear (surface);
  int c2=0, c;
  textprintf (surface,mainf,left,frame+20,lblue,
             "nr  label    adress  hex dec ascii SEG LED(bin)");
  for (c=frame+30;c<surface->h-20;c+=10)
  {
    textprintf (surface,mainf,left,c,white,
               "%2d: %9s (%2x) =  %2x %3d   %c",
               c2+1,reg_label[c2],reg[c2],sfr[reg[c2]],sfr[reg[c2]],sfr[reg[c2]]);
    draw_SEG_digit (surface,left+288,c,green,4,sfr[reg[c2]]);
    draw_LED_bin   (surface,left+314,c+4,lred,sfr[reg[c2]]);
    c2++;
    if (c2==25) break;
  };
  hline (surface,frame,surface->h-frame,surface->w-frame,white);
  vline (surface,frame,frame,surface->h-frame,white);
  vline (surface,surface->w-frame,frame,surface->h-frame,white);
  for (c=frame;c<=frame+10;c+=2)
  {
    hline (surface,frame,c,surface->w-frame,white);
  }
  textprintf_centre (surface,mainf,surface->w/2,frame+2,white," SFR Registers ");
}

void regs::hexoutB (int x,int y,int color,BYTE numb)
{
  if (numb>0x10)
  {
    textprintf (surface,mainf,x,y,color,"%2x",numb);
  } else
  {
    textprintf (surface,mainf,x,y,color,"%2x",numb);
    textprintf (surface,mainf,x,y,color,"0");
  }
}

dis_asm::dis_asm (BYTE *tmpB, BITMAP *tmp_buf)
{
  frame=2;
  left=12;
  changed=true;
  ram = tmpB;
  buf = tmp_buf;
  surface = create_bitmap(300,500);
  clear (surface);
}

void dis_asm::hexoutB (int x,int y,int color,BYTE numb)
{
  if (numb>0x10)
  {
    textprintf (surface,mainf,x,y,color,"%2x",numb);
  } else
  {
    textprintf (surface,mainf,x,y,color,"%2x",numb);
    textprintf (surface,mainf,x,y,color,"0");
  }
}

void dis_asm::draw (WORD adress)
{
  int red   = makecol(255,0,0);
  int white = makecol(255,255,255);
  int lblue = makecol(0,128,255);
  int color = white;
  int colPC = white;
  int c, cnt;
  clear (surface);
  for (c=20;c<surface->h-10;c+=10)
  {
     if (PC==adress)
     {
       color = red;
     } else {
       color = white;
     }
     asm51[ram[adress]].make_ds(adress);
     if (asm51[ram[adress]].code==ram[adress])
     {
       textprintf (surface,mainf,left,c,color,"%4x:",adress);
       for (cnt=0;cnt<asm51[ram[adress]].lenght;cnt++)
       {
          if (PC==(adress+cnt)) colPC=lblue;
          else colPC=color;
          hexoutB (left+50+cnt*24,c,colPC,ram[adress+cnt]);
       }
       textprintf (surface,mainf,left+144,c,color,"%s",asm51[ram[adress]].display_string);
       adress+=asm51[ram[adress]].lenght;
     } else
     {
       textprintf (surface,mainf,left,c,color,"%4x: ",adress);
       if (PC==adress) colPC=lblue;
          else colPC=color;
       hexoutB (left+50,c,colPC,ram[adress]);
       textprintf (surface,mainf,left+144,c,color,"???");
       adress++;
     }
  }
  hline (surface,frame,surface->h-frame,surface->w-frame,white);
  vline (surface,frame,frame,surface->h-frame,white);
  vline (surface,surface->w-frame,frame,surface->h-frame,white);
  for (c=frame;c<=frame+10;c+=2)
  {
    hline (surface,frame,c,surface->w-frame,white);
  }
  textprintf_centre (surface,mainf,surface->w/2,frame+2,white," Disassembler ");
}

void dis_asm::blit_it (int x,int y)
{
  blit(surface,buf,0,0,x,y,surface->w,surface->h);
}

dis_asm::~dis_asm (void)
{
}

int get_lenght (BYTE code)
{
    if (asm51[code].code==code) return asm51[code].lenght;
    else return 1;
}

class flags
{
  private:
         void hexoutB(int,int,int color,BYTE numb); //x,y,color,byte
  public:
         int frame;
         int left;
         bool changed;
         BYTE *sfr;
         BITMAP *surface, *buf;
         void draw ();
         void blit_it (int, int);
         flags (BITMAP*);
         ~flags ();
};
flags::flags (BITMAP *tmp_buf)
{
  buf=tmp_buf;
  surface=create_bitmap (100,300);
  frame=2;
  left=12;
  changed=false;
}
flags::~flags ()
{
}
void flags::draw ()
{
  int red   = makecol(255,0,0);
  int white = makecol(255,255,255);
  int lblue = makecol(0,128,255);
  clear (surface);
  textprintf (surface,mainf,left,frame+20,white,
             "C  : %d",check_C());
  textprintf (surface,mainf,left,frame+30,white,
             "AC : %d",check_AC());
  textprintf (surface,mainf,left,frame+40,white,
             "OV : %d",check_OV());
  textprintf (surface,mainf,left,frame+50,white,
             "P  : %d",check_P());
  textprintf (surface,mainf,left,frame+60,red,
             "PC : %4x",PC);
  textprintf (surface,mainf,left,frame+70,lblue,
             "SP : %2x",*SP);
  textprintf (surface,mainf,left,frame+80,lblue,
             "Acc: %2x",*Acc);
  textprintf (surface,mainf,left,frame+90,lblue,
             "B  : %2x",*B);
  textprintf (surface,mainf,left,frame+100,lblue,
             "R0 : %2x",R[0]);
  textprintf (surface,mainf,left,frame+110,lblue,
             "R1 : %2x",R[1]);
  textprintf (surface,mainf,left,frame+120,lblue,
             "R2 : %2x",R[2]);
  textprintf (surface,mainf,left,frame+130,lblue,
             "R3 : %2x",R[3]);
  textprintf (surface,mainf,left,frame+140,lblue,
             "R4 : %2x",R[4]);
  textprintf (surface,mainf,left,frame+150,lblue,
             "R5 : %2x",R[5]);
  textprintf (surface,mainf,left,frame+160,lblue,
             "R6 : %2x",R[6]);
  textprintf (surface,mainf,left,frame+170,lblue,
             "R7 : %2x",R[7]);
  textprintf (surface,mainf,left,frame+180,lblue,
             "DPTR: %4x",*DPTR);
  textprintf (surface,mainf,left,frame+270,red,
             "cycles :");
  textprintf (surface,mainf,left,frame+280,red,
             "%lld",c_time);

  hline (surface,frame,surface->h-frame,surface->w-frame,white);
  vline (surface,frame,frame,surface->h-frame,white);
  vline (surface,surface->w-frame,frame,surface->h-frame,white);
  for (int c=frame;c<=frame+10;c+=2)
  {
    hline (surface,frame,c,surface->w-frame,white);
  }
  textprintf_centre (surface,mainf,surface->w/2,frame+2,white," RegFlag ");
}
void flags::blit_it (int x,int y)
{
  blit(surface,buf,0,0,x,y,surface->w,surface->h);
}

class ramv
{
  private:
         void hexoutB(int,int,int color,BYTE numb); //x,y,color,byte
  public:
         int frame;
         int left;
         bool changed;
         BYTE *ram;
         BYTE reg[25];
         char reg_label[25][10];
         BITMAP *surface, *buf;
         void draw (WORD);
         void blit_it (int, int);
         ramv (BYTE*, BITMAP*);
         ~ramv ();
};

ramv::ramv (BYTE *tmpB, BITMAP *tmpBMP) // extram, screen
{
  frame=2;
  left=12;
  changed=true;
  ram = tmpB;
  buf = tmpBMP;
  surface=create_bitmap(500,200);
  clear (surface);
}

ramv::~ramv ()
{
}

void ramv::blit_it (int x, int y)
{
  blit(surface,buf,0,0,x,y,surface->w,surface->h);
}

void ramv::draw (WORD cur)
{
  int red   = makecol(255,0,0);
  int white = makecol(255,255,255);
  int lgreen = makecol(128,255,128);
  int green = makecol(0,255,0);
  int lblue = makecol(0,128,255);
  int yellow = makecol (255,255,0);
  clear (surface);
  int c2=0, c;
  int y;
  y=frame+20;
  int curcol=white;
  for (c=cur;true;c+=8)
  {
    if (!(y<surface->h-10)) break;
    textprintf (surface,mainf,left,y,lblue,"%4X: ",(WORD)(c));
    for (c2=0;c2<8;c2++)
    {
      if (PC==(WORD)(c+c2))
      {
        curcol=red;
        draw_SEG_digit (surface,left+326+c2*8,y, yellow,3,ram[(WORD)(c+c2)]);
      } else if (*DPTR==(WORD)(c+c2))
	  {
		curcol=lgreen;
        draw_SEG_digit (surface,left+326+c2*8,y, yellow,3,ram[(WORD)(c+c2)]);
	  } else
      {
        curcol=white;
        draw_SEG_digit (surface,left+326+c2*8,y, green,3,ram[(WORD)(c+c2)]);
      }
      hexoutB (left+48+c2*24,y,curcol,ram[(WORD)(c+c2)]);
      textprintf (surface,font,left+252+c2*8,y,curcol,"%c",ram[(WORD)(c+c2)]);

    }
    y+=10;
  }
  hline (surface,frame,surface->h-frame,surface->w-frame,white);
  vline (surface,frame,frame,surface->h-frame,white);
  vline (surface,surface->w-frame,frame,surface->h-frame,white);
  for (c=frame;c<=frame+10;c+=2)
  {
    hline (surface,frame,c,surface->w-frame,white);
  }
  textprintf_centre (surface,mainf,surface->w/2,frame+2,white," RAM Monitor ");
}

void ramv::hexoutB (int x,int y,int color,BYTE numb)
{
  if (numb>0x10)
  {
    textprintf (surface,mainf,x,y,color,"%2x",numb);
  } else
  {
    textprintf (surface,mainf,x,y,color,"%2x",numb);
    textprintf (surface,mainf,x,y,color,"0");
  }
}

/*bool vret (void)
{
  if (inportb(0x3da) & 8) return true;
  else return false;
  
}*/

void wait_for_retrace (void)
{
  /*while (!vret())
  {
  }*/
  //vsync();
}

int load_code_table (code_51 *tmp_tab)
{
  FILE *strio;
  char _mnem[6];
  int _code;
  int _lenght;
  int _cycles;
  char _datas[12];
  if ( (strio = fopen ( "list51.txt" , "r" )) == NULL )
  {
	fprintf(stderr,"Can't find file list51.txt !!!!\n");
	exit(-1);
  }
  for (int c=0;c<256;c++)
  {
     fscanf (strio,"%x",&_code);
     fscanf (strio,"%s",_mnem);
     fscanf (strio,"%s",_datas);
     fscanf (strio,"%d",&_lenght);
     fscanf (strio,"%d",&_cycles);
     if (_mnem[0]=='E' && _mnem[1]=='N' && _mnem[2]=='D') break;
     tmp_tab[_code].code=(BYTE)_code;
     strcpy (tmp_tab[_code].mnem,_mnem);
     strcpy (tmp_tab[_code].datas,_datas);
     tmp_tab[_code].lenght=(BYTE)_lenght;
     tmp_tab[_code].cycles=(BYTE)_cycles;
  }
  fclose (strio);
  return 0;
}

int load_ram (char *filename,BYTE *ram)
{
  FILE *strin;
  strin=NULL;
  if (*filename!=0)
  {
	strin = fopen (filename,"rb");
  }
  if ( strin == NULL)
  {
    ShowMessage ("Can't open the file",screen,250,100,200,100,"Error!");
  } else {
    WORD c=0;
    for (c;!feof(strin);c++)
		ram[c]=fgetc(strin);

	fclose (strin);
  }

  return 0;
}

int load_hex (char *filename,BYTE *ram)
{
  FILE *strin;
  int Adr=0;
  int Cnt;
  //int to_mem;
  int i;
  char line[50];
  char word[5];
  char byte[3];

  byte[2]=word[4]=0;
  strin=NULL;

  if (*filename != 0) strin = fopen (filename,"r");
  
  if ( strin == NULL)
  {
    ShowMessage ("Can't open the file",screen,250,100,200,100,"Error!");
	return -1;
  } else {
    while (!feof(strin))
    {
      fscanf (strin, "%s", line);
	  //fprintf (out,"%s\n", line);
      byte[0]=line[1];
      byte[1]=line[2];
      sscanf (byte,"%x",&Cnt);
      word[0]=line[3];
      word[1]=line[4];
      word[2]=line[5];
      word[3]=line[6];
      sscanf (word,"%x",&Adr);
      byte[0]=line[7];
      byte[1]=line[8];
      sscanf (byte,"%x",&i);
      if (i==1) break;
      else
      {
		byte[2]=0;
        for (i=9;i<9+Cnt*2;i+=2)
        {
          byte[0]=line[i];
          byte[1]=line[i+1];
	 
          sscanf (byte, "%x", (ram+Adr));
          
          Adr++;
        }
      }
    }
	fclose (strin);
	//fclose (out);
  }  
  return 0;
}

void
ChangeMonitedSFRRegister
(regs* regwnd)
{
  char lab[10];
  char ln[3];
  int  line_num=0;
  int  reg_adr=0;
  GetText (ln,screen,50,50,300,70,3,"Enter line number (dec)");
  sscanf (ln,"%d", &line_num);
  if  (line_num<1 || line_num>25) return;
  GetText (lab,screen,50,80,300,70,10,"Enter reg. label (ascii)");
  GetText (ln,screen,50,100,300,70,3,"Enter register number (hex)");
  sscanf (ln,"%x", &reg_adr);
  line_num--;
  strcpy (regwnd->reg_label[line_num], lab);
  regwnd->reg [line_num] = (BYTE)reg_adr;
  return;
}
void reset51 (void)
{
	if (QuestionBox("Are you sure want to reset 8051",screen,250,240,300,60,"Confirm!",makecol(255,255,255))) 
		init_8051();
  return;
}
WORD ViewAt (void)
{
  char hex4[6];
  int tmp_adr;
  GetText (hex4,screen,50,80,300,70,5,"Enter new view point (hex)");
  if (hex4[0]==32) return 0;
  sscanf (hex4,"%x",&tmp_adr);
  return (WORD) tmp_adr;
}
void ChangePC (void)
{
  char hex4[5];
  int tmpPC=0;
  GetText (hex4,screen,50,80,300,70,5,"Enter new PC (hex)");
  if (hex4[0]==32) return;
  sscanf (hex4,"%x",&tmpPC);
  PC=(WORD)tmpPC;
  return;
}
void ChangeRegValue_SFR (void)
{
  char hex2[3];
  char hex2_a[3];
  int tmp_adr;
  int tmp_dat;
  GetText (hex2_a,screen,50,80,300,70,3,"Enter adress from SFR (hex)");
  if (hex2_a[0]==32) return;
  GetText (hex2,screen,60,90,300,70,3,"Enter byte (hex)");
  sscanf (hex2_a,"%x",&tmp_adr);
  sscanf (hex2,"%x",&tmp_dat);
  SFR [(BYTE) tmp_adr] = (BYTE) tmp_dat;
  return;
}
void ChangeRegValue_RAM (void)
{
  char hex2[3];
  char hex4_a[5];
  int tmp_adr;
  int tmp_dat;
  GetText (hex4_a,screen,50,80,400,70,5,"Enter adress from External Memory (hex)");
  if (hex4_a[0]==32) return;
  GetText (hex2,screen,60,90,300,70,3,"Enter byte (hex)");
  sscanf (hex4_a,"%x",&tmp_adr);
  sscanf (hex2,"%x",&tmp_dat);
  EXT_RAM [(WORD) tmp_adr] = (BYTE) tmp_dat;
  return;
}
void LoadRAM (void)
{
  char filename[40];
  GetText (filename,screen,30,80,700,70,40,"Load *.bin file");
  for (int c=38;filename[c]==' ' || filename[c]=='\r';c--)
  {
    filename[c]=0;
  }
  load_ram (filename,EXT_RAM);
	return;
}
void LoadHEX (void)
{
  char filename[40];
  GetText (filename,screen,30,80,700,70,40,"Load *.hex file");
  for (int c=38;filename[c]==' ' || filename[c]=='\r';c--) filename[c]=0;
  load_hex (filename,EXT_RAM);
  return;
}
void LoadSrc (void)
{
	char cmd[256];
	char filename[256];
	char comp_msg[512];
	FILE *msgin;
	int c=0;

	GetText (filename,screen,30,80,700,70,40,"Load and compile source file (*.*)");
	sprintf (cmd,"asm51.exe %s > out.tmp",filename);

	system (cmd);

	msgin=NULL;
  
	msgin = fopen ("out.tmp","rb");
  
	if ( msgin == NULL)
	{
		ShowMessage ("Can't open the file: out.tmp",screen,250,100,200,100,"Error!");
	} else {    
		for (c;c<=0xffff;c++)
		{
			comp_msg[c]=fgetc(msgin);
			if (feof(msgin)) break;
		}
	    fclose (msgin);
	}
	
	ShowMsgEx(comp_msg,screen,100,80,600,140,"Assembling status",makecol(255,255,255));

	for (c=512;c>0;c--)
	{
		if (comp_msg[c]==',' && comp_msg[c+2]=='0')
		{
				change_ext (filename);
				if (load_hex (filename,EXT_RAM)==0)	ShowMessage ("Compiled file is loaded.",screen,250,100,300,80,"Message!");
				break;			
		}
	}
	return;
}
void EditSrc (void)
{
	code_editor ed1;
	ed1.process();
	return;
}
void ChMaxI (int *maxi)
{
  char dec6_a[7];
  GetText (dec6_a,screen,50,80,400,70,7,"Enter number of instructions per frame");
  if (dec6_a[0]==32) return;
  sscanf (dec6_a,"%d",maxi);
  return;
}
void FillExt (void)
{
  char h4beg[5], h4end[5], h2f[3];
  int from, to, i, ic=1, fill;
  GetText (h4beg,screen,50,80,400,70,5,"From adress... (16bit hex)");
  GetText (h4end,screen,50,80,400,70,5,"...to adress (16bit hex)");
  GetText (h2f,screen,50,80,400,70,3,"with byte (8bit hex)");
  sscanf (h4beg,"%x",&from);
  sscanf (h4end,"%x",&to);
  sscanf (h2f,"%x",&fill);
  if (from>to) ic=-1;
  for (i=from;i!=to;i+=ic)
      EXT_RAM[i]=fill;
  EXT_RAM[to]=fill;
  return;
}
void main_init (void)
{
  load_code_table (asm51);
  //load_ram ("lekcja3.bin",EXT_RAM);
  //load_ram ("avt.bin",EXT_RAM);

  init_8051();
  allegro_init();
  install_timer();
  install_keyboard();
  //install_mouse();
  set_color_depth (8);
  set_gfx_mode (GFX_AUTODETECT_WINDOWED, 800, 600,0,0);
  //set_gfx_mode (GFX_AUTODETECT, 800, 600,0,0);

  set_palette (desktop_palette);
  //show_mouse (screen);
  RGB col;
  col.r=0;
  col.g=0;
  col.b=0;
  set_color (0,&col);
  col.r=63;
  col.g=15;
  col.b=0;
  set_color (1,&col);
  col.r=20;
  col.g=0;
  col.b=0;
  set_color (2,&col);
  col.r=255;
  col.g=255;
  col.b=255;
  set_color (255,&col);
  DATAFILE *dataf;
  dataf=load_datafile("data.dat");
  mainf=(FONT*) dataf[0].dat;
}
int main (void)
{
  main_init();

  int white,red,lred,green,blue;
  white=makecol (255,255,255);
  red=makecol (255,0,0);
  lred=makecol (255,64,0);
  green=makecol (0,255,0);
  blue=makecol (0,0,255);

  dis_asm disasm(EXT_RAM,screen);
  regs SFRregs(SFR,screen);
  flags FlagWnd (screen);
  ramv RMon (EXT_RAM,screen);
  strcpy(SFRregs.reg_label[0],"Delay AVT");
  SFRregs.reg[0]=0x75;
  strcpy(SFRregs.reg_label[1],"Key AVT  ");
  SFRregs.reg[1]=0x76;
  strcpy(SFRregs.reg_label[2],"B        ");
  SFRregs.reg[2]=0xF0;
  strcpy(SFRregs.reg_label[3],"Acc      ");
  SFRregs.reg[3]=0xE0;
  strcpy(SFRregs.reg_label[4],"PSW      ");
  SFRregs.reg[4]=0xD0;
  strcpy(SFRregs.reg_label[5],"P0       ");
  SFRregs.reg[5]=0x80;
  strcpy(SFRregs.reg_label[6],"P1       ");
  SFRregs.reg[6]=0x90;
  strcpy(SFRregs.reg_label[7],"P2       ");
  SFRregs.reg[7]=0xa0;
  strcpy(SFRregs.reg_label[8],"P3       ");
  SFRregs.reg[8]=0xb0;
  strcpy(SFRregs.reg_label[9],"IP       ");
  SFRregs.reg[9]=0xB8;
  strcpy(SFRregs.reg_label[10],"IE       ");
  SFRregs.reg[10]=0xA8;
  strcpy(SFRregs.reg_label[11],"SCON     ");
  SFRregs.reg[11]=0x98;
  strcpy(SFRregs.reg_label[12],"SBUF     ");
  SFRregs.reg[12]=0x99;
  strcpy(SFRregs.reg_label[13],"TCON     ");
  SFRregs.reg[13]=0x88;
  strcpy(SFRregs.reg_label[14],"TMOD     ");
  SFRregs.reg[14]=0x89;
  strcpy(SFRregs.reg_label[15],"TL0      ");
  SFRregs.reg[15]=0x8A;
  strcpy(SFRregs.reg_label[16],"TL1      ");
  SFRregs.reg[16]=0x8B;
  strcpy(SFRregs.reg_label[17],"TH0      ");
  SFRregs.reg[17]=0x8C;
  strcpy(SFRregs.reg_label[18],"TH1      ");
  SFRregs.reg[18]=0x8D;
  strcpy(SFRregs.reg_label[19],"SP       ");
  SFRregs.reg[19]=0x81;
  strcpy(SFRregs.reg_label[20],"DPL      ");
  SFRregs.reg[20]=0x82;
  strcpy(SFRregs.reg_label[21],"DPH      ");
  SFRregs.reg[21]=0x83;
  strcpy(SFRregs.reg_label[22],"PCON     ");
  SFRregs.reg[22]=0x87;
  strcpy(SFRregs.reg_label[23],"1        ");
  SFRregs.reg[23]=0x09;
  strcpy(SFRregs.reg_label[24],"0        ");
  SFRregs.reg[24]=0x08;
  PC=0x0000;
  WORD aadr=0x0000;
  WORD madr=0x0000;
  bool manual_scrolling=true;
  bool step=true;
  bool into=false;
  bool over=false;
  unsigned long cnt=0;
  int break_point=-1; char break_str[5];
  //load_ram("avt.bin",EXT_RAM);
  int i=0,maxi=45000;
  while (!(key[KEY_X] && key[KEY_ALT]))
  {
    rect (screen,2,2,798,18,white);
    // key shortcuts disp
    rect (screen,2,521,798,598,white);
    textprintf (screen,mainf,128,528,white,
               "F1     F2     F3     F4     F5     F6     F7     F8     F9     F10    F11    F12" );
    textprintf (screen,mainf,128,548,white,
               "Help   ChSFR  ManS   Debug  DView  WrPC   TInto  SOver  SrcEd  FillE  Speed  Reset");
    textprintf (screen,mainf,128,568,white,
               "WrRAM  WrSFR  AutoS  Run    MView         LdBIN  LdHEX  ScrCpl BrkPt");
    hline (screen,2,540,798,white);
    textprintf (screen,mainf,6,548,white,"Function key :");
    hline (screen,2,560,798,white);
    textprintf (screen,mainf,6,568,white,"Shift+key    :");
    hline (screen,2,580,798,white);
    textprintf (screen,mainf,6,587,white,"Alt+X: Exit  ");
	textout (screen, mainf, "emu51 v0.02a alpha (Mariusz Kasolik Changes:Trapier Manoel)",6,7,white);
    textout (screen, mainf, "AVT-2250 Segment display:",490,7,white);


    cnt++;
    if (manual_scrolling)
    {
      disasm.draw (aadr);
    } else {
      disasm.draw (PC);
    }
    disasm.blit_it (0, 20);
    RMon.draw (madr);
    RMon.blit_it(300,320);
    SFRregs.draw ();
    SFRregs.blit_it (400, 20);
    FlagWnd.draw ();
    FlagWnd.blit_it (300,20);

    if (!step)
    {
      for (i=0;i<maxi;i++)
	  {
		  if (PC==break_point || *SP==0)
		  {
			  step=true;
			  break;
		  }
          asm51[EXT_RAM[PC]].process();
	  }
    }
    else {
      if (over)
      {
        asm51[EXT_RAM[PC]].process();
        over=false;
      }
      if (into)
      {
        asm51[EXT_RAM[PC]].process();
        into=false;
      }
    }

    for (i=0;i<=7;i++)   // for compatibility with avt2250
      draw_SEG_digit (screen,700+8*i,6,lred,4,SFR[i+0x78]);
    if (key[KEY_LCONTROL])
    {
      if (key[KEY_0_PAD]) SFR[0x76]=0x30;
      if (key[KEY_1_PAD]) SFR[0x76]=0x31;
      if (key[KEY_2_PAD]) SFR[0x76]=0x32;
      if (key[KEY_3_PAD]) SFR[0x76]=0x33;
      if (key[KEY_4_PAD]) SFR[0x76]=0x34;
      if (key[KEY_5_PAD]) SFR[0x76]=0x35;
      if (key[KEY_6_PAD]) SFR[0x76]=0x36;
      if (key[KEY_7_PAD]) SFR[0x76]=0x37;
      if (key[KEY_8_PAD]) SFR[0x76]=0x38;
      if (key[KEY_9_PAD]) SFR[0x76]=0x39;
      if (key[KEY_INSERT]) SFR[0x76]=0x41;
      if (key[KEY_HOME]) SFR[0x76]=0x42;
      if (key[KEY_PGUP]) SFR[0x76]=0x43;
      if (key[KEY_DEL]) SFR[0x76]=0x44;
      if (key[KEY_END]) SFR[0x76]=0x45;
      if (key[KEY_PGDN]) SFR[0x76]=0x46;
      if (key[KEY_ENTER_PAD]) SFR[0x76]=0x0d;
    } else
    {
      if (key[KEY_UP]) aadr-=3;
      if (key[KEY_DOWN])
      {
        aadr+=get_lenght(EXT_RAM[aadr]);
        aadr+=get_lenght(EXT_RAM[aadr]);
      }
      if (key[KEY_PGUP] && !(key[KEY_LSHIFT] || key[KEY_RSHIFT])) madr-=16;
      if (key[KEY_PGDN] && !(key[KEY_LSHIFT] || key[KEY_RSHIFT])) madr+=16;

      if (key[KEY_PGUP] && (key[KEY_LSHIFT] || key[KEY_RSHIFT])) madr-=1;
      if (key[KEY_PGDN] && (key[KEY_LSHIFT] || key[KEY_RSHIFT])) madr+=1;

      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F2]) ChangeMonitedSFRRegister(&SFRregs);
      if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F2]) ChangeRegValue_SFR();
      if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F1]) ChangeRegValue_RAM();
      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F3]) manual_scrolling=true;
      if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F3]) manual_scrolling=false;
      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F4]) step=true;
      if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F4]) step=false;
      if (step)
      {
        if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F7]) into=true;
        while (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F7])
        {
        }
        if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F8]) over=true;
        while (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F8])
        {
        }
      }
      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F5])   aadr=ViewAt();
      if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F5])    madr=ViewAt();
      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F6])   ChangePC();
      if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F7])    LoadRAM();
      if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F8])    LoadHEX();
	  if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F9])    LoadSrc();
	  if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F9])   EditSrc();
      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F12])  reset51();
      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F11])  ChMaxI(&maxi);
      if (!(key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F10])  FillExt();
	  if ((key[KEY_LSHIFT] || key[KEY_RSHIFT]) && key[KEY_F10])
	  {
		  GetText (break_str,screen,50,80,400,70,5,"Enter break piont adress");
		  sscanf (break_str,"%x",&break_point);
	  }
    }
  }
  return 0;
} END_OF_MAIN();

