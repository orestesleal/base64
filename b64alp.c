/* print the base64 alphabet chars as hex numbers */
#include <stdio.h>
#include <stdlib.h>
void phex(const unsigned char *a);
char b64_alp[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
int main(void)
{
	phex(b64_alp);
	return 0;
}
void phex(const unsigned char *a)
{
	static char hex[16] = "0123456789abcdef";
	unsigned char t,d;

	t = d = 0;
	while (*a) {
		t = hex[*a & 0x0f];
		d = hex[*a >> 4];
		printf("%c -> hex = 0x%c%c\n", *a, d,t);
		++a;
	}
}

