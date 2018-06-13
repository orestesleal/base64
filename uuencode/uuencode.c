/*
 *  encode one line of text into uuencode data 
 */
#include <stdio.h>
#include <string.h>
#include "uu.h"
int main(int argc, char *argv[])
{
	if (argc > 1) {
		unsigned int len = strlen(argv[1]);
		char b[len*2];
		uuencode(argv[1], b, len);
		printf("%s\n", b);
	}
	return 0;
}
