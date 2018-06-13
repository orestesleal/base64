#include <stdio.h>
#include <stdlib.h>
#include "base64.h"
int main(void)
{
	char buf[256];
	char *b64_str = "RGVsZXRlMjI="; /* base64 encoded string */
	base64_dec(b64_str, buf); /* decode the base64 string */ 
	printf("%s\n", buf);

	return 0;
}
