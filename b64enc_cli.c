/*
 * demonstrating test vectors for the new encoding decoding
 * base64 functions, in this case encoding text via argument
 * on the command line, and then the encoded string is
 * decoded and printed on the command line
 *
 * copyright 2015 Orestes Leal Rodríguez <lukes357@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
int main(int argc, char *argv[])
{
	if (argc > 1) {
		int len = strlen(argv[1]);
		char buf[len*2];
		char buf2[len+1];
		b64_enc((unsigned char *)argv[1], buf, len);
		b64_dec((unsigned char *)buf, buf2, strlen(buf));
		printf("%s\n", buf);
		printf("%s\n", buf2);
	}
	return 0;
}
