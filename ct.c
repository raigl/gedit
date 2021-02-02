#include <curses.h>
#define  ESC 27

char *getenv();

char *terminal;


main(argc,argv)
int argc;
char *argv[];
{
	if (argc == 2) {
		terminal = argv[1];
/*                newterm(terminal, fileno(stdout), fileno(stdin));     */
		setterm(terminal);
	} else {
		terminal = getenv("TERM");
		}
	initscr();
	printf("terminal=%s=%s\n", terminal, longname());
	cbreak();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	perform();
	endwin();
}

perform()
{
	int c;
	printw("This %s \nhas %d lines and %d cols", longname(), LINES, COLS);
	newline();
	while ((c=getch()) != 'x') {
	    switch (c) {
#ifdef __STDC__
#define show_if(x) case x: printw(#x); break;
#else
#define show_if(x) case x: printw("<x>"); break;
#endif
show_if(KEY_BREAK)
show_if(KEY_DOWN)
show_if(KEY_UP)
show_if(KEY_LEFT)
show_if(KEY_RIGHT)
show_if(KEY_HOME)
#ifdef KEY_END
show_if(KEY_END)
#endif
#ifdef KEY_DO
show_if(KEY_DO)
#endif
#ifdef KEY_COMMAND
show_if(KEY_COMMAND)
#endif
#ifdef KEY_CANCEL
show_if(KEY_CANCEL)
#endif
show_if(KEY_BACKSPACE)
show_if(KEY_F0)
show_if(KEY_F(1))
show_if(KEY_F(2))
show_if(KEY_F(3))
show_if(KEY_F(4))
show_if(KEY_F(5))
show_if(KEY_F(6))
show_if(KEY_F(7))
show_if(KEY_F(8))
show_if(KEY_F(9))
show_if(KEY_F(10))
show_if(KEY_F(11))
show_if(KEY_F(12))
show_if(KEY_F(13))
show_if(KEY_F(14))
show_if(KEY_DL)
show_if(KEY_IL)
show_if(KEY_DC)
show_if(KEY_IC)
show_if(KEY_EIC)
show_if(KEY_CLEAR)
show_if(KEY_EOS)
show_if(KEY_EOL)
show_if(KEY_SF)
show_if(KEY_SR)
show_if(KEY_NPAGE)
show_if(KEY_PPAGE)
show_if(KEY_STAB)
show_if(KEY_CTAB)
show_if(KEY_CATAB)
show_if(KEY_ENTER)
show_if(KEY_SRESET)
show_if(KEY_RESET)
#ifdef KEY_PRINT
show_if(KEY_PRINT)
#endif
show_if(KEY_LL)
show_if(KEY_A1)
show_if(KEY_A3)
show_if(KEY_B2)
show_if(KEY_C1)
show_if(KEY_C3)
   default:     if ((c < 0) | (c > 126)) {
			printw("<%0o>", c);
			refresh();
			continue;
			}
		if (c==ESC) {
			addstr("\\E");
			refresh();
			continue;
			}
		if ((c=='\n') || (c=='\r')) {
			newline();
			continue;
			}
		if (c<' ') {
			addch('^');
			addch(c+'@');
			refresh();
			continue;
			}
		if (c == 's') {
			curs_set(1);
			continue;
			}
		if (c == 'b') {
			curs_set(2);
			continue;
			}
		if (c == 'h') {
			curs_set(0);
			continue;
			}
		addch(c);
		}
		refresh();
	    }
}

newline()
{
	int r,c;
	getyx(stdscr, r, c);
	move(r+1,0);
	refresh();
}
