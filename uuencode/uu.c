/**
 * Implementation of 'Unix-to-Unix' (uu) encoding/decoding
 * utilities.
 *
 * Copyright Orestes Leal Rodriguez 2015 <lukes357@gmail.com>
 *
 *	all utilities make use of uu.c (main uuencoding and decoding
 *	API) and lib.c (small general purpose utility functions)
 *  therefore to build you must compile using this .c files,
 *	i.e: `gcc infile.c lib.c uu.c -o outfile -std=c99 -Wall`
 *
 * -devlog
 *		Aug 23: uuencoding added to uu.c
 *		Aug 24: uudecoding added to uu.c
 *		Aug 25: 'uudecode' now checks for invalid inputs outside
 *			    the standard uuencode alphabet.
 *				The 'uuencode' and 'uudecode' utilities are for 
 *				encode and decode text strings from the command line,
 *				not files.
 *		Aug 27: Added function: 'uenc', which encode using the 
 *				uuencode header and termination format (begin...end). 
 *				Added utilities functions for writing uuencoded data
 *				to a file. 
 *				Fixed a bug that causes incorrect encoding when  
 *				iso8859-1 Latin charset characters appears in the 
 *				input of the encoding functions.
 *		Aug 29: added 'eval_uu_file' a validator of uuencoded files
 *				using the uuencoded header and terminator format
 *		Aug 30:	A lot of rework and addition of the command line 
 *				utilities 'udec' and 'uenc' for encoding and decoding
 *				files (binary or text). 
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include "lib.h"

#define UU_INP_INVAL "error: %d -> %c is not an uuencode alphabet character.\n"
typedef unsigned int u32_t;
void setln(char *b, u32_t *nbc, u32_t *w, u32_t *cpl);

/* 
 * uuencode: uuencode a line of text 
 * s: string to encode
 * b: destination buffer
 * len: length of string to encode
 */
void uuencode(unsigned char *s, char *b, unsigned int len)
{
	unsigned int i,w;
	unsigned char t,x,save,get;

	w = 0;
	get = 6;
	save = 2;
	x = 0;
	for (i = 0; i < len; i++) {
		t = ((s[i] >> save) | x) + 0x20;
		b[w++] = t + (t == 0x20 ? 64: 0); 
		get -= 2;
		save += 2;
		x = (s[i] << get) & 0x3f;

		if (save == 8) {
			b[w++] = x + (x == 0 ? 64: 0) + 0x20; 
			get = 6;
			save = 2;
			x = 0;
		}
	}
	/* uuencode the last six bits if needed */ 
	if (get != 6) {
		b[w++] = x + (x == 0 ? 64 : 0) + 0x20;
	}
	/* check if padding is needed.. */
	t = len % 3;
	if (t) {
		t = (t == 1) ? 2 : 1;
		for (i = 0; i < t; i++) {
			b[w++] = '`';
		}
	}
	b[w] = '\0';
}
/* 
 * uudecode: decode a uuencoded string 
 * s: uuencoded string to decode 
 * b: destination buffer for decoded data
 * len: length of string to decode 
 */
int uudecode(unsigned char *s, char *b, unsigned int len)
{
	unsigned char movr,ileft,t,x;
	unsigned int i,w;

	ileft = 2;
	movr = 6;
	w = 0;
	for (i = 0; i < len; i++) {
		if ((s[i] < ' ' || s[i] > '_') && 
			 s[i] != '`') {
			fprintf(stderr, UU_INP_INVAL, i,s[i]);
			return -1;
		}
		t = (s[i] - 0x20) << ileft;
		movr -= 2; 
		ileft += 2;

		if (s[i+1] != '`') {
			x = (s[i+1] - 0x20) >> movr;
			b[w++] = t |= x;
		}
		else {
			b[w++] = t;
		}
		if (movr == 0) {
			ileft = 2;
			movr = 6;
			++i;
		}
	}
	b[w] = '\0';
	return 0;
}
/* uuencode an input file loaded in 's' to writting 
   the encoded characters to the buffer 'b', also
   the uuencode header is added to the resulting 
   output */
unsigned int uuenc(unsigned char *s, char *b, unsigned int len, char *f)
{
	unsigned int i,w,cpl,nbc;
	unsigned char t,x,save,get;

	w = 1;
	get = 6;
	save = 2;
	x = cpl = nbc = 0; 

	/* insert the uuencoded header */
	b += sprintf(b, "begin 644 %s\n", f);

	for (i = 0; i < len; i++) {
		t = ((s[i] >> save) | x) + 0x20;
		b[w++] = t;
		get -= 2;
		save += 2;
		x = (s[i] << get) & 0x3f;
		cpl++;

		if (cpl == 60) {
			setln(b, &nbc, &w, &cpl);
		}

		if (save & 8) {
			b[w++] = x + 0x20; 
			get = 6;
			save = 2;
			x = 0;

			++cpl;
			if (cpl == 60) {
				setln(b, &nbc, &w, &cpl);
			}
		}
	}
	if (get != 6) { /* uuencode the last six bits if needed */ 
		b[w++] = x + (x == 0 ? 64 : 0) + 0x20;
		cpl++;
	}

	/* compute the number of bytes encoded (last line) */
	b[nbc] = ((cpl*6) >> 3) + 0x20; 

	/* if the input is not an integral group of 3, add padding */
	t = len % 3;
	if (t) {
		t = (t == 1) ? 2 : 1;	
		for (i = 0; i < t; i++) {
			b[w++] = '`';
		}
	}
	/* write the last part of the uuencoded identification or
	   non-data part */ 
	b[w++] = '\n';  
	b[w++] = '`'; 
	b[w++] = '\n';  
	w += sprintf(b+w,"end");
	b[w] = '\0';

	/* compute and return the total length of file */
	return w + strlen("begin 644 ") + strlen(f) + 1;;
}

/* uudecoder, decode uu encoded files */
int uudec(unsigned char *s, char *b, unsigned int len)
{
	unsigned char movr,ileft,t,x;
	unsigned int i,w;
	unsigned int cn;

	ileft = 2;
	movr = 6;
	w = 0;

	/* cn holds the number of encoded bytes per line,
	   at the end of the procedure it should contain
	   the exact number of bytes of the original file */
	cn = s++[0]-32;

	for (i = 0; i < len; i++) {
		/* end of line found, set i one count ahead to skip the
		   first char of the next line (number of bytes encoded on
		   the line) and points to the first real encoded byte
		   of the line to decode */
		if (s[i] == '\n') {
			++i; 
			cn += s[i]-32; /* update number of encoded bytes */
			continue;
		}

		if ((s[i] < ' ' || s[i] > '_') && 
			 s[i] != '`') {
			fprintf(stderr, UU_INP_INVAL, i,s[i]);
			return -1;
		}

		t = (s[i] - 0x20) << ileft;
		movr -= 2; 
		ileft += 2;

		/* do not encode '`' since it means 0 byte */
		if (s[i+1] == '`') {
			goto rest_cycle;
		}
		else {
			x = (s[i+1] - 0x20) >> movr;
			b[w++] = t |= x;
		}
rest_cycle:
		if (movr == 0) {
			ileft = 2;
			movr = 6;
			++i;
		}
	}
	return cn;

}

/*  used when each line of uuencoded output has 60 bytes, 
	this handles setting the first byte as 'M', write a 
	newline at end of line, update the position of 
	buffers and setup the next location of the first byte 
	of the line */
void setln(char *b, u32_t *nbc, u32_t *w, u32_t *cpl)
{
	b[*nbc] = 'M'; 	
	b[*w] = '\n'; 		
	(*w)++;
	*nbc = *w; 	/* setup the position of the 1st byte on the next line */
	(*w)++; 	/* setup the first position for uuencoded data  */ 
	*cpl = 0;  	/* restart the byte counter */
}

/* evaluate if the uuencoded file loaded into 'b' has a valid
   syntax (begin, mode, end, etc..) and adjust the buffer and
   the address returned in a way the encoded data is the only 
   data accessible for the decoding routine */ 
char *eval_uu_file(char *b) /* lib.c */
{
	char *sx;
	int p;

	if ((srch(b, "begin ") >= 0) && 
		(srch(b, "\n") >= 0) && 
		((sx = dosrch(b, "\n`\nend"))) != NULL) {
		*sx = '\0';
		p = srch(b, "\n");
		return b+p+1; 
	}
	return NULL;
}

/* specific allocation routine to allocate memory for
   a uuencoded file with the classic format (begin..end) */
char *uualloc(unsigned int size, char *out)
{
	char *p;
	p = (char *)malloc((size*2) + 16 + strlen(out));

    if (p == NULL) {
		fprintf(stderr, "malloc(3) error: can't allocate %d bytes\n", size);
		exit(EXIT_FAILURE);
	}
	return p;
}
