
#include <stdio.h>
#include <termio.h>

main()
{
	int  c;
	raw(1);
	printf("hallo, los gehts\n");
#ifdef VT100
	printf("\033=");
#endif
	while (1)
	{       c=getchar();
		if (c==0x03)
		{
#ifdef VT100
			printf("\033=");
#endif
			raw(0);
			putchar('\n');
			exit(0);
		}
		if ((c<0) || (c>126)) {
			printf("<%d>", c);
			continue;
			}
		if (c<' ')
			printf("%c%c", 0x5E, c+'@');
		else
			putchar(c);
		if (c=='\n')
			putchar(c);
	}
}

raw(on)
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
	     act.c_cc[VMIN] = 3; /* MIN */
	     act.c_cc[VTIME] = 2; /* TIME */
	}
	else
	     act = old;
	ioctl(0,TCSETA, &act);
}
