/*
edits.c  Screen manager for Editor
*/

#include "edit.h"

#ifdef MSDOS
#include <conio.h>
#include <dos.h>
#endif

extern bool chgflg;     /* change result from tget */
extern char ruler[];    /* Spaltenlineal mit Tabulatoren */
extern int oldpage;
extern int thispage;
extern int tabs;
#ifdef MSDOS
int insflg = 0;         /* Start in overwrite mode */
#else
int insflg = 1;         /* Start in insert    mode */
#endif

#ifdef CURSES
void putnstr(x, l)
char *x;
int l;
{
      addnstr(x, l);
}
#endif
void putstr(x)
char *x;
{
	while (*x)
	   putsc(*x++);
}

/*
   Put a string onto screen, control chars inverted
*/
void puttxt(p, maxlen, taboff)
char *p;
int maxlen, taboff;
{
	int i;
	char *q;
	
	if (p == NIL)
	    return;
	/*TODO: treat unicode correctly */
	i = strlen(p);
	if (i > maxlen)
	    i = maxlen;
	/* ohne nachfolgende Leerzeichen */
	while ( (i-- >  0 ) && (p[i] == ' ')  )
	     ;
	i++; /* anzahl  zeichen */
	/* ein Leerzeichen hinten dran lassen, damit DelC funkt. */
	if (p[i] != '\0')
	     i = min(i+1, maxlen);
	/* curses displays unicode if given as string, not as characters */
	q = p;
	while ( i-- > 0 )
	    if (*p != TAB) {
	 	++p;
		++taboff;
	    } else {
	    	if (p != q)
		    putnstr(q, p-q);
		do putsc(' ');
		while ((++taboff % tabs) != 0);
		++p;
		q = p;
	    }
	if (p != q)
		putnstr(q, p-q);
}

#ifdef IBMBIOS

char funkey(c)
char c;
{
	switch ( (int) c & 0xff)
	{
			 case  15: return(BTAB);
			 case  60: return(ScChg);
			 case  61: return(Wrght);
			 case  62: return(Wlft);
			 case  63: return(PgHic);
			 case  64: return(PrtSc);
			 case  65: return(uSrch);
			 case  66: return(PlUp);
			 case  67: return(dSrch);
			 case  68: return(PlDn);
			 case  71: return(CuHm);
			 case  72: return(CuUp);
			 case  73: return(PgUp);
			 case  75: return(CuL);
			 case  77: return(CuR);
			 case  79: return(EoLn);
			 case  80: return(CuDn);
			 case  81: return(PgDn);
			 case  82: return(INS);
			 case  83: return(DelC);
			 case 115: return(Wrght);
			 case 116: return(Wlft);
			 case 126: return('\\');
			 case 127: return('{');
			 case 128: return('}');
	 default: return('\0');
	}
}

int  conin(zeile, spalte)
int zeile, spalte;
{
	int c;

	if (zeile != -1)
	    curpos(zeile, spalte);
	c=getch();
	if (c == 0)
	    c=funkey(getch());
	if (c != ESC)
	    return(c);
	if (zeile != -1)
	    curpos(zeile, 0);
	return(ESC);
}
#else  /* not IBMBIOS */

#ifdef CURSES
char funkey(c)
int c;
{
	switch (c) {
	case KEY_DOWN : c=CuDn; break;
	case KEY_UP   : c=CuUp; break;
	case KEY_LEFT : c=CuL ; break;
	case KEY_RIGHT: c=CuR ; break;
	case KEY_NPAGE: c=PgDn; break;
	case KEY_PPAGE: c=PgUp; break;
	case KEY_HOME : c=CuHm; break;
#ifdef KEY_END
	case KEY_END  : c=EoLn; break;
#endif
#ifdef KEY_CANCEL
	case KEY_CANCEL : c=Cancel; break;
#endif
#ifdef KEY_COMMAND
	case KEY_COMMAND : c=Cmd; break;
#endif
	case KEY_IC   : c=INS ; break;
	case KEY_DC   : c=DelC; break;
	case KEY_BACKSPACE:c=BS;break;
	case KEY_F(1) : c=Save ; break;
	case KEY_F(2) : c=Cmd ; break;
	case KEY_F(3) : c=Wlft; break;
	case KEY_F(4) : c=Wrght; break;
	case KEY_F(5) : c=uSrch; break;
	case KEY_F(6) : c=dSrch; break;
	case KEY_F(7) : c=PlDn; break;
	case KEY_F(8) : c=PlUp; break;
	case KEY_F(9) : c=PrtSc; break;
/*	case KEY_F(10): c=PlDn; break; */
/*	case KEY_F(11): c=ScChg; break;  */
	case KEY_F(12): c=PgHic ; break;
	case KEY_B2   : c=PgHic; break;
	case KEY_C1   : c=EoLn; break;
	}
	return c< 0400 ? c : -1;
}
#else  /* not CURSES */

#ifdef LINUX
char funkey(c)
int  c;
{
	switch (c) {
	case '[':
	     c = getchar();
	     switch (c) {
	     case '[':
		  c = getchar();
		  switch( c ) {
		       case 'A': return(CuUp);
		       case 'B': return(CuDn);
		       case 'C': return(Wlft);
		       case 'D': return(Wrght);
		       case 'E': return(PgHic);
		       default: return 0;
		       }
	     case '1':
		  c = getchar();
		  switch (c) {
		  case '7': c = PrtSc; break;
		  case '8': c = uSrch; break;
		  case '9': c = PlUp;  break;
		  default: return 0;
		  }
		  if ( getchar() != '~') return 0;
		  return c;
	     case '2':
		  c = getchar();
		  switch (c) {
		  case '0': c = dSrch; break;
		  case '1': c = PlDn;  break;
		  default: return 0;
		  }
		  if ( getchar() != '~') return 0;
		  return c;
	    default:
		  return 0;
	    }
	case 'O':
	     switch( c=getchar() )
		   {
		       case 'x': return(CuUp);
		       case 'r': return(CuDn);
		       case 'v': return(CuR);
		       case 't': return(CuL);
		       case 'w': return(CuHm);
		       case 'q': return(EoLn);
		       case 'y': return(PgUp);
		       case 's': return(PgDn);
		       case 'p': return(INS);
		       case 'n': return(DelC);
		       case 'u': return(PgHic);
		       default:  return(0);
		   }
	default:
		   return(c);
	}
}
#else  /* not LINUX */

char funkey(c)
int  c;
{
	switch (c)
	{
	    case '[':
		   c = getchar();
		   switch( c )
		   {
		       case 'A': return(CuUp);
		       case 'B': return(CuDn);
		       case 'C': return(CuR);
		       case 'D': return(CuL);
		       case 'H': return(CuHm);
		       case 'K': return(EoLn);
		       case 'U': return(PgUp);
		       case 'V': return(PgDn);
		       case '@': return(INS);
		       default: return(0);
		   }
	    case 'O':
		   switch( c=getchar() )
		   {
#ifdef MICROPORT
		 /*    case 'c': return(BTAB);  F1
		       case 'd': return(PgUp);  F2      */
		       case 'e': return(Wlft);
		       case 'f': return(Wrght);
		       case 'g': return(PgHic);
		       case 'h': return(PrtSc);
		       case 'i': return(uSrch);
		       case 'k': return(dSrch);
		       case 'j': return(PlUp);
		       case 'l': return(PlDn);
#endif /* MICROPORT */
		       default: printf(" %c", c); fflush(stdout);  return(0);
		   }
	    default:
		   return(c);
	}
}
#endif /* LINUX */

#endif /* CURSES */

#ifdef  MICROPORT
#define empty(p) ((p)->_cnt == 0)
#endif

#ifdef  LINUX
#define empty(p) ((p)->_gptr >= (p)->_egptr)
#endif

#ifndef empty
#define empty(p) (0)
#endif

int  conin(zeile, spalte)
int zeile, spalte;
{
	int  c;

	if (zeile != -1)
	    curpos(zeile, spalte);
#ifdef CURSES
	refresh();
	c=getch();
	return funkey(c);
#else
	c=getchar();
	if (c != ESC)
	    return(c);
	if (empty(stdin))
	{
	    if (zeile == -1)
		return(ESC);
	    else
		curpos(zeile, 0);
	}
	c=getchar();
	c=funkey(c);
	if (iscntrl(c) && (c!=ESC))
	    return(c);
	else
	{
	    ungetc(c, stdin);
	    return(ESC);
	}
#endif /* CURSES */
}

#endif /* IBMBIOS */

/* Calculate length with tab expansion  */

int tabbedlen(text, cursor, taboff)
char *text;
int cursor, taboff;
{       int res = 0;
	int i;
	for (i=0; i<cursor; ++i) {
	    ++res;
	    if (text[i] == TAB) {
		while (((res+taboff) % tabs) != 0)
		   ++res;
	    }
	}
	return res;
}


/*
   Request to update a field on the screen.
   Returns if a control char not for field edit is received.
   At end of field, CR is generated.
   The field must not be longer than MAXLINE characters.
*/
char tget(text, len, row, col, cursor, taboff)
char *text;
int len, row, col, taboff;
int *cursor;
{
	int i, tlen;
	int c;
	char sav[MAXLINE+1];
	strncpy(sav, text, MAXLINE);
	sav[MAXLINE]='\0';
	chgflg  = FALSE;
	while (1)
	{
		c=conin(row, col + tabbedlen(text, *cursor, taboff));
		switch (c)
		{
			default:
				if ( (c>0) && (c<32) && (c!=TAB)) /* ignore contr.*/
					break;
				if (insflg)
				{
				    chgflg = TRUE;
				    for (i=strlen(text);--i>*cursor;)
					text[i]=text[i-1];
				    text[*cursor]=c;
				    puttxt(&text[*cursor],
					   cols-(col+*cursor),
					   taboff+tabbedlen(text, *cursor, taboff));
				}
				else
				{
				   putsc(c);
				   chgflg |= (text[*cursor] != c);
				   text[*cursor]=c;
				}
			case    CuR:
			right:
				if (++(*cursor) >= len)
				{
					*cursor=0;
					return(CuDn);
				}
				break;
			case    EoLn:
				tlen = min(strlen(text) - 1,
					   len);
				for(i = 0; i<tlen; i++)
				     if (text[i] != ' ') *cursor = i;
				goto right;
			case    BS:
				if (--(*cursor) < 0)
					*cursor = 0;
				else
				{
				/*  putsc('\b');      */
				    curpos(row, col + tabbedlen(text, *cursor, taboff));
				    if (insflg)
					goto delete;
				    c = sav[*cursor];
				    putsc(c);
				    text[*cursor] = c;
				}
				break;
			case    CuL:
				if (--(*cursor) < 0)
				{
					*cursor=len-1;
					return(CuUp);
				}
				break;
			case    INS:
				insflg= !insflg;
				messg(NULL);
				break;
			case    DelC:
			case    DEL:
			delete:
				chgflg = TRUE;
				tlen = strlen(text) - 1;
				for(i = *cursor; i<tlen; i++)
						text[i]=text[i+1];
				text[tlen] = ' ';
				clreoln();
				puttxt(&text[*cursor],
				       cols-(col+*cursor),
				       taboff+tabbedlen(text, *cursor, taboff));
				break;
#ifdef EXPAND_TABS
			case    TAB:
				do
				{
					if      ((*cursor)>=len)
					{
						*cursor=0;
						return(CuDn);
					}
				}
				while (ruler[++(*cursor)] != '*');
				break;
#endif
			case    BTAB:
				do
				{
				    if ((*cursor)<0)
				    {
					*cursor=len-1;
					return(CuUp);
				    }
				}
				while (ruler[--(*cursor)] != '*');
				break;
			case    PrtSc:
				c = 0x1F & conin(-1);
				invers(c + '@');
				chgflg |= (text[*cursor] !=     c);
				text[*cursor] = c;
				if (++(*cursor) >= len)
				{
					*cursor=0;
					return(CuDn);
				}
				break;
			case    ScChg:
#ifdef IBMBIOS
				newpage(oldpage);
				while (ScChg != conin(-1))
				    ;
				newpage(thispage);
#else
				reshow(L1, LL);
#endif
				break;
			case    CuUp:
			case    CuDn:
			case    CuHm:
			case    PgUp:
			case    PgDn:
			case    PgHic:
			case    PlUp:
			case    PlDn:
			case    Wlft:
			case    Wrght:
			case    uSrch:
			case    dSrch:
			case    Cmd:
			case    Cancel:
			case    Done:
			case 	Save:
			case    Find:
			case    ESC:
			case    CR:
				return(c);
			/*      break;  unreached code, compiler complain */
			case    0xff:
			case    -1:
				break;
		}
	}
}

