/*
 *  decode a uuencoded file given on first argument and
 *  write the original file that was encoded to the file
 *  given on the second argument, both binary and text
 *	inputs are supported.
 *
 *	Copyright Orestes Leal Rodríguez 2015 <lukes357@gmail.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lib.h"
#include "uu.h"
int main(int argc, char *argv[])
{
	struct finfo *f;
	char *c,*d;
	int s;

	if (argc < 3) {
		return 0;
	}

	f = get_file(argv[1]);
	if ((c = eval_uu_file(f->addr)) != NULL) {
		d = alloc(strlen(c));
		s = uudec((unsigned char *)c, d, ulen(c+1));
		wrfile(argv[2],d,s); 
		free(d);
	}
	else {
		fprintf(stderr, "error: %s is not a uuencoded file\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	free(f->addr);
	return 0;
}
