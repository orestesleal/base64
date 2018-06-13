/*
 * 	Implementations of Base64 and Base16 data encodings as they are 
 *	outlined by rfc4648 of 2006, there are several versions in this
 *	file some encode text input other encode any type of input like
 *	binary input [1].
 *
 *	[1] b64_enc is a general purpose base64 encoding function, 
 *		including support for binary input.
 *		all others work on text input
 *
 *	Orestes Leal Rodriguez 2015 <lukes357@gmail.com>
 *
 *	-devlog
 *		Aug 2: initial revision
 *		Aug 3: support for escaped characters.
 *		Aug 6: added base64 decoding 
 *		Aug 7: added base64url encoding
 *		Aug 8: base64 encoding for all file formats, including binary,
 *   	       this new version is named 'b64_enc' named so to not break
 *   	  	   the existing utilities that use 'base64_enc', if one is 
 * 			   encoding text files with escapes characters like the ones
 * 			   used to generate auth strings for the AUTH PLAIN SMTP command 
 *			   then 'base64_enc' is the more appropiate, otherwise b64_enc
 *		       will encode just about everything and is more efficient,
 *			   clear, and fast.
 */
#define ESC '\\'
#define PAD '='
char b64_alp[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char b64_url_alp[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/* get the position of the character 'tk' on the alphabet
   'alp' and test at most 'len' positions on it */
static inline unsigned char get_token_pos(char tk, unsigned char len, char alp[])
{
	unsigned char pos, lim;

	for (pos = 0, lim = len; tk != alp[pos] && lim; ++pos, lim--) ;
	return pos <= len ? pos : 0;
}
/* Base 64 decoding with URL and Filenamee Safe Alphabet */
void base64url_dec(char *s, char b[])
{
	unsigned int x;
	int i,z,w;

	x = w = 0;
	for (i = 0; s[i] != PAD && s[i]; i++) {
		for (x = 0, z = 0; z < 4 && s[i] && s[i] != PAD; z++) {
			x |= get_token_pos(s[i++], 64, b64_url_alp);
			z < 3 && s[i] != PAD && s[i] ? x <<= 6 : 0;
		}
		/*  if there are less than 24 bits of input, add 0s on 
			the right to make an integral number of 6 bit groups 
			(sec. 4. rfc4648 */
		for (;z < 4; z++, x <<= 6);  
		x <<= 8;
		b[w++] = x >> 24;
		b[w++] = (x << 8) >> 24;
		b[w++] = (x << 16) >> 24;
		--i;
	}
	b[w] = '\0';
}
/* Base 64 encoding with URL and Filenamee Safe Alphabet */
void base64url_enc(char *s, char b[])
{
	unsigned int x, npad;
	int i;

	npad = 0;
	for (i = 0; *s; i++) {
		x = s[0] == ESC ? s++[1]-'0' : s[0];
		x <<= 8;
		x |= s[1] == ESC ? s++[2]-'0' : s[1];
		x <<= 8;
		x |= s[2] == ESC ? s++[3]-'0' : s[2];
		x <<= 8;
		b[i++] = b64_url_alp[x >> 26];
		b[i++] = b64_url_alp[(x << 6) >> 26];

		if (s[1] == '\0') { /* quantum of encoding of 8 bits */
			npad = 2;
			break;
		}
		b[i++] = b64_url_alp[(x << 12) >> 26];
		
		if (s[2] == '\0') { /* quantum of 16 bits */
			npad = 1;
			break;
		}
		b[i] = b64_url_alp[(x << 18) >> 26];
		s += 3; /* point to the next 24 bit group */ 
	}
	for (;npad > 0; npad--) { /* add padding if needed */
		b[i++] = PAD;
	}
	b[i] = '\0';
}


/* decode a base64 string into the original text string */
void base64_dec(char *s, char b[])
{
	unsigned int x;
	int i,z, w;

	x = w = 0;
	for (i = 0; s[i] != PAD && s[i]; i++) {
		for (x = 0, z = 0; z < 4 && s[i] && s[i] != PAD; z++) {
			x |= get_token_pos(s[i++], 64, b64_alp);
			z < 3 && s[i] != PAD && s[i] ? x <<= 6 : 0;
		}
		/*  less than 24 bits of input, add 0s on the right to make
			an integral number of 6 bit groups (sec. 4. rfc4648 */
		for (;z < 4; z++, x <<= 6);  
		x <<= 8;
		b[w++] = x >> 24;
		b[w++] = (x << 8) >> 24;
		b[w++] = (x << 16) >> 24;
		--i;
	}
	b[w] = '\0';
}
/* encode a text string or text file into base64 */
void base64_enc(char *s, char b[])
{
	unsigned int x, npad;
	int i;

	npad = 0;
	for (i = 0; *s; i++) {
		x = s[0] == ESC ? s++[1]-'0' : s[0];
		x <<= 8;
		x |= s[1] == ESC ? s++[2]-'0' : s[1];
		x <<= 8;
		x |= s[2] == ESC ? s++[3]-'0' : s[2];
		x <<= 8;
		b[i++] = b64_alp[x >> 26];
		b[i++] = b64_alp[(x << 6) >> 26];

		if (s[1] == '\0') { /* quantum of encoding of 8 bits */
			npad = 2;
			break;
		}
		b[i++] = b64_alp[(x << 12) >> 26];
		
		if (s[2] == '\0') { /* quantum of 16 bits */
			npad = 1;
			break;
		}
		b[i] = b64_alp[(x << 18) >> 26];
		s += 3; /* point to the next 24 bit group */ 
	}
	for (;npad > 0; npad--) { /* add padding if needed */
		b[i++] = PAD;
	}
	b[i] = '\0';
}
void base16_encoder(char *s, char b[])
{
	char b16_alp[] = "0123456789ABCDEF";
	int i;
	for (i = 0; *s; i++, s++) { 
		unsigned char x = (unsigned char)*s >> 4;
		unsigned char z = ((unsigned char)(*s << 4)) >> 4;
		b[i] = b16_alp[x];
		b[++i] = b16_alp[z];
	}
	b[i] = '\0';
}
void base16_decoder(char *b16, char b[])
{
	int i;
	unsigned char n1, n2;
	for (i = 0; *b16; i++, b16+=2) {
		if (*b16  < 'A') {
			n1 = (*b16 - '0') << 4;
		}
		else {
			n1 = (*b16 - 0x37) << 4;
		}
		if (*(b16 + 1) < 'A') {
			n2 = *(b16 + 1) - '0';
		}
		else {
			n2 = *(b16 + 1) - 0x37;
		}
		b[i] =  n1 | n2;
	}
	b[i] = '\0';
}

/* base64 encoding for all types of inputs */
void b64_enc(unsigned const char *s, char b[], unsigned int len)
{
	unsigned int x;
	int i,z,w,u;
	unsigned char rm;

	rm = len % 3; 
	w = 0;
	for (i = 0; i < len; i++) {
		for (x = 0, z = 0; z < 3 && i < len; z++, i++) {
			x |= s[i];	
			(z < 2 && i+1 < len) ? x <<= 8 : 0;
		}
		for (;z < 4; z++, x <<= 8);
		b[w++] = b64_alp[x >> 26];
		for (u = 6; u <= 18; u+=6) {
			b[w++] = b64_alp[(x << u) >> 26];
		}

		/*
		b[w++] = b64_alp[(x << 6) >> 26];
		b[w++] = b64_alp[(x << 12) >> 26];
		b[w++] = b64_alp[(x << 18) >> 26];
		*/
		--i;
	}
	b[w] = '\0';
	if (rm > 0) { /* padding */ 
		for (x = rm == 1 ? 2 : 1; x > 0; --x) {
			b[--w] = PAD;
		}
	}
}
