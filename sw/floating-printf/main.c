
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "unistd.h"

int main(int argc, char **argv) {
	int n = printf("ABCDEFX %s\n", "Done");
	int e = errno;
	float i = 0.5;
	puts(strerror(e));
	
	puts ("Hello World!");
	puts ("  Hello World!");
	puts (" X A ");
	putchar('a');
	putchar('b');
	putchar('c');
	putchar('\n');
	printf("  X Test\n");
	printf("%d\n", 12);
	if(i > 0.2)
		printf("i is > 0.2\n");
	printf("%f\n", i);
	puts ("Hello World!");

	return 0;
}
