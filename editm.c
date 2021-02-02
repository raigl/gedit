/*
editm.c   Main module for Editor
*/

#include "edit.h"

#define IDLEN 40
char id[IDLEN+1] = "edit";
extern char *relver;

#ifdef MSDOS
#include <dir.h>
char drive[MAXDRIVE];
char dir[MAXDIR];
char basename[MAXFILE];
char ext[MAXEXT];
#endif

char fname[FNLEN+1];    /* name of command file */
bool inplace=FALSE, readonly;
int tabs = 8;
#ifdef UNIX
#include <locale.h>
extern char *viewpgm, *alterpgm;
#endif


void parmerr()
{
	printf(" Usage: %s -ri -t8 +n datei\n", id);
	exit(1);
}

void options(args)
char *args;
{
	char c;

	while ((c = *args) != 0) {
	    ++args;
	    switch (tolower(c)) {
	       case '-':  /* skip (first) option char */
			  break;
	       case 'r':  readonly = TRUE;
			  break;
	       case 'i':  inplace  = TRUE;
			  break;
	       case 't':  tabs = atoi(args);
			  return;
	       default:   parmerr();
			  break;
	       }
	}
}

main(argc, argv)
int argc;
char *argv[];
{
	char *s;
	int startline = 1;
	int filecnt = 0;

	strncpy(id, argv[0], IDLEN);
#ifdef MSDOS
	fnsplit(id, drive, dir, basename, ext);
	strcpy(id, basename);
#endif
#ifdef UNIX
	readonly =  0 == strcmp(id, viewpgm);
	inplace  =  0 == strcmp(id, alterpgm);
	setlocale(LC_ALL, "");
#endif
	while (argc > 1) {
	    ++argv; --argc;
	    switch (*argv[0]) {
		case '-':  options(*argv);
			   break;
		case '+':  startline = atoi(*argv);
			   break;
		default:   if (filecnt++) parmerr();
			   if (strlen(*argv) > FNLEN)
			       parmerr();
			   else
			       strcpy(fname, *argv);
			   break;
		}
	}
	if (filecnt != 1) parmerr();
	strncat(id, relver, IDLEN-strlen(id));
	beginscr();
	messg("Reading %s ... ", fname);
	readonly |= (0 == access(fname, 4)) &&
		   (0 != access(fname, 2));
	fetch(fname);
	if (readonly)
		messg("File is Read-Only", NULL);
	run(startline);
	return 0;       /* never reached */
}
