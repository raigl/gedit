/*
editlm.c     Line Manager for Editor
*/

#include "edit.h"

#ifdef UNIX
#include <malloc.h>
#else
#include <alloc.h>
#endif

struct linehdr
{       struct linehdr *next, *prev;
	long  lnum;
	int   textlen;
	char *textadr;
};

typedef struct linehdr *lptr;

static lptr curr = NIL;

/*
   Advance the current pointer plus or minus 'cnt' lines.
   Returns FAIL if empty or at either border.
*/
bool advlne(cnt)
register int cnt;
{
	register lptr x;

	x = curr;
	if (x == NIL)
		return(FAIL);

	while ( cnt > 0 )
	{
		if ( x->next == NIL)
		{
			curr = x;
			return(FAIL);
		}
		x = x->next;
		cnt--;
	}
	while ( cnt < 0 )
	{
		if ( x->prev == NIL)
		{
			curr = x;
			return(FAIL);
		}
		x = x->prev;
		cnt++;
	}
	curr = x;
	return(SUCC);
}

/*
    Position to a line by its linenumber.
    If the line does not exist,
	the current line will be the line before.
	(Unless it is before the first,
	then the current line will be the first).
    Fails if the line does not exist,
	but even then changes the current line position, see above.
*/
bool poslne(adr)
long adr;
{
	register lptr x;

	x = curr;
	if (x == NIL)           /* still empty */
	    return(FAIL);
	while ( ( adr < x->lnum ) && ( x->prev != NIL ) )
	    x = x->prev;
	while ( ( adr > x->lnum ) && ( x->next != NIL ) )
	    x = x->next;
	curr = x;
	if ( adr != x->lnum )
	    return(FAIL);
	else
	    return(SUCC);
}

/*
   Replace the text of the current line.
   Fails if no lines or no space for update
*/
bool putlne(text)
char *text;
{
	register lptr y;
	register char *p;

	y = curr;
	if (y == NIL)
	    return(FAIL);

	if ( slen(text) > y->textlen )
	{   /* does not fit, re-allocate */
	    y->textlen = slen(text);
	    /* to avoid exessive fragmentation, round up text length */
	    /* y->textlen = (y->textlen +15) & -16;                  */
	    p = malloc( y->textlen + 1);
	    if (p == 0)
		 return(FAIL);
	    if (y->textadr != NIL)
		 free( y->textadr );
	    y->textadr =  p;
	}
	if (y->textadr != NIL)
	      strcpy( y->textadr, text);
	return(SUCC);
}

/*
   Delete the current line.
   Current position is the line after it,
       unless it is the last one, then it is the previous one.
   Fails if none exits.
*/
bool dellne()
{
	register lptr x, y, z;

	y = curr;
	if (y == NIL)
	    return(FAIL);
	x = y->prev;
	z = y->next;
	if (z != NIL)
	    curr = z;
	else
	    curr = x;
	if (x != NIL)
	   x->next = z;
	if (z != NIL)
	   z->prev = x;
	if (y->textadr != NIL)
	   free( y->textadr);
	free( y );
	return(SUCC);
}

/*
   Insert after the current line.
   The inserted line becomes the current line.
   Fails if no space.
*/
bool inslne(text)
char *text;
{
	register lptr x, y, z;

	x = curr;
	z = NIL;
	if (x != NIL)
	    z = x->next;
	/* create the new lines header */
	y = (lptr) malloc ( sizeof(struct linehdr));
	if (y == 0) /* no space */
	    return(FAIL);
	y->next = z;
	if (z != NIL)
	    z->prev = y;
	y->prev = x;
	if (x != NIL)
	    x->next = y;
	if (x == NIL)
	    y->lnum = makeloc(1,0);
	else
	    y->lnum = x->lnum;    /* will be renumbered later */
	/* textlen is set 0 and putlne used to allocate text buffer */
	y->textlen = 0;
	y->textadr = NIL;
	curr = y;          /* now current line */
	if (putlne(text) != SUCC)
	{       /* no space for text, remove header and return fail */
		dellne();
		return(FAIL);
	}

	/* local renumbering */
	y = curr;
	x = y->prev;
	while ((y != NIL) && (x != NIL) && (y->lnum <= x->lnum))
	{
	      y->lnum = x->lnum + 1;
	      x = y;
	      y = x->next;
	}
	/* the last line in the file is made non-fractional */
	if ((y == NIL) && ( fracloc(x->lnum) != 0 ))
	{
	    x->lnum = makeloc(intloc(x->lnum)+1,0);
	}
	return(SUCC);
}

/*
   Get the current line contents.
   Returns NIL if no lines exist.
*/
char *lnetxt()
{
	if (curr == NIL)
	    return(NIL);
	else
	    return( curr->textadr );
}

/*
   Get the current line number.
   Returns 0 if no lines exist.
*/
long lneadr()
{
	if (curr == NIL)
	    return( 0L );
	else
	    return( curr->lnum );
}


