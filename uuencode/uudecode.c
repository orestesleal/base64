/*
 *  decode one line of encoded data 
 */
#include <stdio.h>
#include <string.h>
#include "uu.h"
int main(int argc, char *argv[])
{
	if (argc > 1) {
		int len = strlen(argv[1]);
		char buf[len];
		if (!uudecode(argv[1], buf, len)) {
			printf("%s\n", buf);
		}
	}
	return 0;
}
