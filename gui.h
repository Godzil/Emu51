#ifndef __MARIO_GUI
#define __MARIO_GUI

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>

bool checkBit (BYTE tmp_b,int pos)   // Returns state of a single bit
{                                    // which is on 'pos' position
  if ( tmp_b&(1<<pos) )              // in tmp_b byte
  {
    return true;
  } else {
    return false;
  }
}

void
draw_SEG_digit
(BITMAP *buf, int x, int y, int color, int size, BYTE segm_code)
{
  int darkc,normc;
  normc=color;
  darkc=makecol( getr(normc)/4,getg(normc)/4,getb(normc)/4);
  if (checkBit (segm_code,1))
  {
    color=normc;
  } else
  {
    color=darkc;
  }
  vline (buf, x+size, y, y+size,color);
  if (checkBit (segm_code,2))
  {
    color=normc;
  }	else
  {
    color=darkc;
  }
  vline (buf, x+size, y+size, y+2*size,color);

  if (checkBit (segm_code,4))
  {
    color=normc;
  } else
  {
    color=darkc;
  }
  vline (buf, x, y+2*size, y+size,color);
  if (checkBit (segm_code,5))
  {
    color=normc;
  } else
  {
    color=darkc;
  }
  vline (buf, x, y+size, y,color);
  if (checkBit (segm_code,6))
  {
    color=normc;
  } else
  {
    color=darkc;
  }
  hline (buf, x, y+size, x+size,color);
  if (checkBit (segm_code,0))
  {
    color=normc;
  } else
  {
    color=darkc;
  }
  hline (buf, x, y, x+size,color);
  if (checkBit (segm_code,3))
  {
    color=normc;
  } else
  {
    color=darkc;
  }
  hline (buf, x+size, y+2*size, x,color);
  if (checkBit (segm_code,7))
  {
    color=normc;
  } else
  {
    color=darkc;
  }
  putpixel (buf, x+size+2, y+2*size, color);
}

void
draw_LED_bin
(BITMAP *buf, int x, int y, int color, BYTE code)
{
  int darkc,normc;
  int i;
  normc=color;
  darkc=makecol( getr(normc)/4,getg(normc)/4,getb(normc)/4);
  for (i=0;i<8;i++)
  {
    if (checkBit (code,7-i))
    {
      color=normc;
    } else
    {
      color=darkc;
    }
    circlefill (buf, x+7*i, y, 2,color);
  }
}

void change_char (char *string,char ch,int pos)
{
  string[pos-1]=ch;
}

void insert_char (char *string,char ch,int pos,int lenght)
{
  for (int c=lenght;c>pos;c--)
  {
    string[c-1]=string[c-2];
  }
  string[pos-1]=ch;
}

void cut_char (char *string,int pos,int lenght)
{
  for (int c=pos;c<lenght;c++)
  {
    string[c-1]=string[c];
  }
  string[lenght-1]=(char)(32);
}

void change_ext(char *s)
{
	int c; 
  for (c=254;c>=0;c--)
  {
	if (s[c]=='.')
	{
		s[c+1]='h';
		s[c+2]='e';
		s[c+3]='x';
		break;
	}
  }
  return;
}

void
GetText
(char *text, BITMAP* buf, int x, int y, int w, int h, int lenght, char *title)
{
  int c;
  BITMAP *surface = create_bitmap(w,h);
  BITMAP *cache = create_bitmap(w,h);
  blit (buf,cache,x,y,0,0,w,h);
  clear (surface);
  hline (surface,2,surface->h-2,surface->w-2,makecol (255,255,255));
  vline (surface,2,2,surface->h-2,makecol (255,255,255));
  vline (surface,surface->w-2,2,surface->h-2,makecol (255,255,255));
  for (c=2;c<=2+10;c+=2)
  {
    hline (surface,2,c,surface->w-2,makecol (255,255,255));
  }
  textprintf_centre (surface,mainf,surface->w/2,4,makecol (255,255,255)," %s ",title);
  text[0]=32;
  for (c=1;c<=lenght-1;c++)
  {
    text[c]=32;
  }
  text[lenght-1]=0;
  clear_keybuf();
  int pos=1;
  WORD key_p;
  WORD key_ch;
  bool flag;
  while (key[KEY_ENTER])
  {
  }
  while (!key[KEY_ENTER])
  {
    textprintf (surface,mainf,15,surface->h/2,makecol (255,255,255),"%s",text);
    hline (surface,15+(pos-1)*8,surface->h/2+8,15+pos*8,makecol (255,255,255));
    blit (surface,buf,0,0,x,y,w,h);
    hline (surface,15+(pos-1)*8,surface->h/2+8,15+pos*8,makecol (0,0,0));
    key_p = readkey();
    key_ch = key_p & 0xff;
    key_p=key_p>>8;
    flag=false;
    switch (key_p)
    {
      case KEY_BACKSPACE:
        if (pos>1)
        {
          pos--;
          cut_char (text,pos,lenght);
          text[lenght-2]=32;
          text[lenght-1]=0;
        }
        flag=true;
      break;
      case KEY_DEL:
        cut_char (text,pos,lenght);
        text[lenght-2]=32;
        text[lenght-1]=0;
        flag=true;
      break;
      case KEY_LEFT:
        if (pos>1) pos--;
        flag=true;
      break;
      case KEY_RIGHT:
        if (pos<lenght) pos++;
        flag=true;
      break;
    }
    if ( !key[KEY_ENTER] && key_ch!=0 && pos<lenght && !flag)
    {
      insert_char ( text, key_ch, pos, lenght );
      pos++;
      text[lenght-1]=0;
    }
  }

  text[lenght-1]=0;
  blit (cache,buf,0,0,x,y,w,h);
	destroy_bitmap(cache);
	destroy_bitmap(surface);
}
void
ShowMessage
(char *text, BITMAP* buf, int x, int y, int w, int h, char *title)
{
  BITMAP *surface = create_bitmap(w,h);
  BITMAP *cache = create_bitmap(w,h);
  blit (buf,cache,x,y,0,0,w,h);
  clear (surface);
  hline (surface,2,surface->h-2,surface->w-2,makecol (255,255,255));
  vline (surface,2,2,surface->h-2,makecol (255,255,255));
  vline (surface,surface->w-2,2,surface->h-2,makecol (255,255,255));
  for (int c=2;c<=2+10;c+=2)
  {
    hline (surface,2,c,surface->w-2,makecol (255,255,255));
  }
  textprintf_centre (surface,mainf,surface->w/2,4,makecol (255,255,255)," %s ",title);
  textprintf (surface,mainf,15,surface->h/2,makecol (255,255,255),"%s",text);
	textprintf_centre (surface,mainf,surface->w/2,surface->h-16,makecol (255,0,0),"Press ENTER");
  blit (surface,buf,0,0,x,y,w,h);
  while (key[KEY_ENTER])
  {
  }
  while (!key[KEY_ENTER])
  {
  }
  blit (cache,buf,0,0,x,y,w,h);
	destroy_bitmap(cache);
	destroy_bitmap(surface);
}

bool
QuestionBox
(char *text, BITMAP* buf, int x, int y, int w, int h, char *title, int _color)
{
  BITMAP *surface = create_bitmap(w,h);
  BITMAP *cache = create_bitmap(w,h);
	int len,c,line=0,pos=0;
  blit (buf,cache,x,y,0,0,w,h);
  clear (surface);
  hline (surface,2,surface->h-2,surface->w-2,makecol (255,255,255));
  vline (surface,2,2,surface->h-2,makecol (255,255,255));
  vline (surface,surface->w-2,2,surface->h-2,makecol (255,255,255));
  for (c=2;c<=2+10;c+=2)
  {
    hline (surface,2,c,surface->w-2,makecol (255,255,255));
  }
  textprintf_centre (surface,mainf,surface->w/2,4,makecol (255,255,255)," %s ",title);
	textprintf_centre (surface,mainf,surface->w/2,surface->h-16,makecol (255,0,0),"Press ENTER (OK)  or  ESC (Cancel)");
	len=(int)strlen (text);
	for (c=0;c<len;c++)
	{
		if (text[c]==10)
		{
			line++;
			pos=0;
		} else
		{
			if (text[c]!=13)
			{
				if (text[c]>126 || text[c]<32 ) break;
				textprintf (surface,mainf,15+pos*8,20+line*10, _color,"%c",text[c]);
				pos++;
			}
		}
	}
  blit (surface,buf,0,0,x,y,w,h);
  while (key[KEY_ENTER] || key[KEY_ESC])
  {
  }
  while ( !(key[KEY_ENTER] || key[KEY_ESC]) )
  {
  }
	if (key[KEY_ENTER]) return true;
	else return false;

  blit (cache,buf,0,0,x,y,w,h);
	destroy_bitmap(cache);
	destroy_bitmap(surface);
}

void
ShowMsgEx
(char *text, BITMAP* buf, int x, int y, int w, int h, char *title, int _color)
{
  BITMAP *surface = create_bitmap(w,h);
  BITMAP *cache = create_bitmap(w,h);
	int len,c,line=0,pos=0;
  blit (buf,cache,x,y,0,0,w,h);
  clear (surface);
  hline (surface,2,surface->h-2,surface->w-2,makecol (255,255,255));
  vline (surface,2,2,surface->h-2,makecol (255,255,255));
  vline (surface,surface->w-2,2,surface->h-2,makecol (255,255,255));
  for (c=2;c<=2+10;c+=2)
  {
    hline (surface,2,c,surface->w-2,makecol (255,255,255));
  }
  textprintf_centre (surface,mainf,surface->w/2,4,makecol (255,255,255)," %s ",title);
	textprintf_centre (surface,mainf,surface->w/2,surface->h-16,makecol (255,0,0),"Press ENTER");
	len=(int)strlen (text);
	for (c=0;c<len;c++)
	{
		if (text[c]==10)
		{
			line++;
			pos=0;
		} else
		{
			if (text[c]!=13)
			{
				if (text[c]>126 || text[c]<32 ) break;
				textprintf (surface,mainf,15+pos*8,20+line*10, _color,"%c",text[c]);
				pos++;
			}
		}
	}
  blit (surface,buf,0,0,x,y,w,h);
  while (key[KEY_ENTER])
  {
  }
  while (!key[KEY_ENTER])
  {
  }
  blit (cache,buf,0,0,x,y,w,h);
	destroy_bitmap(cache);
	destroy_bitmap(surface);
}


class code_editor
{
private:
			char *text_edit[65536];
			int lines;
			int cl, cp;
			int scl, scp;
			int _w,_h;

public:
		  code_editor(void);
			~code_editor(void);
			bool alloc_new_line(void); // ret. true on success
			bool insert_new_line(int ln);
			void delete_last (void);
			void cut_line (int ln);

			void insert_char (int ln, char ch);
			void replace_char (int ln, char ch);
			void backspace (void);
			void del_ch (int ln);
			void draw_cr (void);
			void clear_cr (void);

			void load_source (char *filename);	
			void save_source (char *filename);
			int compile (int *err); // out-num errors, in-table of int's 
															// thet represtents lines with errors
															// (automactly allocated)
			int left,top;
			int tab_size;
			BITMAP *surface;
			void process(void);
			void disp (void);
};
void code_editor::load_source(char *filename)
{
	FILE *strin;
	int c,i,tp,j=0;
	strin=NULL;
	strin=fopen(filename,"rb");
	lines=0;
	if (strin==NULL) ShowMessage("Can't open the file",screen,200,200,200,60,"Message!");
	else 
	{
		//for (c=0;c<65535;c++)
		//{
		//	if (text_edit[c]!=NULL) delete[] text_edit[c];
		//	else break;
		//}

		while (!feof(strin))
		{
			alloc_new_line();
			for (i=0; (i<256) && !feof(strin); i++)
			{
				c=fgetc(strin);
				if (c==9) //TAB
				{
					tp=i/tab_size;
					tp++;
					tp*=8;
					for (i;i<=tp;i++) text_edit[j][i]=32;
					i--;
				} else
				{
					if ((c==10) || (c==13)) 
					{						
						if (c==13) fgetc(strin);
						for (i;i<256;i++)
								text_edit[j][i]=10;
					} else text_edit[j][i]= c==-1?10:c ;
				}
			}
			j++;
		}
		lines=j;
		fclose (strin);
	}
	cp=0; cl=0;
	scp=0; scl=0;
	disp();
}
void code_editor::disp (void)
{
	int i,j,cnti,cntj=0;
	
	for (j=cl-scl;j<cl-scl+_h+1 && cntj<lines;j++)
	{
		cnti=0;
		bool endf=false;

		for (i=cp-scp;i<cp-scp+_w && !endf;i++)
		{
			if (text_edit[j][i]==10 || text_edit[j][i]==0 || text_edit[j][i]==13) endf=true;
			else 
			{ 
				textprintf (surface, mainf,left+cnti*8, top+cntj*10, makecol(255,255,255),"%c",text_edit[j][i]);
			  cnti++;
			}
		}
		for (i;i<cp-scp+_w+1;i++)
		{
	  	textprintf (surface, mainf,left+cnti*8, top+cntj*10, makecol(255,255,255)," ");
			cnti++;
		}
		cntj++;
	}
	if (cntj<_h) rectfill (surface,left,top+cntj*10,surface->w-left,surface->h-left,0);
}
void code_editor::process(void)
{
	BITMAP *cache;
	int c,i;
	bool	exit_ed=false;
	char *filen;
	filen=new char[256];

	cache=create_bitmap(SCREEN_W, SCREEN_H);
	blit (screen, cache, 0,0,0,0,SCREEN_W, SCREEN_H);

	clear (surface);
  hline (surface,2,surface->h-2,surface->w-2,makecol (255,255,255));
  vline (surface,2,2,surface->h-2,makecol (255,255,255));
  vline (surface,surface->w-2,2,surface->h-2,makecol (255,255,255));
  for (c=2;c<=2+10;c+=2)
  {
    hline (surface,2,c,surface->w-2,makecol (255,255,255));
  }
  textprintf_centre (surface,mainf,surface->w/2,4,makecol (255,255,255)," Source editor ");
	lines=0;
	alloc_new_line();
	WORD key_p;
  WORD key_ch;
	while(!exit_ed)
	{
		key_p = readkey();
    key_ch = key_p & 0xff;
    key_p=key_p>>8;
    
    switch (key_p)
    {
		  case KEY_BACKSPACE:
        if (cp>0)
        {
					clear_cr();
					if (scp>0) scp--;
          cp--;
          backspace ();
        } else 
				{
					if (cl>0)
					{
						clear_cr();					
						for (c=0;(c<256) && (text_edit[cl-1][c]!=10);c++)
						{
						}
						cp=c;
						scp=cp;
						if (scp>=_w) scp=_w-1;

						for (c;c<256;c++)
						{														
							text_edit[cl-1][c] = text_edit[cl][c-cp];
							if (text_edit[cl][c-cp]==10) break;            
						}	
						delete[]text_edit[cl];
						for (i=cl;i<lines;i++) text_edit[i]=text_edit[i+1];
						lines--;
						text_edit[lines]=NULL;
						cl--;
						if (scl>0) scl--;	
					}
				}
		  break;
			case KEY_DEL:
        backspace ();
	    break;
			case KEY_ENTER:
				if (cl<65536)
				{
					clear_cr();
					cl++;
					if (scl<_h) scl++;
					if (!insert_new_line(cl)) 
					{
						ShowMessage("Out of memory",surface,200,100,200,60,"Error!");
					} else {
						for (c=cp;c<256;c++)
						{
							text_edit[cl][c-cp]=text_edit[cl-1][c];
							text_edit[cl-1][c]=10;
						}
						cp=0;
				  	scp=0;					
					}

				} else ShowMessage("Source can't has more then 65536 lines of code!!!",surface,200,100,200,60,"Error!");
			break;
			case KEY_END:
				clear_cr();
				for (cp=0;cp<256 && text_edit[cl][cp]!=10;cp++)
				{
				}
				scp=cp;
				if (scp>=_w) scp=_w-1;
			break;
			case KEY_HOME:
				clear_cr();
				cp=0;
				scp=0;
			break;
			case KEY_LEFT:
	  		if (cp>0)
				{
					clear_cr();
					cp--;
					if (scp>0) scp--;
				} else
				{
					if (cl>0)
					{
						clear_cr();
						for (c=0;(c<256) && (text_edit[cl-1][c]!=10);c++)
						{
						}
						cp=c;
						scp=cp;
						if (scp>=_w) scp=_w-1;

						cl--;
						if (scl>0) scl--;
					}
				}
			break;
			case KEY_RIGHT:
				if (cp<256 && text_edit[cl][cp]!=10)
				{
			 		clear_cr();
					cp++;
					if (scp<_w) scp++;
				} else
				{
					if (cl+1<lines)
					{
						clear_cr();
						cp=0;
						scp=cp;
						if (scp>=_w) scp=_w-1;
						cl++;
						if (scl<_h) scl++;						 
					}
				}
			break;
			case KEY_UP:
				if (cl>0)
				{
					clear_cr();
					cl--;
					if (scl>0) scl--;	
					for (c=0;c<256 && text_edit[cl][c]!=10;c++)
					{
					}
					if (c-1<cp) 
					{
						cp=c;
						scp=cp;
						if (scp>=_w) scp=_w-1;
					}
				}
			break;
			case KEY_DOWN:
				if (cl<65536 && cl<(lines-1))
				{ 
					clear_cr();
					cl++;
					if (scl<_h) scl++;
					for (c=0;c<256 && text_edit[cl][c]!=10;c++)
					{
					}
					if (c-1<cp) 
					{
						cp=c;
						scp=cp;
			  		if (scp>=_w) scp=_w-1;
					}
				}
			break;
			case KEY_ESC:
				if (QuestionBox("Are you sure want to EXIT?" ,screen,200,200,400,60,"Confirm!",makecol(255,0,0)))	
				{
					exit_ed=true;				
					for (c=0;c<lines;c++)	
					{
						if (text_edit[c] != NULL) 
						{
							delete[] text_edit[c];		
						}
					}
					lines=0;
				}
				else
				{
					rest (500);
					clear_keybuf();
				}
			break;
			case KEY_F3:
				GetText(filen,screen,100,200,600,60,256," Enter source code file name ");
				for (c=254;filen[c]==' ' || filen[c]=='\r';c--)
				{
			    filen[c]=0;
			  }
				load_source(filen);
			break;
		  default:
				if (key_ch!=0 && cp<256)
				{
					insert_char (cl,key_ch);
					clear_cr();
					cp++;
					if (scp<_w) scp++;
					key_ch=0;
				}
			break;
    }
		if (!exit_ed)
		{
			disp();
			draw_cr();
		}
		textprintf (surface, mainf, 1, 1, makecol (255,255,255)," p%d l%d sp%d sl%d c%d ln%d",cp,cl,scp,scl, text_edit[cl][cp], lines);

		blit (surface,screen,0,0,0,0,surface->w,surface->h);

	}

	blit (cache, screen, 0,0,0,0,SCREEN_W, SCREEN_H);
	destroy_bitmap(cache);
	delete[] filen;
	return;
}
void code_editor::insert_char (int ln, char ch)
{
	int i;
	for (i=256;i>cp;i--)
	{
		text_edit[cl][i]=text_edit[cl][i-1];
	}
	text_edit[cl][cp]=ch;
}
void code_editor::backspace (void)
{
	int i;
	for (i=cp;i<256;i++)
	{
		text_edit[cl][i]=text_edit[cl][i+1];
	}
}
void code_editor::draw_cr(void)
{
	hline (surface,left+8*scp,top+10*scl+9,left+8*scp+8,makecol(255,255,255));
}
void code_editor::clear_cr(void)
{
	hline (surface,left+8*scp,top+10*scl+9,left+8*scp+8,0);
}
code_editor::code_editor(void)
{
	int i;
	for (i=0;i<65536;i++) text_edit[i]=NULL;
	lines=0;
	top=24;
	left=11;
	cl=0; cp=0;
	scl=0; scp=0;
	surface=create_bitmap(SCREEN_W,SCREEN_H-100);
	_w=(surface->w-left*2)/8-1;
	_h=(surface->h-top-left)/10-1;
	tab_size=8;
}
code_editor::~code_editor(void)
{
}
bool code_editor::alloc_new_line(void)
{
	int i;
	if (lines < 65536)
	{
		text_edit[lines]=new char[256];
		for (i=0;i<256;i++) text_edit[lines][i]=10;
		lines++;
		return text_edit[lines]==NULL?false:true;
	}
	return false;
}
bool code_editor::insert_new_line(int ln)
{
	int i;
	if (lines < 65536)
	{
  	for (i=lines;i>ln;i--) text_edit[i]=text_edit[i-1];
		text_edit[ln]=NULL;
  	text_edit[ln]=new char[256];
  	for (i=0;i<256;i++) text_edit[ln][i]=10;
  	lines++;
  	if (text_edit[ln]==NULL)
		{
			for (i=ln;i<lines;i++) text_edit[i]=text_edit[i+1];
			lines--;
			return false;
		} else return true;
	}
	return false;
}
void code_editor::delete_last(void)
{
	lines--;
	delete[] text_edit[lines];
	text_edit[lines]=NULL;
}
void code_editor::cut_line(int ln)
{
	int i;
	if (text_edit[ln]!=NULL)
	{
		for (i=ln;i<lines;i++) text_edit[i]=text_edit[i+1];
		lines--;
	}
}

#endif   // __MARIO_GUI
