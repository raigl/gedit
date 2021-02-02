#include <curses.h>

char *getenv();
char *tigetstr();
#define ESC '\033'

main(argc, argv)
int argc;
char *argv[];
{
	char *term;
	char *key;
	char *pattr;
	int i;
	char c;

	term = getenv("TERM");
	setupterm(term, fileno(stdout), NULL);
	printf("TERM=%s\n", term);

	for (i = 1; i < argc; ++i) {
	     key = argv[i];
	     pattr = tigetstr(key);
	     if (pattr == NULL) {
		printf("%s NULL\n", key);
		continue;
		}
	     if (pattr == (char *) -1) {
		printf("%s undefined\n", key);
		continue;
		}
	     printf("%s=<", key);
	     while (c = *(pattr++)) {
		     if ((c < 0) | (c > 126)) {
			     printf("<%0o>", c);
			     continue;
			     }
		     if (c==ESC) {
			     printf("\\E");
			     continue;
			     }
		     if ((c=='\n') || (c=='\r')) {
			     printf("\n");
			     continue;
			     }
		     if (c<' ') {
			     printf("^%c", c+'@');
			     continue;
			     }
		     printf("%c", c);
		     }
	     printf(">\n");
		}

}
