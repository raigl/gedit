/*
      Header file for SPF like Editor
*/

#ifdef CURSES
#include <ncurses.h>
#else
#include <stdio.h>
#include <ctype.h>
#endif

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#endif

#ifdef __TURBOC__
#define MSDOS
#endif

#ifdef LINUX
#define UNIX
#endif

#ifdef MICROPORT
#define UNIX
#define HAS_NO_RENAME
#endif

#ifdef TARGON31
#define UNIX
#define HAS_NO_RENAME
#endif

#ifdef XENIX
#define UNIX
#define HAS_NO_RENAME
#endif

/* General definitions */
#define SUCC 1
#define FAIL 0

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef bool
#define bool int
#endif

typedef unsigned int word;
typedef unsigned char byte;

#ifdef NULL
#define NIL NULL
#else
#define NIL 0
#endif

#ifndef min
#define min(x,y) ((x)<(y) ? (x) : (y) )
#define max(x,y) ((x)>(y) ? (x) : (y) )
#endif

#ifndef iscntrl
#define iscntrl(x)  ( ((x) < ' ') && ((x) >= 0))
#endif

#define slen(sptr) ( (sptr) != NULL ? strlen(sptr) : 0 )

/* Line Manager interface */
/*
   Line Manager interface

   Line numbers are kept in fractional form,
   where the original line number will be the integral part
   and any inserted material receives fractions.
   Both are packed into a long integer using a scale factor,
   normally max short int.
*/
#ifdef MICROPORT
#define LONG_MAX 0x7FFFFFFF
#else
#include <limits.h>
#endif
#define LSCALE 32768L
#define makeloc(i,f) ( min((long)i, LONG_MAX/LSCALE)*LSCALE+(f))
#define fracloc(l) ((l) % LSCALE)
#define intloc(l) ((l) / LSCALE)
extern bool advlne();
extern bool poslne();
extern bool putlne();
extern bool dellne();
extern bool inslne();
extern long lneadr();
extern char *lnetxt();

/*
   File manager interface
*/

#define FNLEN 63        /* max characters in a filename */
#define MAXLINE 4096    /* max characters per line */

extern bool save();
extern bool fetch();



/*
	Screen  handling.

    All Funktion Keys are mapped into a single Control Character.

*/


#define Cmd   0x01      /* ^A */
#define Col1  0x02	    /* ^B */
#define DelC  0x03      /* ^C */
#define Done  0x04      /* ^D */
#define uSrch 0x05	    /* ^E */
#define Find  0x06      /* ^F */
#define dSrch 0x07 	    /* ^G */
#define BS    0x08      /* ^H */
#define TAB   '\t'      /* ^I */
#define LF    '\n'      /* ^J */
#define PlDn  0x0B      /* ^K */
#define PlUp  0x0C      /* ^L */
#define CR    0x0D      /* ^M */
			/* ^N */
#define EoLn  0x0F      /* ^O */
#define PrtSc 0x10      /* ^P */
#define CuHm  0x11      /* ^Q */
#define PgHic 0x12      /* ^R */
#define Save  0x13      /* ^S */
#define BTAB  0x14      /* ^T */
#define PgUp  0x15      /* ^U */
#define PgDn  0x16      /* ^V */
#define INS   0x17      /* ^W */
#define Cancel 0x18     /* ^X */
#define Wlft  0x19      /* ^Y */
#define Wrght 0x1A      /* ^Z */
#define ESC   0x1B      /* ^[ */
#define CuR   0x1C      /* ^\ */
#define CuL   0x1D      /* ^] */
#define CuUp  0x1E      /* ^^ */
#define CuDn  0x1F      /* ^_ */

#define DEL   0x07F

#define MAXLINES 256    /* longer screens are not fully used */

#ifndef CURSES
extern int COLS, LINES; /* similar to CURSES, well. */
#endif

#define cols COLS
#define rows LINES

#define WCOLS (COLS-2)  /* number of columns in the edit window */
#define L1 0            /* first row for data */
#define LL (L1+LINES-2) /* Last row for data */
#define CL (L1+LINES-1) /* Command line on screen */

extern char funkey();
extern char tget();
extern int  conin();
extern void messg();
extern void error();
extern void info();

/* VT100 is default, undef if not to be used */
#define VT100

#ifdef CURSES
#undef VT100
#define termscr endwin
#define clrscrn clear
#define clreoln clrtoeol
#define curpos  move
#define scprintf printw
#endif

#ifdef IBMBIOS
#undef VT100
#endif

#ifdef VT100
#define scprintf printf
#endif
