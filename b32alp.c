/* print the base32 alphabet chars as hex numbers */
#include <stdio.h>
#include <stdlib.h>
void phex(const unsigned char *a);
static char b32_alp[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
int main(void)
{
	phex(b32_alp);
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

