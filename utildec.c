#include <stdio.h>
#include <stdlib.h>
#include "base64.h"
int main(int argc, char *argv[])
{
	if (argc > 2) {
		decode_rd_file(argv[1], argv[2], BASE32);
	}
	return 0;
}
