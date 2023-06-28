/*
editu.c   User interface for Editor
*/

#include "edit.h"

extern char id[];

#ifdef MSDOS
#include <conio.h>
#include <dos.h>
#endif

long adr[MAXLINES];     /* linenumbers of the current lines */
int zeile = 0;          /* index into adr too */
int spalte = 0;
int lmarg = 0;          /* left window margin */
bool chgflg;            /* change result from tget */
bool updflg;            /* raised if any changes in file */
extern bool insflg;     /* raised if in insert mode */
bool drawflg;
char prevkey;

extern bool readonly;          /* raised if file may not be updated */
extern char fname[];    /* name of the current file */
extern bool inplace, readonly; /* main file options */
extern int tabs;

#define FVBUFL 30
char findbuf[FVBUFL+1], varybuf[FVBUFL+1];
char ruler[MAXLINE];    /* Spaltenlineal mit Tabulatoren */

#define MC  0   /* Message Column */

#ifdef MSDOS
#define BAR1 '³'
#define BAR2 'º'
#define BAR3 '°'
#else
#define BAR1 '|'
#define BAR2 '>'
#define BAR3 '"'
#endif

bool varylne();
bool varywrd();
/*
   Print a location, i.e. a linennumber
*/

void uprintf(fmt, val)
char *fmt;
unsigned int val;
{
     char b[80];
     sprintf(b, fmt, val);
     putstr(b);
}

void prtloc(loc)
long loc;
{
	int y;
	
	if (loc < 0)
	    return;

	if (updflg)
	    putstr("*");
	else
	    putstr(" ");
	
	y = intloc(loc);
	uprintf("%5u", y);
	y = fracloc(loc);
	if (y>0)
		uprintf(".%-5u",y);
	else          /*123456*/
		putstr("      ");
	return;
}

/*
   Reshow part of the screen.
   The Parameters are indices into the line number index array.
   Spare lines are not touched.
*/
void reshow(from, to)
int from, to;
{
	int i, j;
	long l;
	register char *p;

	for (i = from; i <= to; ++i)
	{
	     curpos(L1+i, 0);
	     clreoln();
	     l  = adr[i];
	     if (l == 0)
		putsc(BAR3);
	     else if ( (fracloc(l)) != 0)
		putsc(BAR2);
	     else
		putsc(BAR1);
	     putsc(' ');
	     if (l > 0)
	     {
		 poslne(l);
		 p = lnetxt();
		 if (p == NIL)
		     continue;
		 if (strlen(p) < lmarg)
		     continue;
		 p = p + lmarg;
		 if (*p != '\0')
		 {
		     puttxt(p, cols-2, lmarg);
		 }
	     }
	}
}

/*
   Refill the screen, starting at a certain line
   The Parameter is an index into the line number index array.
   Spare Lines are removed.
*/
void refill(from)
int from;
{
	int i;

	/* if the 'from'-line is a spare line, take a real one before */
	while (from>0 && adr[from] <= 0)
	     --from;
	/* fill the array */
	i = from;
	poslne( adr[i]  );
	while (++i <= LL)
	{
	    if  (advlne(1))
		adr[i] = lneadr();
	    else
		adr[i] = 0;
	}
	reshow(from, LL);
}

void strlower(text)
char *text;
{
	int i;
	char c;
	for (i=0; (c=text[i]) != 0; i++)
		text[i] = tolower(c);
}

/*
   Scroll up or down
*/

void down();

void scrll(n)
int n;
{
	poslne( adr[0]  );
	advlne(n);
	adr[0]  = lneadr();
	zeile -= n;
	if (zeile < 0)
		zeile   = 0;
	if (zeile > LL)
		zeile   = LL;
}

void home()
{
	zeile=0;
	spalte=0;
}

void carret()
{
	register i, j;
	register char *txt;

	down();

	spalte = 0;
	i = zeile;
	while (adr[i] == 0)
	    --i;
	poslne( adr[i]  );
	txt = lnetxt();
	if (txt == NIL || strlen(txt) <= lmarg)
		return;
	txt += lmarg;

	/* find first nonblank/tab  in line */
	for (i=0; (*txt == ' ') || (*txt == TAB); i++) {
		if (*txt == TAB && adr[zeile]==0) i+=tabs-1;
		txt++;
		}
	/* the line might contain blanks only, treat as empty */
	if (*txt == '\0')
		i = 0;
	if ( i  < WCOLS )
	    spalte=i;
}


/* block command globals */
long afterloc = 0, fromloc= 0, toloc = 0;
char cmdchar = ' ';

void ExecBlkCmd();

void delcmd(tail)
char *tail;
{
	int cnt;

	if (readonly)
	{
	    error("File is Read-Only");
	    return;
	}
	if (adr[zeile] == 0)
	{
	    error("Not on a dummy line. ");
	    return;
	}
	if (tail[0] != 'd')     /* Number assumed */
	{
		cnt=1;
		sscanf(tail, "%d", &cnt);
		if (cnt < 1) return;
		poslne(adr[zeile]);
		while (cnt-- && (adr[zeile] <= lneadr()))
		    dellne();
		if (zeile == 0)
		{
		    advlne(-1);
		    adr[0] = lneadr();
		}
		else
		    --zeile;
		refill(zeile);
		updflg = TRUE;
		messg(NULL);
	}
	else
	{
		toloc   = fromloc;
		fromloc = adr[zeile];
		cmdchar = 'd';
		ExecBlkCmd();
	}
}

void transcmd();
void up();
void find();

void movecmd(tail)
char *tail;
{
	cmdchar = 'm';
	transcmd(tail);
}


void copycmd(tail)
char *tail;
{
	cmdchar = 'c';
	transcmd(tail);
}


void transcmd(tail)
char *tail;
{
	int cnt;

	if (adr[zeile] == 0)
	{
	    error("Not on a dummy line. ");
	    return;
	}
	toloc = fromloc;
	fromloc = adr[zeile];
	poslne(fromloc);
	if (tail[0] !=  cmdchar)
	{
		cnt =   0;
		sscanf(tail, "%u", &cnt);
		if (cnt > 1)
		    advlne(cnt-1);
		toloc   = lneadr();
	}
	ExecBlkCmd();
}


bool blockdel(from, to)
long from, to;
{
	if ( (from>to)  || !poslne(from))
		return(FALSE);
	/* dellne() positioniert auf die nachfolgende Zeile.
	   Ist es die letzte, auf die Zeile davor.
	   Daher die zus„tzliche Abfrage mit from */
	while ( (lneadr() <= to) && (lneadr() >= from) )
	    if  (!dellne())
		return(FALSE);
	return(TRUE);
}


bool blockvary(from, to)
long from, to;
{
	if (varywrd() != SUCC)
		return FALSE;
	if ( (from>to)  || !poslne(from))
		return FALSE;
	while ( lneadr() <= to)
	{
	    varylne(findbuf, varybuf);
	    advlne(1);
	}
	return TRUE;
}

bool varylne(wrd, repl)
char *wrd, *repl;
{
    int wln, hln;       /* holds the length from "wrd" -1 */
    char *p, *q;        /* hold pointer in current line */
    char *thisline;
    char *strchr();     /* function to find first char in string */
    char tbuf[MAXLINE];

    p = q = thisline = lnetxt();             /* take line */
    if (((wln = slen(wrd)) < 1) || (slen(thisline) < 1))
	return FAIL;
    while ((p = strchr(p, *wrd)) !=  NULL)   /* look first chr */
    {
	if(strlen(p) < wln)
	     return FAIL;                    /* no fit on rest of line */
	if(strncmp(p, wrd, wln) == 0)
	{ /* found it, replace now */
		hln = p - thisline;          /* length of header */
		if (hln > 0)
		    strncpy(tbuf, q, hln);   /* put header */
		q = tbuf + hln;              /* after header */
		strcpy(q, repl);             /* add replacement */
		strcat(q, p+wln);            /* and add tail */
		putlne(tbuf);
		return SUCC;
	}
	p++;
    }
    return FAIL;
}




bool blockcopy(from, to, after)
long from, to, after;
{
	char *txtptr;
	int i;
	long where;

	if ((intloc(from) <= intloc(after)) && (after < to))
	{
		putstr(" Impossible.");
		conin(-1); /*   wait for attention */
		return(FALSE);
	}

	where = after;
	while (from <=  to)
	{
		poslne(from);
		txtptr = lnetxt();
		if (advlne(1))
		    from = lneadr();
		else
		    from = to + 1; /* exit loop next time */
		poslne(where);
		if ( !inslne(txtptr))
		{
		     error("No space, Aborted. ");
		     return(FALSE);
		}
		where   = lneadr();
	}
	return(TRUE);
}

bool reposit(to)
long to;
{
	int i;

	/* if the destination is on the current screen, do not scroll */
	for (i=L1; i<=LL; ++i)
	    if  (adr[i] == to)
	    {
		zeile  = i;
		spalte = 0;
		return (SUCC);
	    };
	/* destination is not on the current screen, go there */
	poslne(to);
	for (zeile=0; (zeile<6) && advlne(-1); ++zeile)
	    ;
	adr[0]  = lneadr();
	spalte  = 0;
	return(FAIL);
}

void ExecBlkCmd()
{
	long xchgloc;

	if (readonly)
	{
	    error("File is Read-Only");
	    return;
	}
	messg(NULL);

	if ( (cmdchar == ' ') && (afterloc == 0) )
		return;

	if (cmdchar !=  ' ') {
		putsc(cmdchar);
		putsc(cmdchar);
		putsc(' ');
		}
	if (fromloc !=  0)
		prtloc(fromloc);

	putsc(' ');

	if (toloc != 0)
		prtloc(toloc);

	if ( (cmdchar != 'd') && (afterloc != 0) )
	{
		putstr(" after ");
		prtloc(afterloc);
	}

	if (toloc == 0)
		return;

	if ( afterloc == 0 && cmdchar != 'd' && cmdchar != 'v')
		return;

	if (toloc < fromloc)
	{
		xchgloc = fromloc;
		fromloc = toloc;
		toloc   = xchgloc;
	}

	putstr(" [busy] ");
	fflush(stdout);

	switch  (cmdchar)
	{
		case 'd':
			blockdel(fromloc, toloc);
			if ((fromloc    <= adr[0]) || (adr[0] <= toloc))
			{ /*    first line on screen deleted */
			    advlne(-1);
			    adr[0] =    lneadr();
			    zeile = 0;
			}
			reposit(fromloc);
			break;
		case 'v':
			blockvary(fromloc, toloc);
			reposit(fromloc);
			break;
		case 'm':
			if (blockcopy(fromloc, toloc, afterloc))
				blockdel(fromloc, toloc);
			reposit(afterloc);
			break;
		case 'c':
			blockcopy(fromloc, toloc, afterloc);
			reposit(afterloc);
			break;
	}
	refill(0);
	cmdchar = ' ';
	toloc = fromloc = afterloc = 0;
	updflg = TRUE;
	messg(NULL);
}

void inscmd(tail)
char *tail;
{
	int i,  cnt;

	if (readonly)
	{
	    error("File is Read-Only");
	    return;
	}
	cnt=1;
	sscanf(tail, "%d", &cnt);
	cnt = min(cnt,  LL-zeile);
	if (cnt < 1)
	    return;
	for ( i = LL; i > zeile; --i)
	    if  ( i-cnt > zeile )
		adr[i] = adr[i-cnt];
	    else
		adr[i] = 0;
	reshow(zeile, LL);
	carret();
}

void opncmd(tail)
char *tail;
{
	int i,  cnt;

	if (zeile==L1)
	{
	    scrll(-1);
	    refill(L1);
	}
	cnt=1;
	sscanf(tail, "%u", &cnt);
	cnt = min(cnt,  LL-L1-zeile);
	if (cnt < 1)
	    return;
	for ( i = LL; i >= zeile; --i)
	    if  ( i < zeile+cnt )
		adr[i] = 0;
	    else
		adr[i] = adr[i-cnt];
	reshow(zeile, LL);
	up();
	carret();
}


void aftercmd()
{
	afterloc = adr[zeile];
	if (afterloc == 0)
	    error("Not on a dummy line. ");
	ExecBlkCmd();
}


void beforecmd()
{
	poslne( adr[zeile]);
	if (!advlne(-1))
	    error("Sorry, cannot do that. ");
	afterloc = lneadr();
	if (afterloc == 0)
	    error("Not on a dummy line. ");
	ExecBlkCmd();
}

long gostack[16];

void gocmd(tail)
char *tail;
{
	unsigned i,j,k;
	long toloc;

	k = sscanf(tail, "%u.%u", &i, &j);
	if (k == 0)
	    return;
	toloc = makeloc(i,0);
	if (k == 2)
	    toloc =makeloc(intloc(toloc),j);
	poslne( toloc );
	adr[0]  = lneadr();
	if (toloc != adr[0])
	{
	     advlne(-1);
	     adr[0] = lneadr();
	}
	zeile = 0;
	refill(0);
}

void here()
{
	register i;

	i = zeile;
	while (adr[i] == 0)
	    --i;
	adr[0]  = adr[i];
	zeile = 0;
	refill(0);
}

void up()
{
	--zeile;
	if (zeile < L1)
		zeile   = LL;
}


void down()
{
	++zeile;
	if (zeile > LL)
		zeile   = L1;
}

void join()
{
	long a1, a2;
	char buf[MAXLINE+1];
	if (zeile == LL)
	    return;
	a1 = adr[zeile];
	a2 = adr[zeile+1];
	if ((a1==0) || (a2==0))
	    return;
	poslne(a1);
	strncpy(buf, lnetxt(), MAXLINE);
	poslne(a2);
	strncat(buf, lnetxt(), MAXLINE-strlen(buf));
	buf[MAXLINE] = '\0';
	dellne();
	poslne(a1);
	putlne(buf);
	refill(zeile);
}

void split()
{
	long a;
	char *p;
	int col;
	a = adr[zeile];
	if (a==0)
	    return;
	poslne(a);
	p = lnetxt();
	col = spalte+lmarg;
	if ((col==0) || (col >= slen(p)))
	    return;
	inslne(&p[col]);
	p[col] = '\0';
	refill(zeile);
}

void namecmd()
{
	int cu, key;
	char buf[FNLEN+1];

	strcpy(buf, fname);
	     /*12345678901234*/
	messg("New Filename: ", buf);
	cu = 0;
	key = tget(buf, FNLEN, CL, MC+14, &cu, 0);
	if (key == ESC)
		return;
	if (cu > 0)
	    buf[cu] = '\0';
	strcpy(fname, buf);
	readonly = (0 == access(fname, 4)) &&
		   (0 != access(fname, 2));
	if (readonly)
	    error("File is Read-Only");
	else
	    messg(NULL);    /* show the new name */
}

bool savecmd()
{
	if (!updflg)
	{
	    messg("Nothing changed, no save. ");
	    return TRUE;
	}
	if (save(fname, makeloc(0,0), makeloc(32767,0), inplace))
	     updflg = FALSE;
	messg(NULL);
	return !updflg;
}

bool yes()
{
	char c, tc;
	while ( (c=conin(-1)) < ' ')
	    ; /* consume control chars  */
	putsc(c);
	tc = conin(-1); /* termination character, must be CR */
	c = tolower(c);
	return  ( tc == CR && ((c=='y') || (c=='j')));
}


void quitcmd()
{
	if (updflg)
	{
	    messg("Abort ? ");
	    if  (!yes() ) {
	        messg(NULL);
		return;
		}
	}
	termscr();
	exit(0);
}


void endcmd()
{
	if (savecmd())
	    quitcmd();
}

char tabmark(i)
int i;
{
	if (0 == (i % 10))
	    return( ((i/10)%10)+'0' );
	if (0 == (i % 5))
	    return(':');
	return( '.' );
}

void showruler()
{
	int i;
	curpos(CL,2);
	for (i=lmarg; i<lmarg+cols-2-1; ++i)
	  if (ruler[i] != 0)
	    putsc(ruler[i]);
}

void tabcmd(tail)
char *tail;
{
	int i;

	if (sscanf(tail, "%u", &i) == 1)
	    if ( (i>0) && (i<MAXLINE) )
		ruler[i-1] = (ruler[i-1]=='*') ? tabmark(i) : '*';
	showruler();
}

void readfile()
{
    int cu, key;
    static char fn[FNLEN+1] = { '\0' };

    if (readonly)
    {
	error("File is Read-Only");
	return;
    }
    if ( adr[zeile] == 0)
    {
	error("Not on a dummy line. ");
	return;
    }
    poslne( adr[zeile] );
	 /*123456789012        */
    messg("Read File: %s ", fn);
    cu = 0;
    key = tget(fn, sizeof(fn)-1, CL, MC+11, &cu, 0);
    if (cu > 0)
	fn[cu] = '\0';
    if (key == CR)
    {
	fetch(fn);
	updflg=TRUE;
	refill(zeile);
	messg(NULL);
    }
    else
	messg(NULL);
}

void findwrd()
{
    int cu, chf, key;

    if ( adr[zeile] == 0)
    {
	error("Not on a dummy line. ");
	return;
    }
	 /*123456*/
    messg("find: ", findbuf);
    cu = 0;
    key = tget(findbuf, sizeof(findbuf)-1, CL, MC+6, &cu, 0);
    if (cu > 0)
	findbuf[cu] = '\0';
    switch(key)
    {
	case Cmd:
	case ESC:
		    break;
	case uSrch:
		    find(findbuf, -1);
		    break;
	case dSrch:
	default:
		    find(findbuf,  1);
		    break;
    }
}

void find(wrd, dir)
char *wrd;
int dir;
{
    int col;

    if(slen(wrd) < 1)
	return;
    if ( adr[zeile] == 0)
    {
	error("Not on a dummy line. ");
	return;
    }
    messg("find: %s [busy] ", wrd);
    col = findit(wrd, dir);
    if( col >= 0 )
    {
	messg(NULL);
	if ( !reposit( lneadr() ))
	   refill(0);
	spalte = col;
	if (spalte > WCOLS)
	{
	    while (spalte > (WCOLS-strlen(wrd)))
	    {
		spalte -=8;
		lmarg += 8;
	    }
	    refill(0);
	}
    }
    else
	messg("find: %s -- Not found. ", wrd);
}


int findit(wrd, dir)
char *wrd;              /* find this word */
int dir;                /* in this direction (up: -1; down +1) */
{
    int wln;            /* holds the length from "wrd" -1 */
    char *p, *q;        /* hold pointer in current line */
    char *strchr();     /* function to find first char in string */

    wln = slen(wrd);
    poslne( adr[zeile] );
    while( advlne(dir) )
    {
	p = q = lnetxt();                        /* take line */
	if (p == NIL)
	    continue;
	while ((p = strchr(p, *wrd)) !=  NULL)   /* look first chr */
	{
	    if(slen(p) >= wln)
		if(strncmp(p, wrd, wln) == 0)
		    return(p-q);              /* found it in col. */
	    ++p;                              /* use next char */
	}
    }
    return(-1);
}

bool varywrd()
{
    int cu, key;
    char tempbuf[FVBUFL+4+1];
    if (strlen(findbuf) <= 0)
    {
	 error("Find first.");
	 return FAIL;
    }
    strcpy(tempbuf, findbuf);
    strcat(tempbuf, " by ");
    strcat(tempbuf, varybuf);
    messg("vary: %s", tempbuf);
    cu = 0;
    key = tget(varybuf, sizeof(varybuf)-1, CL, MC+10+strlen(findbuf), &cu, 0);
    if (cu > 0)
	varybuf[cu] = '\0';
    if (key != CR)
	return FAIL;
    return SUCC;
}

bool varycmd(tail)
char *tail;
{
	int cnt;

	if (readonly)
	{
	    error("File is Read-Only");
	    return FAIL;
	}
	if (adr[zeile] == 0)
	{
	    error("Not on a dummy line. ");
	    return FAIL;
	}
	if (tail[0] != 'v')     /* Number assumed */
	{
		if (varywrd() != SUCC)
			return FAIL;
		cnt=1;
		sscanf(tail, "%u", &cnt);
		messg("vary for %d lines", cnt);
		poslne(adr[zeile]);
		while (cnt--)
		{
		    varylne(findbuf, varybuf);
		    if (!advlne(1)) break;
		}
		refill(zeile);
		updflg = TRUE;
		messg(NULL);
	}
	else
	{
		toloc   = fromloc;
		fromloc = adr[zeile];
		cmdchar = 'v';
		ExecBlkCmd();
	}
	return SUCC;
}

int linecmd()
{
	char c,k,key;
	int cursor;
	char cmdbuf[8+1];
	for (cursor=0;  cursor<=8; cursor++)
		cmdbuf[cursor]='\0';
	key = conin(zeile,0);
	if (iscntrl(key))
	    return(key);
	putsc(key);
	cmdbuf[0]=key;
	cursor=1;

	k = tget(cmdbuf, 8, zeile, 0, &cursor, 0);
	strlower(cmdbuf);
	switch  (cmdbuf[0])
	{
		case ' ':
			cmdchar = ' ';
			afterloc = toloc = fromloc =    0;
			ExecBlkCmd();
			break;
		case '+':
			scrll(LINES-2);
			refill(0);
			break;
		case '-':
			scrll(-LINES+2);
			refill(0);
			break;
		case '*':
			here();
			break;
		case 'i':
			inscmd(&cmdbuf[1]);
			break;
		case 'o':
			opncmd(&cmdbuf[1]);
			break;
		case 'd':
			delcmd(&cmdbuf[1]);
			break;
		case 'm':
			movecmd(&cmdbuf[1]);
			break;
		case 'c':
			copycmd(&cmdbuf[1]);
			break;
		case 'a':
			aftercmd(&cmdbuf[1]);
			break;
		case 'b':
			beforecmd(&cmdbuf[1]);
			break;
		case 's':
			savecmd();
			break;
		case 'q':
			quitcmd();
			break;
		case 'e':
			endcmd();
			break;
		case 'f':
			findwrd();
			break;
		case 't':
			tabcmd(&cmdbuf[1]);
			break;
		case 'r':
			readfile();
			break;
		case 'j':
			join();
			break;
		case 'k':
			split();
			break;
		case 'n':
			namecmd();
			break;
		case 'v':
			varycmd(&cmdbuf[1]);
			break;
		case 'z':
			drawflg = !drawflg;
			break;
		case 0:
			break;
		default:
			gocmd(cmdbuf);
			break;
	}
	return(k);
}

void expand();
void trim();
void exec();

void run(startline)
int startline;
{
	char buff[MAXLINE+1];
	char *badr, *p;
	char ctl;
	int i;

	ctl = 0;
	zeile = 0;
	spalte  = 0;
	poslne(0L);
	updflg = FALSE;
	p = lnetxt();
	/* ensure there is always a tabmark */
	for (i=1; i<MAXLINE+1; ++i)
	    ruler[i-1] = tabmark(i);
/* special arrangement for Nixdorf 8860 assember code *
	if ( (slen(p)>9) && (strncmp(p+9,"START",5) == 0) )
	{
	    ruler[0] = '*';
	    ruler[9] = '*';
	    ruler[15] = '*';
	    ruler[35] = '*';
	    ruler[70] = '*';
	}
	else
*/
	for (i=0; i<MAXLINE; i += tabs)
	    ruler[i] = '*';

	if (startline > 1) {
	    for (i=L1; i<=LL; ++i)
		adr[i] = 0;             /* reposit will seach adr[] */
	    reposit(makeloc(startline,0));
	    }
	else {
	    adr[0]  = lneadr();
	    }
	refill(0);
	if (readonly)
	    error("File is Read-Only");
	else
	    messg(NULL);
	while (TRUE)
	{
	    for (i=zeile; i>0 && adr[i]==0; --i)
		 ;
	    curpos(CL, cols-20);
	    prtloc(adr[i]);
	    if (adr[zeile] == 0)
		putsc('+');
	    else
		putsc(' ');
	    switch (ctl)
	    {
	    case 0:
		if (adr[zeile] != 0)
		{
		    poslne(adr[zeile]);
		    expand(buff, lnetxt(), MAXLINE);
		}
		else
		{
		    expand(buff, " ",   MAXLINE);
		    for (i=0; i < spalte/tabs; ++i)
			buff[i] = TAB;
		    spalte -= i*(tabs-1);
		}
		ctl = tget(&buff[lmarg], WCOLS, zeile, 2, &spalte, lmarg%tabs);
		/* a dummy line left with CR or cursor down is new */
		chgflg |= (adr[zeile]   == 0) && (ctl == CuDn);
		chgflg |= (adr[zeile]   == 0) && (ctl == CR);
		if (!chgflg)
		    break;
		if (readonly)
		{
		    error("File is Read-Only");
		    break;
		}
		updflg = TRUE;
		messg(NULL);
		trim(buff);
		if (adr[zeile] != 0)
		    putlne(buff);
		else
		{
		    i   = zeile;
		    while (adr[i] == 0)
			--i;
		    poslne(adr[i]);
		    inslne(buff);
		    adr[zeile] = lneadr();
		    i   = zeile;
		    while (++i <= LL)
		    {
			if (adr[i] == 0)
			    continue;
			if (intloc(adr[i]) != intloc(adr[zeile]))
			    break;
			advlne(1);
			adr[i] = lneadr();
		    }
		    reshow(zeile, i-1);
		}
		break;
	    case Cmd:
	    case ESC:
		ctl =   linecmd();
		if (ctl == CR)
		    ctl=0;
		else
		{
		    exec(ctl);
		    ctl=Cmd;
		}
		break;
	    default:
		exec(ctl);
		ctl=0;
		break;
	    }
	}
}

/*
   Copy and expand a string to a buffer
*/

void expand(to, from, len)
register char *to, *from;
register int len;
{
	register char c;
	if (from != NIL)
	    while ( ((c = *from++) != 0) && len--)
		*to++ = c;
	while ( len-- )
	    *to++ = ' ';
	*to++ = '\0';
}

/*  remove trailing blanks */
void trim(str)
char *str;
{
	register char *p;
	register int l;

	p = str;
	l = slen(p);
	if (l <= 0)
		return;
	p += l; /* behind */
	while (l-- && *(--p) == ' ')
		;
	*++p = '\0';
}

/*
   Execute a function key
*/
void exec(ctl)
char ctl;
{
	switch  (ctl)
	{
	    case 0:
		 break;
	    case CuUp:
		 up();
		 break;
	    case CuDn:
		 down();
		 break;
	    case CuHm:
		 home();
		 break;
	    case PgUp:
		 scrll(-LINES+2);
		 refill(0);
		 break;
	    case PgDn:
		 scrll(LINES-2);
		 refill(0);
		 break;
	    case PlUp:
		 scrll(-LINES/2);
		 refill(0);
		 break;
	    case PlDn:
		 scrll(LINES/2);
		 refill(0);
		 break;
	    case PgHic:
		 here();
		 break;
	    case uSrch:
		 find(findbuf, -1);
		 break;
	    case dSrch:
		 find(findbuf, 1);
		 break;
	    case Wlft:
		 lmarg += 16;
		 reshow(L1, LL);
		 messg(NULL);
		 break;
	    case Wrght:
		 lmarg = max(0, lmarg-16);
		 reshow(L1, LL);
		 messg(NULL);
		 break;
	    case CR:
		 carret();
		 break;
	    case Cancel:
		 quitcmd();
		 break;
	    case Done:
		 endcmd();
		 break;
	    case Cmd:
	    case ESC:
		 break;
	    case Save:
	    	 savecmd();
		 break;
	    case Find:
		 findwrd();
		 break;
	    case Col1:
		 spalte=0;
		 fprintf(stderr, "Col1");
		 break;
	    default:
		 spalte=0;
		 carret();
		}
}

/*
   Print a message.
*/
void messg(fmt, str)
char *fmt;
void *str;
{
	curpos(CL, MC);
	clreoln();
	showruler(); /* positions cursor itself to the left */
	curpos(CL,cols-1-strlen(id)-2-strlen(fname)-1-1-5-12-1);
	putstr(" ");
	putstr(id);
	putstr(" [");
	putstr(fname);
	putstr("]");
	if (insflg)
	    putstr(" INS ");
	else
	    putstr(" OVR ");
	      /*32767.32737+*/
	putstr("            ");
	curpos(CL, MC);
	if (fmt != NULL)
	    scprintf(fmt, str);
#ifdef UNIX
	fflush(stdout);
#endif
}

void info(fmt, str)
char *fmt;
void *str;
{
	int x,y;
	getyx(stdscr, y, x); /* macro, thus no & */
	messg(fmt, str);
	curpos(y, x);
}

void error(txt)
char *txt;
{
	/* beep */
	messg(txt, NULL);
}
