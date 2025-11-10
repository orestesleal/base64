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
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <ctype.h>
#include "base64.h"
#define ESC '\\'
#define PAD '='
/* alphabets for base64, base64url, base32, and base16 */ 
static const char b64_alp[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char b64_url_alp[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const char b32_alp[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
static const char b16_alp[17] = "0123456789ABCDEF";

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
int get_token_pos(char tk, unsigned char len, const char alp[])
{
        for (unsigned char pos = 0; pos < len; ++pos) {
                if (tk == alp[pos]) {
                        return (int)pos;
                }
        }
        return -1;
}
/* Base 64 decoding with URL and Filenamee Safe Alphabet */
void base64url_dec(char *s, char b[])
{
        unsigned int x;
        int i,z,w;

        x = w = 0;
        for (i = 0; s[i] != PAD && s[i]; i++) {
                for (x = 0, z = 0; z < 4 && s[i] && s[i] != PAD; z++) {
                        int pos = get_token_pos(s[i++], 64, b64_url_alp);
                        if (pos < 0) {
                                errno = EINVAL;
                                b[0] = '\0';
                                return;
                        }
                        x |= (unsigned int)pos;
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
                        int pos = get_token_pos(s[i++], 64, b64_alp);
                        if (pos < 0) {
                                errno = EINVAL;
                                b[0] = '\0';
                                return;
                        }
                        x |= (unsigned int)pos;
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
        unsigned int trimmed_len = len;
        unsigned int w = 0;

        if (b == NULL || s == NULL) {
                errno = EINVAL;
                return 0;
        }

        errno = 0;

        while (trimmed_len > 0 && (s[trimmed_len - 1] == '\r' || s[trimmed_len - 1] == '\n')) {
                --trimmed_len;
        }

        if (trimmed_len == 0) {
                b[0] = '\0';
                return 0;
        }

        if ((trimmed_len % 4) != 0) {
                errno = EINVAL;
                b[0] = '\0';
                return 0;
        }

        for (unsigned int i = 0; i < trimmed_len; i += 4) {
                unsigned char c0 = s[i];
                unsigned char c1 = s[i + 1];
                unsigned char c2 = s[i + 2];
                unsigned char c3 = s[i + 3];

                if (c0 == PAD || c1 == PAD) {
                        errno = EINVAL;
                        b[0] = '\0';
                        return 0;
                }

                unsigned char v0 = b64_lookup[c0];
                unsigned char v1 = b64_lookup[c1];
                unsigned char v2 = 0;
                unsigned char v3 = 0;

                if ((v0 == 0 && c0 != 'A') || (v1 == 0 && c1 != 'A')) {
                        errno = EINVAL;
                        b[0] = '\0';
                        return 0;
                }

                if (c2 != PAD) {
                        v2 = b64_lookup[c2];
                        if (v2 == 0 && c2 != 'A') {
                                errno = EINVAL;
                                b[0] = '\0';
                                return 0;
                        }
                }

                if (c3 != PAD) {
                        v3 = b64_lookup[c3];
                        if (v3 == 0 && c3 != 'A') {
                                errno = EINVAL;
                                b[0] = '\0';
                                return 0;
                        }
                }

                unsigned int triple = (v0 << 18) | (v1 << 12) | (v2 << 6) | v3;

                b[w++] = (triple >> 16) & 0xff;
                if (c2 != PAD) {
                        b[w++] = (triple >> 8) & 0xff;
                }
                if (c3 != PAD) {
                        b[w++] = triple & 0xff;
                }
        }

        b[w] = '\0';
        return w;
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
void b32_enc(const unsigned char *s, unsigned char *b, unsigned int len)
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
unsigned int b32_dec(const unsigned char *s, char *b, unsigned int len)
{
        unsigned int clean_len = len;
        unsigned int pad = 0;
        unsigned int w = 0;
        unsigned long long buffer = 0;
        unsigned int bits = 0;
        static const unsigned char lkpad[7] = {0,4,0,3,2,0,1};

        if (s == NULL || b == NULL) {
                errno = EINVAL;
                return 0;
        }

        errno = 0;

        while (clean_len > 0 && (s[clean_len - 1] == '\r' || s[clean_len - 1] == '\n')) {
                --clean_len;
        }

        while (clean_len > 0 && s[clean_len - 1] == PAD) {
                ++pad;
                --clean_len;
        }

        if (clean_len == 0 && pad == 0) {
                b[0] = '\0';
                return 0;
        }

        for (unsigned int i = 0; i < clean_len; ++i) {
                int idx = get_token_pos((char)s[i], 32, b32_alp);
                if (idx < 0) {
                        errno = EINVAL;
                        b[0] = '\0';
                        return 0;
                }
                buffer = (buffer << 5) | (unsigned int)idx;
                bits += 5;
                if (bits >= 8) {
                        bits -= 8;
                        b[w++] = (char)((buffer >> bits) & 0xff);
                }
        }

        if (pad) {
                if (pad >= sizeof(lkpad)) {
                        errno = EINVAL;
                        b[0] = '\0';
                        return 0;
                }
                if (w < lkpad[pad]) {
                        errno = EINVAL;
                        b[0] = '\0';
                        return 0;
                }
                w -= lkpad[pad];
        }

        b[w] = '\0';
        return w;
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
        unsigned int w = 0;

        if (s == NULL || b == NULL) {
                errno = EINVAL;
                return 0;
        }

        errno = 0;

        if ((len & 1U) != 0) {
                errno = EINVAL;
                b[0] = '\0';
                return 0;
        }

        for (unsigned int i = 0; i < len; i += 2) {
                char hi = (char)toupper((unsigned char)s[i]);
                char lo = (char)toupper((unsigned char)s[i + 1]);
                int hi_pos = get_token_pos(hi, 16, b16_alp);
                int lo_pos = get_token_pos(lo, 16, b16_alp);

                if (hi_pos < 0 || lo_pos < 0) {
                        errno = EINVAL;
                        b[0] = '\0';
                        return 0;
                }

                b[w++] = (char)((hi_pos << 4) | lo_pos);
        }

        b[w] = '\0';
        return w;
}

/* -------------------------------------------------------------------> utilities */
/* write an input file into a destination file encoded in
   the base encoding specified in 'mode'. 
   modes supported : BASE64, BASE32, BASE16 */
int encode_wr_file(const char *src, const char *dst, unsigned char mode)
{
        struct finfo *fd = get_file(src);
        char *enc_buf = NULL;
        int ofd = -1;
        ssize_t wr;
        int flags = O_CREAT | O_RDWR;
        int perms = S_IRUSR | O_TRUNC | S_IWUSR;
        int status = -1;
        size_t buf_len = 0;
        size_t out_len = 0;

        if (fd == NULL) {
                return -1;
        }

        switch (mode) {
                case BASE64:
                        buf_len = b64_enc_size(fd->size);
                        break;
                case BASE32:
                        buf_len = ((fd->size + 4) / 5) * 8 + 1;
                        break;
                case BASE16:
                        buf_len = fd->size * 2 + 1;
                        break;
                default:
                        errno = EINVAL;
                        goto cleanup;
        }

        enc_buf = malloc(buf_len);
        if (enc_buf == NULL) {
                errno = ENOMEM;
                goto cleanup;
        }

        switch (mode) {
                case BASE64:
                        b64_enc((const unsigned char *)fd->addr, enc_buf, fd->size);
                        break;
                case BASE32:
                        b32_enc((const unsigned char *)fd->addr, (unsigned char *)enc_buf, fd->size);
                        break;
                case BASE16:
                        b16_enc((const unsigned char *)fd->addr, enc_buf, fd->size);
                        break;
        }

        ofd = open(dst, flags, perms);
        if (ofd == -1) {
                goto cleanup;
        }

        out_len = strlen(enc_buf);
        wr = write(ofd, enc_buf, out_len);
        if (wr == -1 || (size_t)wr != out_len) {
                goto cleanup;
        }

        status = 0;

cleanup:
        if (ofd != -1) {
                close(ofd);
        }
        free(enc_buf);
        free_finfo(fd);
        return status;
}

char *alloc(unsigned int size)
{
        char *ptr = (char *)malloc(size);
        if (ptr == NULL) {
                errno = ENOMEM;
        }
        return ptr;
}

/* open a file, get file info, load it to memory and return file 
   information on memory as a pointer to a structure 'finfo', 
   returns NULL on error */
struct finfo *get_file(const char *f)
{
        int fd = -1;
        struct stat fp;
        char *addr = NULL;
        struct finfo *st_addr = NULL;
        size_t to_read = 0;
        size_t offset = 0;

        if (f == NULL) {
                errno = EINVAL;
                return NULL;
        }

        fd = open(f, O_RDONLY);
        if (fd == -1) {
                return NULL;
        }

        if (fstat(fd, &fp) == -1) {
                close(fd);
                return NULL;
        }

        if (fp.st_size < 0) {
                errno = EINVAL;
                close(fd);
                return NULL;
        }

        if ((unsigned long long)fp.st_size > (unsigned long long)(SIZE_MAX - 1)) {
                errno = EOVERFLOW;
                close(fd);
                return NULL;
        }

        to_read = (size_t)fp.st_size;
        addr = malloc(to_read + 1);
        if (addr == NULL) {
                errno = ENOMEM;
                close(fd);
                return NULL;
        }

        while (offset < to_read) {
                ssize_t chunk = read(fd, addr + offset, to_read - offset);
                if (chunk < 0) {
                        free(addr);
                        close(fd);
                        return NULL;
                }
                if (chunk == 0) {
                        break;
                }
                offset += (size_t)chunk;
        }

        close(fd);
        addr[offset] = '\0';

        st_addr = malloc(sizeof(*st_addr));
        if (st_addr == NULL) {
                free(addr);
                errno = ENOMEM;
                return NULL;
        }

        st_addr->addr = addr;
        st_addr->size = offset;
        return st_addr;
}

void free_finfo(struct finfo *info)
{
        if (info == NULL) {
                return;
        }
        free(info->addr);
        free(info);
}

int decode_rd_file(const char *src, const char *dst, unsigned char mode)
{
        struct finfo *fd = get_file(src);
        char *dec_buf = NULL;
        int ofd = -1;
        ssize_t wr;
        unsigned int dec = 0;
        int flags = O_CREAT | O_RDWR;
        int perms = S_IRUSR | O_TRUNC | S_IWUSR;
        int status = -1;

        if (fd == NULL) {
                return -1;
        }

        dec_buf = malloc(fd->size + 1);
        if (dec_buf == NULL) {
                errno = ENOMEM;
                goto cleanup;
        }

        errno = 0;
        switch (mode) {
                case BASE64:
                        dec = b64_dec((const unsigned char *)fd->addr, dec_buf, fd->size);
                        break;
                case BASE32:
                        dec = b32_dec((const unsigned char *)fd->addr, dec_buf, fd->size);
                        break;
                case BASE16:
                        dec = b16_dec(fd->addr, dec_buf, fd->size);
                        break;
                default:
                        errno = EINVAL;
                        goto cleanup;
        }

        if (errno != 0) {
                goto cleanup;
        }

        ofd = open(dst, flags, perms);
        if (ofd == -1) {
                goto cleanup;
        }

        wr = write(ofd, dec_buf, dec);
        if (wr == -1 || (unsigned int)wr != dec) {
                goto cleanup;
        }

        status = 0;

cleanup:
        if (ofd != -1) {
                close(ofd);
        }
        free(dec_buf);
        free_finfo(fd);
        return status;
}


