/*
 *	test vector for encoding an input file with b32_enc into base32
 *	and write the encoded contents to a file.
 *
 *  Copyright Orestes Leal Rodríguez 2015 <lukes357@gmail.com>
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
int main(int argc, char *argv[])
{
	struct finfo *fd;
	char *enc_buf;
	int ofd;
	size_t wr;

	if (argv[1]) {
		/* load the input file */
		if ((fd = get_file(argv[1])) == NULL) {
			fprintf(stderr, "get_file() failed loading %s in main memory\n", argv[1]);
			exit(EXIT_FAILURE);
		}
		if (argv[2]) {
			/* encode and write the encoded file into argv[2] */
			enc_buf = alloc(fd->size*2); /* buffer for the base32 encoding string */
			b32_enc(fd->addr, enc_buf, fd->size);

            if ((ofd = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | O_TRUNC | S_IWUSR)) == -1) {
            	fprintf(stderr, "open(2) error, can't create or truncate %s\n", argv[2]);
	        	exit(EXIT_FAILURE);
			}
			if ((wr = write(ofd, enc_buf, strlen(enc_buf))) == -1) {
				fprintf(stderr, "write(2) error, can't write to %s\n", argv[2]);
				exit(EXIT_FAILURE);
			}
			close(ofd);
			free(enc_buf);
        }
		free(fd);
	}
	return EXIT_SUCCESS;
}
