#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void b16_enc(const char *s, char *b, unsigned int len);
void b16_dec(const char *s, char *b, unsigned int len);
int b16_val(const char t);
static char hex[17] = "0123456789ABCDEF"; 
int main(int argc, char *argv[])
{
	if (argc > 1) {
		char buf[strlen(argv[1])*2];
		char dec_buf[strlen(argv[1]+1)];

		b16_enc(argv[1], buf, strlen(argv[1]));
		printf("%s\n", buf);
		b16_dec(buf, dec_buf, strlen(buf));
		printf("%s\n", dec_buf);
	}
	return 0;
}
/* general purpose base16 encoder */
void b16_enc(const char *s, char *b, unsigned int len)
{
	while (len--) {
		*b++ = hex[*s >> 4];
		*b++ = hex[*s++ & 0x0f]; 	
	}
	*b = '\0';
}
/* general purpose base16 decoder */
void b16_dec(const char *s, char *b, unsigned int len)
{
	unsigned char t[2],d;
	unsigned int i;

	d = 0;
	for (i = 0; i < len; i+=2) {
   		for (d = 0; d < 2; d++) {
			if ((s[i+d] >= '0' && s[i+d] <= '9') || 
				(s[i+d] >= 'A' && s[i+d] <= 'F')) {
				for (t[d] = 0; s[i+d] != hex[t[d]]; ++t[d]); /* get position */
			}
			else { /* rfc4648 sec 3.3 paragraph #2 */ 
				fprintf(stderr, "error: non alphabet character found on encoded stream\n");
				exit(EXIT_FAILURE);
			}
		}
		t[0] <<= 4;
		t[0] |= t[1];
		*b++ = t[0];
	}	
	*b = '\0';
	return;
}
