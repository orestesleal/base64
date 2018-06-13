/*
 *	test vectors for b32_enc a general purpose base32 encoder
 *  declared in base64.c
 */
#include <stdio.h>
#include <string.h>
#include "base64.h"
int main(int argc, char *argv[])
{
	char buf[1024];
	char b[1024];
	if (argv[1]) {
		b32_enc(argv[1], buf, strlen(argv[1]));
		b32_dec(buf, b, strlen(buf));
		printf("%s\n---\n", buf);
		printf("%s\n", b);
	}
	return 0;
}
