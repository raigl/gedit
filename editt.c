#include "edit.h"

#ifdef IBMBIOS

#include <dos.h>
#define int16(fun) regs.h.ah=fun; int86(16, &regs, &regs)

static union REGS regs;
int    COLS = 80, LINES = 25;   /* use CURSES names */
static char scrattr = 7;
static page = 1;
static oldrow=0, oldcol=0;
static mda = 0;
int thispage = 1;
int oldpage = 0;

void curpos();
void clrscrn();
void newpage();

void beginscr()
{
	cols = peek(0, 0x44a);
	rows = peekb(0, 0x484) + 1;
	rows = min(rows, MAXLINES); /* we have an array[MAXLINES] */

	int16(15);  /* get current page and video mode */
	oldpage = regs.h.bh;
	if (regs.h.al == 7) { /* monochrome card, has only 1 page */
	   mda = 1;
	   thispage = 0;
	   }

	int16(3);   /* get cursor position on BH-page */
	oldrow=regs.h.dh;
	oldcol=regs.h.dl;

	curpos(0,0);
	regs.h.bh = oldpage;
	int16(8);  /* get char attribute */
	scrattr = regs.h.ah;
	if (scrattr == 0) scrattr = 0x71; /* blue on white */

	newpage(thispage);
	curpos(0,0);
	clrscrn();
}

void termscr()
{
	newpage(oldpage);
	if (mda)
	    curpos(24, 0);
	else
	    curpos(oldrow, oldcol);
}


void newpage(other)
int other;
{

	regs.h.al = other;
	int16(5);       /* set page */
}

void curpos(row, col)
int row, col;
{
	regs.h.bh = thispage;
	regs.h.dh = row;
	regs.h.dl = col;
	int16(2);
}

void clrscrn()
{
	curpos(0,0);
	regs.h.bh = thispage;
	regs.x.cx = rows*cols;
	regs.h.al = ' ';
	regs.h.bl = scrattr;
	int16(9);
}

void clreoln()
{
	int16(3);      /* cursorspalte -> dl */
	regs.h.bh = thispage;
	regs.x.cx = cols - regs.h.dl;
	regs.h.bl = scrattr;
	regs.h.al = ' ';
	int16(9);
}

void putsc(c)
char c;
{
	regs.h.al = c;
	regs.h.bl = scrattr;
	int16(14);
}

static void attrib(a)
int a;
{
	regs.h.bh = thispage;
	regs.h.al = ' ';
	regs.h.bl = a;
	regs.x.cx = 1;
}



void invers(c)
char c;
{
	attrib(7*16);
	int16(9);
	regs.h.al = c;
	regs.h.bl = ((scrattr & 0x07) << 4) + ((scrattr & 0x70) >> 4);
	int16(14);
}

void scprintf(fmt, str)
char *fmt, *str;
{
      char buff[80];
      sprintf(buff, fmt, str);
      putstr(buff);

}

#endif /* IBMBIOS */

#ifdef CURSES

/*  most functions are defined as macro replacements in edit.h */

void beginscr()
{
	initscr();
	raw();
	nonl();
	noecho();
	leaveok(stdscr, FALSE);
	keypad(stdscr, TRUE);
}



void invers(c)
char c;
{
	standout();
	addch(c);
	standend();
}

void putsc(c)
char c;
{
	if (iscntrl(c))
	    invers(c+'A'-1);
	else
	   if (isprint(c))
	      addch( (chtype) c & 0xff);
	   else
	      invers('?');
}


#endif /* CURSES */

#ifdef VT100

beginscr()
{
	rows = 24;
	clrscrn();
	raw_mode(1);

	printf("\033="); /* application keypad mode */
}

termscr()
{
	raw_mode(0);
	curpos(rows-1, 0);
	printf("\033>");  /* numeric keypad */
	putchar('\n');
}

curpos(row, col)
int row, col;
{
	printf("%c[%d;%dH", esc, row+1, col+1);
}

clrscrn()
{
	printf("%c[2J", esc);
}

clreoln()
{
	printf("%c[K", esc);
}

putsc(c)
char c;
{
	putchar(c);
}

invers(c)
char c;
{
	printf("%c[7m%c%c[0m", esc, c, esc);
}


#ifdef UNIX
#include <termio.h>

static raw_mode(on)
bool on;
{
	static struct termio old;
	struct termio act;
	ioctl(0, TCGETA, &act);
	if (on)
	{
	     old = act;
	     act.c_lflag &= ~ISIG;  /* unless we catch the signal */
	     act.c_lflag &= ~ICANON;
	     act.c_lflag &= ~ECHO;
	     act.c_iflag &= ~ICRNL;
	     act.c_iflag &= ~IGNCR;
	     act.c_cc[VMIN] = 3; /* MIN */
	     act.c_cc[VTIME] = 1; /* TIME */
	}
	else
	     act = old;
	ioctl(0,TCSETA, &act);
}
#else

static void raw_mode(on)
{
}

#endif /* UNIX for raw_mode */
#endif /* VT100 */
