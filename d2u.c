/**
 *
 *  DOS to UNIX
 *	Convert CRLF terminated lines in text files coming from
 *	DOS or Windows to LF terminated only (Unix)
 *
 *	this program makes uses of lib.{c,h} which contains some
 *	utils.
 *
 *	Orestes Leal Rodríguez 2015 <olealrd1981@gmail.com>
 *
 *	2015-09-01:		first version
 *  2015-09-02:		command switches for verbose output (-v) and
 *					show help (-h).
 *					d2u now returns a struct with information 
 *					about how many carriage returns are deleted
 *					and the final size (struct d2u_inf).
 *					The program now informs when the input file
 *					is empty (only when -v is given) and proceed
 *					to create the file anyway.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

struct d2u_inf {
	unsigned int size;
	unsigned int cr_rem;
};

struct d2u_inf d2u(char *a, char *b);

int main(int argc, char *argv[])
{
	int wr;
	char *out;
	struct finfo *f;

	if (argc < 3) {
		if (argc == 2 && !strcmp(argv[1], "-h")) {
			printf("usage: %s infile outfile [-v]\n", argv[0]);
		}
		else if (argc == 1) {
			printf("%s -h to see help\n", argv[0]);
		}
		return 0;
	}

	/* f->size is the maximum size of the input file, if one or more 
	   carriage returns are removed, the final size of the output file
	   will be smaller */
	f = rdfile(argv[1]); 
	out = alloc(f->size); 
	wr = wrfile(argv[2], out, d2u(f->addr, out).size); 

	if (argc > 3 && !strcmp(argv[3], "-v")) {
		if (wr > 0) {
			printf("%d carriage returns removed, %d bytes written to %s\n", f->size-wr, wr,argv[2]);
		}
		else {
			printf("%s is empty, however %s was generated anyway\n", argv[1], argv[2]);
		}
	}

	free(f->addr);
	free(out);
	return EXIT_SUCCESS;
}
/* d2u: convert CRLF to LF and return some information 
   about the data processed */
struct d2u_inf d2u(char *a, char *b)
{
	unsigned int size,crm;
	struct d2u_inf ops;
	size = crm = 0;
	while (*a) {
		if (*a != '\r') {
			*b++ = *a;
			size++;
		}
		else {
			crm++;
		}
		a++;
	}
	ops.size = size;
	ops.cr_rem = crm;
	return ops;
}
