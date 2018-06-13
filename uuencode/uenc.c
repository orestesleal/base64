/*
 *  uuencode a file given on input as a first argument
 *  and write the uuencoded file to the 2nd argument on the
 *	command line. both binary and text inputs are suported.
 *
 *  Copyright Orestes Leal Rodríguez 2015 <lukes357@gmail.com>
 */
#include <stdlib.h>
#include <string.h>
#include "uu.h"
#include "lib.h"
int main(int argc, char *argv[])
{
	struct finfo *f;
	char *m;
	unsigned int dlen;

	if (argc < 3) {
		return 0;
	}

	f = get_file(argv[1]);
	m = uualloc(f->size, argv[2]);
	dlen = uuenc(f->addr, m, f->size, argv[2]);
	wrfile(argv[2], m, dlen);
	free(m);
	free(f->addr);

	return 0;
}
