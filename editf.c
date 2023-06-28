/*
editf.c   File manager for Editor
// vi ts=4
*/

#include "edit.h"

#ifdef MSDOS
#include <io.h>
#endif

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

extern int tabs;    /* tab stops */
extern bool untab;
extern char ruler[]; 

FILE *infile, *outfile, *fopen();

/*
   Append the text as a line to the output file.
*/
static writeln(text)
char *text;
{
	register i;
	int len, ind;
	char c, *p;
	len = slen(text);
	ind = 0;
	p = text;
#ifdef SPACES_TO_TABS
	/* replace leading spaces by tabs */
	/* length without trailing blanks */
	p = text + len;
	while ( len > 0 && *(--p)==' ')
		--len;
	if (len <= 0)
	{
	    putc('\n', outfile);
	    if (ferror(outfile))
		return FAIL;
	    else
		return SUCC;
	}
	/* number of leading spaces */
	p = text;
	ind = 0;
	while ( ind < len && *(p++) == ' ')
	     ++ind;

	/* put leading tabs */
	for (i = ind / tabs; i > 0; --i)
	     putc('\t', outfile);

	/* put remaining leading blanks */
	for (i = ind % tabs; i > 0; --i)
	     putc(' ', outfile);
#else
	if (untab) {
	    /* output leading spaces and replace tabs by spaces*/
	    p = text;
	    ind = 0;
	    while (p < text+len && (*p == '\t' || *p == ' ')) {
		if (*p == '\t')  {
		    do	{
			putc(' ', outfile);
		    }
		    while (ruler[++ind] != '*');
		} else 
		    putc(' ', outfile);
		++p;	    
	    }
	    ind = p - text;
	}
#endif
	/* now put rest of line */
	for (p = text+ind, i = len - ind; i > 0; --i)
	     putc(*p++, outfile);
	putc('\n', outfile);
	if (ferror(outfile))
	    return FAIL;
	else
	    return SUCC;
}

check_untab(p)
char *p;
{    /* if the line starts with a blank and is not entirely blank,
	set untab option */
	if (*p == ' ') 
	    if (strspn(p, " ") != strlen(p))
		untab = 1;
}
/*
   Get the next line from the input file to the buffer.
   If the buffer size is exceeded, it is not truncated,
   but continued on the next line.
   Returns:
     EOF if end of file,
     \n  if line fitted into buffer,
     \0  if line is to be continued.
*/
static int readln(lne, len)
char *lne;
int len;
{
	register int col;
	register int  c;
	register char *p;

	col = 1;
	p = lne;
	while ( col <= len)
	{
	    c = getc(infile);
	    switch(c)
	    {
		case '\n':
		case EOF:
		     *p = '\0';
		     check_untab(lne);
		     if (col==1) return(c);
		     return '\n';
		/* replace tabs in input file by spaces 
		case '\t':
			if (untab) {
				do
			 		*(p++) = ' ';
		     	while ((col++) % tabs);
			    break;
			}
		*/
		default:
		     *(p++) = c;
		     ++col;
		     break;
	    }
	}
	*p = '\0';
	check_untab(lne);
	return( *p );
}

/*
   Save a block of lines to a file.
*/
bool save(fnam, from, to, inplace)
char *fnam;
long from, to;
bool inplace;
{
	int i;
	long this;
	int statflg;
	char bnam[FNLEN+1];   /* backup name */

#ifdef UNIX
	struct stat statbuf;
#endif

	if (to < from)
		return(FAIL);   /* illegal call */

	statflg = 0;
	if (!inplace)
		statflg = mkback(fnam, bnam);
	if (statflg == -1)
		return FAIL;

	if ((0 == access(fnam, 4)) && (0 != access(fnam, 2)))
	{
		error("Write protected.");
		return FAIL;
	}

	outfile=fopen(fnam, "w");
	if (outfile==0)
	{
		error("Open error");
		return(FAIL);
	}

	poslne(from);
	while (lneadr() <= to)
	{
		this = lneadr();
		if ( (this == 0L) || (this > to) )
		    break;
		if (!writeln(lnetxt()))
		{
		    error("Write error");
		    return FAIL;
		}
		if (!advlne(1))
		    break;
	}

	if (fclose(outfile) !=0)
	{
		error("Close error");
		return FAIL;
	}
#ifdef UNIX
	/* if we can get the status of a backup file, propagate it */
	if (statflg && (0 == stat(bnam, &statbuf)))
	{
	    chmod(fnam, statbuf.st_mode);
	    chown(fnam, statbuf.st_uid, statbuf.st_gid);
	}
#endif
	return(SUCC);
}

/*
   calculate the name of the backup-file,
   if the output file exists, try to remove it
   and rename the old to the backup file.
*/

static void backname();

int mkback(fnam, bnam)
char *fnam, *bnam;
{
	backname(fnam, bnam);
	if (0 != access(fnam, 4)) /* read access allowed ? */
	    return 0;             /* no, assume new file, backup not needed */
	unlink(bnam);             /* ignore any error */
	if (0 == rename(fnam, bnam) )
	    return 1;             /* all ok, use backup file for attributes */
	error("Cannot backup, write in-place?");
	if ( !yes() )
	     return -1; /* abort */
	return 1;
}

static char
#ifdef UNIX
	    backfix[] = "~";
#else
	    backfix[] = "$";
#endif
static void backname(fnam, bnam)
char *fnam, *bnam;
{
#ifdef VMS
	strcpy(bnam, fnam); /* VMS has version numbers, keep same name */
#else
	char c;
	int bi, fi;

	strcpy(bnam, fnam);             /* use original name */
#ifdef MSDOS
	for (bi = fi = 0; (c=fnam[fi]) != 0; fi++)
	{
	    if ( c == '/' || c == '\\' || c == ':')
		bi = fi+1;
	}
	bnam[bi] = '\0';                     /* stop after last slash */
	strncat(bnam, backfix, FNLEN-strlen(bnam));
	bnam[FNLEN] = '\0';
	strncat(bnam, &fnam[bi], FNLEN-strlen(bnam));
	bnam[FNLEN] = '\0';
#else
	strncat(bnam, backfix, FNLEN-strlen(bnam));
	bnam[FNLEN] = '\0';
#endif
#endif
}

#ifdef CPM68k

struct drfnft {         /* drive, filename and filetype */
	BYTE drive;
	char fname[8];
	char ftype[3];
	};


static parsefn(input, output)
char *input;
struct drfnft *output;
{
	int i;


	output->drive = 0;      /* default drive */
	if (input[1] == ':')
	{
		output->drive = toupper(input[0]) - 'A' + 1;
		input += 2;
	}
	for (i=0; i<8; ++i)     /* extract filename proper */
	{
		if ((*input != '\0') && (*input != '.'))
		{
		output->fname[i] = toupper(*input);
		input++;
		}
		else
		output->fname[i] = ' ';
	}


	if (*input == '.')       /* skip dot separator */
		++input;


	for (i=0; i<3; ++i)
	{
		if (*input != '\0')
		{
		output->ftype[i] = toupper(*input);
		input++;
		}
		else
		output->ftype[i] = ' ';
	}
}




struct FCB {
	struct drfnft oldname;
	BYTE extnt;
	BYTE s1,s2,rc;
	struct drfnft newname;
	BYTE nx[4];
	BYTE cr,r0,r1,r2;
	};


static rename(oldn, newn)
char *oldn, *newn;
{
	struct FCB fcb;


	parsefn(oldn, &(fcb.oldname));
	parsefn(newn, &(fcb.newname));


	if (bdos(23, &fcb) != 0)
	return(-1);


	return(0);
}
#endif

#ifdef  HAS_NO_RENAME
int rename(oldfn, newfn)
char *oldfn, *newfn;
{
	extern int errno;
	if ( 0 != link(oldfn, newfn))
	    return errno;
	if ( 0 != unlink(oldfn))
	    return errno;
	return 0;
}
#endif

char vi_tab_patt[] =  "vi: ts=";
void scanViTabSet(char *lnbuf) {
	char *c;
	c = strstr(lnbuf, vi_tab_patt);
	if (c == NULL)
		return;
	c = c+strlen(vi_tab_patt);
	int t;
	t = atoi(c);
	if (t > 2 && t < 16)
		tabs = t;
	return;
}

bool fetch(name)
char *name;
{
	char lnbuf[MAXLINE+1];

	infile=fopen(name,"r");
	if (infile == 0)
	{
		messg("File %s not found.", name);
		return(FAIL);
	}

	while (readln(lnbuf, MAXLINE) != EOF) {
		if (lnbuf[strlen(lnbuf)-1] == '\r') /* remove trailing CR */
		    lnbuf[strlen(lnbuf)-1] = '\0';
		scanViTabSet(lnbuf);
		if (!inslne(lnbuf))
		{
		     error("Not enough memory");
		     return(FAIL);
		}
	}
	if (fclose(infile) != 0)
		return(FAIL);

	return(SUCC);
}
