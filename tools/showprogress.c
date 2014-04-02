#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

void showprogress(int progress)
{
	 int k = 0;
	 int j = 0;
	 for (k = 0; k < 109; k++)
		 putchar('\b');

	 putchar('[');

	 for ( j = 0; j < progress; j++)
		 putchar('=');
	 putchar('>');

	 for ( j = 1; j <= 100 - progress; j++)
		 putchar(' ');

	 putchar(']');

	 fprintf(stdout, "  %3d%%",progress);
	 fflush(stdout);
	 usleep(10000);
}
int main()
{
	int i = 0;
	while( i++ < 100)
		showprogress(i);
	return 0;
}
