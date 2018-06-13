/* 
 *	using base64.c to encode and decode binary or text files
 *  passed as the first argument on the command line, the
 *	output is a base64 representation the content of the input
 *	file, *	if a second argument encoded base64 representation
 *	of the input file in the first argument is written to the file
 *	specified on the second for testing the correct decoding of the
 *	b64_dec function. 
 *
 *  Orestes Leal Rodríguez 2015 <lukes357@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include "base64.h"
void printf_b64(char *b);
char *alloc(unsigned int size);
int main(int argc, char *argv[])
{
	int fd,fd2;
    char *mp, *mp2, *mp3;
    size_t fsz;
    struct stat s;
	int bdec;
	ssize_t wr;

	if (argv[1]) {
		if ((fd = open(argv[1], O_RDONLY)) == -1) {	
			(void)fprintf(stderr, "error: can't open %s\n", argv[1]);
			exit(EXIT_FAILURE);
		}
		stat(argv[1], &s);
		fsz = s.st_size;
		mp = alloc(fsz);

		if (read(fd, mp, fsz) == -1) {
			printf("read(2) error\n"),
			exit(EXIT_FAILURE);
		}
		mp2 = alloc(fsz*2);
		mp3 = alloc(fsz);

		b64_enc(mp, mp2, fsz);
		/*	printf_b64(mp2); */
		printf("info: input has %d bytes, ", fsz);
		bdec = b64_dec(mp2, mp3, strlen(mp2));
		printf("%d bytes decoded%s", bdec, (argv[2]) ? ", " : "\n");

		if (argv[2]) {
			if ((fd2 = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
				printf("open(2) error, can't open %s\n", argv[2]);
				exit(EXIT_FAILURE);
			}
			if ((wr = write(fd2, mp3, bdec)) == -1) {
				fprintf(stderr, "write(2) error, can't write %s\n", argv[2]);
				exit(EXIT_FAILURE);
			}
			printf("written %d bytes to %s\n", wr,argv[2]);
		}
		free(mp);
		free(mp2);
		free(mp3);
		close(fd);
		argv[2] ? close(fd2) : 0;
	}
	return EXIT_SUCCESS;
}
/* print a base64 encoded representation
   as a 72 colum lines */
void printf_b64(char *b)
{
	int c, i;
	c = 0;
	for (i = 0; b[i]; i++, c++) {
		if (c < 72) {
			printf("%c", b[i]);
		}
		else {
			printf("\n");
			printf("%c", b[i]);
			c = 0;
		}
	}
	printf("\n");
}

char *alloc(unsigned int size)
{
	char *ptr = (char *)malloc(size);
	if (ptr == NULL) {
		fprintf(stderr, "malloc: error, can't allocate %d bytes of memory\n", size);
		exit(EXIT_FAILURE);
	}
	return ptr;
}
