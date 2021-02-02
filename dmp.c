
#include <stdio.h>

main()
{
	int c, cc=0;
	while (1)
	{       c=getchar(); ++cc;
		if (cc > 70) { printf("\n"); cc = 0; }
		if (c==EOF)
		{
			exit(0);
		}
		if (c<' ')
			printf("\\%c", c+'@');
		else
			putchar(c);
		if (c=='\n')
			putchar(c);
	}
}

