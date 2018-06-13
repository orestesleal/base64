#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "base64.h"
void printf_b64(char *b);
int main(int argc, char *argv[])
{
	if (argc < 2) {
		return EXIT_FAILURE;
	}
	FILE *fd;
	char *mp, *mp2;
	size_t fsz;
	struct stat s;

	if (argv[2]) {
		/*
		open file
		get file size
		load into memory
		pass the buffer to Base64_enc
		*/
		if ((fd = fopen(argv[2], "r")) == NULL) {	
			(void)fprintf(stderr, "error: can't open %s\n", argv[2]);
			return EXIT_FAILURE;
		}
		stat(argv[2], &s);
		fsz = s.st_size;
		if ((mp = (char *)malloc(fsz)) == NULL) {
			(void)fprintf(stderr, "error: malloc failed\n");
			return EXIT_FAILURE;
		}
		fread(mp, fsz, 1, fd);
		if ((mp2 = (char *) malloc(fsz*2)) == NULL) {
			(void)fprintf(stderr, "error: malloc failed creating base64 buffer\n");
			return EXIT_FAILURE;
		}
		base64_enc(mp, mp2);
		printf_b64(mp2);
		free(mp);
		free(mp2);
		fclose(fd);
	}


	char *buf16 = (char *)malloc((strlen(argv[1])*2)+1);
	base64_enc(argv[1], buf16);
	printf_b64(buf16);
	free(buf16);

	return EXIT_SUCCESS;
}
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
