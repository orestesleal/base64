/*
 * 	Base64, Base32 and Base16 encodings (and variations) as they are 
 *	standardized by rfc4648 of 2006.
 *
 *	There are several encoding functions in this file, some are intended to 
 *	encode only text and the others encode any type of input, like binary (images,
 *	executables, etc.) [1]
 *
 *	[1] b64_enc, b64_dec, b32_enc and b32_dec are general purpose base64
 *		and base32 encoding and decoding functions, with support for binary
 *		and text input, all others functions work on text data only.
 *
 *	Orestes Leal Rodriguez 2015-2025 <olealrd1981@gmail.com>
 *
 *	-devlog
 *		Aug 2: initial revision, base16 and base64 text encoding
 *		Aug 3: support for escaped characters on base64_enc
 *		Aug 6: added base64 text decoding 
 *		Aug 7: added base64url encoding/decoding
 *		Aug 8: added a base64 encoder for all file formats, incl binary,
 *   	       this new version is named 'b64_enc' named so to not break
 *   	  	   the existing utilities that use 'base64_enc', if one is 
 * 			   encoding text files with escapes characters like the ones
 * 			   used to generate auth strings for the AUTH PLAIN SMTP command, 
 *			   then 'base64_enc' is the more appropiate, otherwise this 
 *			   encoder will suffice, also is more efficient, clear, and fast.
 *		Aug 9: some small optimizations to b64_enc
 *			   added a universal base64 decoder named 'b64_dec'
 *			   which can decode pretty much everything from base64
 *			   encoded text strings to binary images or executables,
 *			   after decoding it returns the number of bytes that
 *			   were encoded in the base64 string, this is useful for
 *			   example in a mail client where an email with an image
 *			   as an attachment is encoded in base64 but the size of
 *			   the output image isn't known, with this return value the
 *			   original file can be saved to disk specifying the number
 *			   of bytes to write.
 *	   Aug 10: b64_dec: Added optimizations to compute length of the encoded
 *			   data outside the decoding loop
 *			   Added a function (get_data_size) to get the size in bytes 
 *			   of the data encoded in a base64 string
 *	   Aug 12: general purpose base32 encoding supported (b32_enc)
 *			   good optimizations added for base32 encoding 
 *	   Aug 13: general purpose base32 decoder added (b32_dec)
 *	   Aug 14: b32_dec now returns the size of the data decoded 
 *	   		   Integrate the get_token_pos() code into b32_dec for avoiding
 *			   the overhead of the function call
 *	   Aug 15: added utilities functions to help encode and decode files
 *			   easily, they can be read at the end of this file or in base64.h
 *			   to look at the arguments they require, they are very simple.
 *	   Aug 17: some changes made to b32_dec to implement the recommentations
 *			   of the rfc on handling invalid input coming from encoded data.
 *	   Aug 17: new base16 encoder and decoder (b16_enc / b16_dec). This new
 *			   versions are better, the decoder handles invalid input and they
 *			   are better coded and work on binary input as well.
 *	   Aug 17: the utitilies for encoding and decoding files now suport base16
 *	   Aug 18: Several fixes to b64_dec for allowing decode files with crlf
 *			   at the end of the file.
 *			   A small number of isolated fixes and reworking all over the code.
 *			   The base16_encoder and base16_decoder functions are marked as 
 *			   obsoletes in favor of using the new base16 encoder/decoder
 *			   functions 'b16_enc' and 'b16_dec'
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include "base64.h"
#define ESC '\\'
#define PAD '='
/* alphabets for base64, base64url, base32, and base16 */ 
char b64_alp[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char b64_url_alp[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
char b32_alp[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
static char b16_alp[17] = "0123456789ABCDEF";

/* Lookup tables for O(1) character decoding */
static const unsigned char b64_lookup[256] = {
    ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,  ['F'] = 5,
    ['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
    ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25,
    ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29, ['e'] = 30, ['f'] = 31,
    ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35, ['k'] = 36, ['l'] = 37,
    ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41, ['q'] = 42, ['r'] = 43,
    ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47, ['w'] = 48, ['x'] = 49,
    ['y'] = 50, ['z'] = 51,
    ['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57,
    ['6'] = 58, ['7'] = 59, ['8'] = 60, ['9'] = 61,
    ['+'] = 62, ['/'] = 63
    /* All other values remain 0 (invalid) */
};

/* get the position of the character 'tk' on the alphabet
   'alp' and test at most 'len' positions on it */
unsigned char get_token_pos(char tk, unsigned char len, char alp[])
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
		/*  TODO: update this to a single computation 
			less than 24 bits of input, add 0s on the right to make
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

/* obsolete: do not use, use b16_enc instead */
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
/* obsolete: do not use, use b16_dec instead */
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
/**
 * @brief Encode binary data to base64
 * @param s Input data to encode
 * @param b Output buffer (must be large enough)
 * @param len Length of input data in bytes
 * @note Output buffer should be at least ((len + 2) / 3) * 4 + 1 bytes
 */
void b64_enc(unsigned const char *s, char b[], unsigned int len)
{
	unsigned int x,i,w;
	unsigned char rm,z;

	w = 0;
	for (i = 0; i < len; i++) {
		for (x = 0, z = 0; z < 3 && i < len; z++, i++) {
			x |= s[i];	
			(z < 2 && i+1 < len) ? x <<= 8 : 0;
		}
		for (;z < 4; z++, x <<= 8);
		b[w++] = b64_alp[x >> 26];
		b[w++] = b64_alp[(x << 6) >> 26];
		b[w++] = b64_alp[(x << 12) >> 26];
		b[w++] = b64_alp[(x << 18) >> 26];
		--i;
	}
	b[w] = '\0';
	rm = len % 3; 
	if (rm) { /* padding */ 
		for (x = rm == 1 ? 2 : 1; x; --x) {
			b[--w] = PAD;
		}
	}
}
/**
 * @brief Decode base64 string to binary data
 * @param s Base64 encoded string to decode
 * @param b Output buffer for decoded data (must be large enough)
 * @param len Length of input base64 string
 * @return Number of decoded bytes, or 0 on error
 * @note Output buffer should be at least (len / 4) * 3 bytes
 */
unsigned int b64_dec(const unsigned char *s, char b[], unsigned int len)
{
	unsigned int x,w,i,rtsize,z;
	unsigned char pad,t;
	t = pad = 0;

	/* handle empty input */
	if (len == 0) {
		b[0] = '\0';
		return 0;
	}

	/* this is non-standard behavior:
		if the input file or stream contains a LINE FEED or CARRIAGE
		RETURN characters  at the end of the file (maybe generated by
		an editor) remove it to allow the padding trimming do his job */
	for (i = len-1; i >= 0 && (s[i] == '\r' || s[i] == '\n'); i--, --len); 

	/* handle case where input becomes empty after CRLF removal */
	if (len == 0) {
		b[0] = '\0';
		return 0;
	}

	/* remove trailing padding and count them, 1 or 2 '='s 
	   for base64 */	
	for (i = len-1; i >= 0 && s[i] == PAD; --i) {
		++pad;
		--len;
	}
	w = 0;
	for (i = 0; i < len; i++) {
		for (x = 0, z = 0; z < 4 && i < len; z++, i++) {
			/* if the rfc is modified to include ignoring CRLF or 
			   PAD's in between encoded data the next block of code
			   should be uncommented
		       rfc4648 sec 3.3: */
			/*
			if (s[i] == PAD || 
				s[i] == '\n' || 
				s[i] == '\r') {
				--z;
				--i;
				continue;
			}
			*/

			/*	rfc4648 section 3.3 */ 
			if ((s[i] >= 'A' && s[i] <= 'Z') || 
   		   		(s[i] >= 'a' && s[i] <= 'z') || 
   		   		(s[i] >= '0' && s[i] <= '9') || 
	       		(s[i] == '+' || s[i] == '/') ) {

				/* use lookup table for O(1) character decoding */
				t = b64_lookup[(unsigned char)s[i]];
				if (t == 0 && s[i] != 'A') {  /* 'A' maps to 0, need explicit check */
					fprintf(stderr, "b64_dec: error, non alphabet char found -> %c -> %d\n",
							s[i], s[i]);
					exit(EXIT_FAILURE);
				}
				x |= t;
				z < 3 && i+1 < len ? x <<= 6 : 0;
			} else {
				fprintf(stderr, "b64_dec: error, non alphabet char found -> %c -> %d\n",
																			s[i], s[i]);
				exit(EXIT_FAILURE);
			}
		}
		for (;z < 4; z++, x <<= 6);
		x <<= 8;
		b[w++] = x >> 24;
		b[w++] = (x << 8) >> 24;
		b[w++] = (x << 16) >> 24;
		--i;
	}
	rtsize = (len >> 2) * 3;
	if (pad) {
		rtsize += pad == 2 ? 1 : 2;
	}
	b[rtsize] = '\0';
	return rtsize; /* number of decoded bytes */
}
/* get the size of the encoded data in a base64 string */
unsigned int get_data_size(char *s, unsigned int len)
{
	unsigned int rtsize, i;
	unsigned char pad = 0;
	for (i = len-1; s[i] == PAD; --i) {
		++pad;
		--len;
	}
	rtsize = (len >> 2) * 3;
	if (pad) {
		rtsize += pad == 2 ? 1 : 2;
	}
	return rtsize;
}
/**
 * @brief Calculate required buffer size for base64 encoding
 * @param input_len Length of input data in bytes
 * @return Required buffer size in bytes (including null terminator)
 */
unsigned int b64_enc_size(unsigned int input_len)
{
	return ((input_len + 2) / 3) * 4 + 1; /* +1 for null terminator */
}
/**
 * @brief Calculate required buffer size for base64 decoding
 * @param input_len Length of base64 encoded string
 * @return Required buffer size in bytes (including null terminator)
 */
unsigned int b64_dec_size(unsigned int input_len)
{
	return (input_len / 4) * 3 + 1; /* +1 for null terminator */
}
/* general purpose Base32 encoding */ 
void b32_enc(unsigned char *s, unsigned char *b, unsigned int len)
{
	unsigned long long x;	/* c99 only */
	unsigned int i,w,z;
	unsigned char ap[5] = {0,6,4,3,1};

	w = x = 0;
	/* 	in base32 per 5 bytes of input we get 40 bits that must
		be treated as eight 5 bits groups, each 5 bit number
		will be used as an index into the base32 alphabet  */
	for (i = 0; i < len; i++) {
		for (z = 0; z < 5 && i < len; z++, i++) {
			x |= s[i];
			x <<= 8;
		}
		/* compute the number of shifts needed to align all bytes
			to the most significant part of x, this avoids several
			iterations of one 'test, shift, and increment' loop */
		x <<= (7 - z) << 3; /* mul by 8 */ 

		/* 	this block could be optimized for size within a loop, but that
			will involve a and additional 'test and increment' step on a 
			loop, which will make the code a tiny fraction more slower,
			this is coded for speed, not size */
		b[w++] = b32_alp[x >> 59];
		b[w++] = b32_alp[(x << 5) >> 59];
		b[w++] = b32_alp[(x << 10) >> 59];
		b[w++] = b32_alp[(x << 15) >> 59];
		b[w++] = b32_alp[(x << 20) >> 59];
		b[w++] = b32_alp[(x << 25) >> 59];
		b[w++] = b32_alp[(x << 30) >> 59];
		b[w++] = b32_alp[(x << 35) >> 59];
		--i;
	}
	b[w] = '\0';
	for (i = ap[len % 5]; i; i--) {
		b[--w] = PAD; 
	}		
	return;
}
/* general purpose Base32 decoding */
unsigned int b32_dec(unsigned char *s, char *b, unsigned int len)
{
	unsigned long long x;
	unsigned int i,w,z, rtsize;
	unsigned char pad;
	unsigned char t;
	/* 	lookup table used as an index with the value of 'pad' 
		to help calculate the size of the encoded data */
	unsigned char lkpad[7] = {0,4,0,3,2,0,1};

	/* in base32 there is a maximum of 6 padding characters
	   in this case this loop gets rid of them anyway 

	   rfc4648 sect 3.3:
	   if more than the allowed number of pad characters is found at
	   the end of the string (e.g., a base 64 string terminated with 
	   "==="), the excess pad characters MAY also be ignored
	*/
	for (pad = 0,i = len-1; s[i] == PAD; i--) {
		++pad;
		--len;
	}

	w = 0;
	for (i = 0; i < len; i++) {
		for (x = 0, z = 0; z < 8 && i < len; z++, i++) {
			/* 
			   if the rfc is modified to include ignoring CRLF or 
			   PAD's in between encoded data the next block of code
			   should be uncommented
		       rfc4648 sec 3.3:
			if (s[i] == PAD || s[i] == '\n' || s[i] == '\r' ) {
				--z;
				continue;
			}
			*/
			/* rfc4648 sec. 3.3
				Implementations MUST reject the encoded data if it contains
				characters outside the base alphabet when interpreting base-encoded
				data
	    	*/
			if ((s[i] >= 'A' && s[i] <= 'Z') ||  
				(s[i] >= '2' && s[i] <= '7')) {	 
				for (t = 0; s[i] != b32_alp[t]; t++);
				x |= t;
				x <<= 5;
			} else {
				fprintf(stderr, "b32_dec: error non alphabet char found\n");
				exit(EXIT_FAILURE);
			}
		}
		/* 	compute the shifting needed to align the input
			into the msb */
		x <<= (19 + ((8-z)*5)); 
		b[w++] = x >> 56; 
		b[w++] = (x << 8) >> 56;
		b[w++] = (x << 16) >> 56;
		b[w++] = (x << 24) >> 56;
		b[w++] = (x << 32) >> 56;
		--i;
	}
	/* divide the length by 8 and multiply the result
	   by 5, then adding the value of 'pad' indexed into
	   'lkpad' computes the size of the encoded data.
	   shr by 3 is the same as "len / 8" */
	rtsize = (len >> 3) * 5 + lkpad[pad]; 
	b[rtsize] = '\0';
	return rtsize;
}
/* general purpose base16 encoder */
void b16_enc(const unsigned char *s, char *b, unsigned int len)
{
	while (len--) {
		*b++ = b16_alp[*s >> 4];
		*b++ = b16_alp[*s++ & 0x0f]; 	
	}
	*b = '\0';
}
/* general purpose base16 decoder */
unsigned int b16_dec(const char *s, char *b, unsigned int len)
{
	unsigned char t[2],d;
	unsigned int i;

	d = 0;
	for (i = 0; i < len; i+=2) {
   		for (d = 0; d < 2; d++) {
			if ((s[i+d] >= '0' && s[i+d] <= '9') || 
				(s[i+d] >= 'A' && s[i+d] <= 'F')) {
				for (t[d] = 0; s[i+d] != b16_alp[t[d]]; ++t[d]); /* get position */
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
	return len >> 1;
}
/* -------------------------------------------------------------------> utilities */
/* write an input file into a destination file encoded in
   the base encoding specified in 'mode'. 
   modes supported : BASE64, BASE32, BASE16 */
void encode_wr_file(const char *src, const char *dst, unsigned char mode)
{
	struct finfo *fd;
	char *enc_buf;
	int ofd;
	size_t wr;
	int flags = O_CREAT | O_RDWR;
	int perms = S_IRUSR | O_TRUNC | S_IWUSR; 
	
	if ((fd = get_file(src)) == NULL) { /* load the input file 'src' */
		fprintf(stderr, "error: get_file() failed to load %s in main memory\n", src);
		exit(EXIT_FAILURE);
	}
	enc_buf = alloc(fd->size*2); /* buffer for the encoded string */
	switch (mode) {
		case BASE64:
			b64_enc((unsigned char *)fd->addr, enc_buf, fd->size);
			break;
		case BASE32:
			b32_enc((unsigned char *)fd->addr, (unsigned char *)enc_buf, fd->size);
			break;
		case BASE16:
			b16_enc(fd->addr, enc_buf, fd->size); 
			break;
		default:
			fprintf(stderr, "error: encoding %d mode not supported\n", mode);
			exit(EXIT_FAILURE);
	}
    if ((ofd = open(dst, flags, perms)) == -1) {
   		fprintf(stderr, "open(2) error, can't create or truncate %s\n", dst);
	    exit(EXIT_FAILURE);
	}
	if ((wr = write(ofd, enc_buf, strlen(enc_buf))) == -1) {
		fprintf(stderr, "write(2) error, can't write to %s\n", dst);
		exit(EXIT_FAILURE);
	}
	close(ofd);
	free(enc_buf);
	free(fd);
}
/* return the memory address of the allocated memory
   or exit(3) the program if an error occurs */
char *alloc(unsigned int size)
{
    char *ptr = (char *)malloc(size);
    if (ptr == NULL) {
		fprintf(stderr, "malloc: error, can't allocate %d bytes\n", size);
		exit(EXIT_FAILURE);
	}
	return ptr;
}

/* open a file, get file info, load it to memory and return file 
   information on memory as a pointer to a structure 'finfo', 
   returns NULL on error */
struct finfo *get_file(const char *f)
{
	int fd;
	struct stat fp;
	char *addr; 
	struct finfo *st_addr;
	size_t rd;

	if ((fd = open(f, O_RDONLY)) == -1) {
		fprintf(stderr, "open(2) failed for file %s\n", f);
		return NULL;
	}
	stat(f, &fp); /* get file info with stat(2) */
	if ((addr = (char *)malloc(fp.st_size)) == NULL) {
		fprintf(stderr, "malloc(3) failed allocating %d bytes\n", (int)fp.st_size);
		close(fd);
		return NULL;
	}
	if ((rd = read(fd, addr, fp.st_size)) == -1) {
		fprintf(stderr, "read(2) failed reading %s with %d bytes into memory\n", f, (int)fp.st_size);
		close(fd);
		free(addr);
		return NULL;
	}
	if ((st_addr = (struct finfo *)malloc(sizeof(struct finfo))) == NULL) {
		fprintf(stderr, "malloc(3) failed allocating the returned structure\n");
		close(fd);
		free(addr);
		return NULL;
	}
	close(fd);
	st_addr->addr = addr;
	st_addr->size = fp.st_size;
	return st_addr;
}
/* read,decode and write the decoded input file to a new file */
void decode_rd_file(const char *src, const char *dst, unsigned char mode)
{
	struct finfo *fd;
	char *dec_buf;
	int ofd;
	size_t wr;
	size_t dec; /* number of decoded bytes */
	int flags = O_CREAT | O_RDWR;
    int perms = S_IRUSR | O_TRUNC | S_IWUSR;

	if ((fd = get_file(src)) == NULL) {
		fprintf(stderr, "get_file() failed loading %s in main memory\n", src);
		exit(EXIT_FAILURE);
	}

	dec_buf = alloc(fd->size); 
	switch (mode) {
		case BASE64:
			dec = b64_dec((const unsigned char *)fd->addr, dec_buf, fd->size); 
			break;
		case BASE32:
			dec = b32_dec((unsigned char *)fd->addr, dec_buf, fd->size); 
			break;
		case BASE16:
			dec = b16_dec(fd->addr, dec_buf, fd->size); 
			break;
		default:
			fprintf(stderr,"error: mode %d not supported\n", mode);
			exit(EXIT_FAILURE);
	}

    if ((ofd = open(dst, flags, perms)) == -1) {
      	fprintf(stderr, "open(2) error, can't create or truncate %s\n", dst);
      	exit(EXIT_FAILURE);
	}
	/* write the contents of `dec_buf` into the file requested on dst */
	if ((wr = write(ofd, dec_buf, dec)) == -1) {
		fprintf(stderr, "write(2) error, can't write to %s\n", dst);
		exit(EXIT_FAILURE);
	}
	close(ofd);
	free(dec_buf);
	free(fd);
}
