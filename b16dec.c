/*
 *	test vector for decoding a base16 encoded file and 
 *	write to disk the decoded file using b16_dec
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
	char *dec_buf;
	int ofd;
	size_t wr;
	size_t dec; /* number of decoded bytes */
	int flags = O_CREAT | O_RDWR;
	int perms = S_IRUSR | O_TRUNC | S_IWUSR;

	if (argv[1]) {
		/* load the encoded file */
		if ((fd = get_file(argv[1])) == NULL) {
			fprintf(stderr, "get_file() failed loading %s in main memory\n", argv[1]);
			exit(EXIT_FAILURE);
		}

		if (argv[2]) {
			/* allocate buffer and decode */
			dec_buf = alloc(fd->size); 

			/*  decode the base32 string on 'dec_buf' */
			dec = b16_dec(fd->addr, dec_buf, fd->size); 

            if ((ofd = open(argv[2], flags, perms)) == -1) {
            	fprintf(stderr, "open(2) error, can't create or truncate %s\n", argv[2]);
	        	exit(EXIT_FAILURE);
			}

			/* write the contents of `dec_buf` into the file requested on argv[2] */
			if ((wr = write(ofd, dec_buf, dec)) == -1) {
				fprintf(stderr, "write(2) error, can't write to %s\n", argv[2]);
				exit(EXIT_FAILURE);
			}
			close(ofd);
			free(dec_buf);
        }
		free(fd);
	}
	return EXIT_SUCCESS;
}
