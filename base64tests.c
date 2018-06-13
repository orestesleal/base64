/*
   tests vectors for all base64 functions, both text and binary 
   Copyright Orestes Leal Rodríguez 2015 <lukes357@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
int main(void)
{
	char *txt = "hello world";
	char buf_enc[128];
	char buf_dec[256];

	base64_enc(txt, buf_enc);
	printf("\nusing base64_enc: %s = %s\n", txt, buf_enc);
	base64_dec(buf_enc, buf_dec);
	printf("using base64_dec: %s = %s\n", buf_enc, buf_dec);


	b64_enc(txt, buf_enc, strlen(txt));
	printf("\n---\nusing b64_enc: %s = %s\n", txt, buf_enc);
	b64_dec(buf_enc, buf_dec, strlen(buf_enc));
	printf("using b64_dec: %s = %s\n", buf_enc, buf_dec);

	return 0;
}
