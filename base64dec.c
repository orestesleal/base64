#include <stdio.h>
#include <stdlib.h>
#include "base64.h"
int main(void)
{
	char *url = "http://www.phoronix.com/_data/encoding.py";
	char buf_enc[128];
	char buf_dec[256];

	base64url_enc(url, buf_enc);
	printf("%s\n", url);
	printf("%s\n", buf_enc);
	base64url_dec(buf_enc, buf_dec);
	printf("%s\n", buf_dec);


	return 0;
}
