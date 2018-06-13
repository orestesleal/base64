/*
 *	gendb.c will conform an entry for use in udb.db in Base64 from the input email 
 *	and password using the base64.c code
 *	
 *	i.e: ./gendb "\0lukes357@gmail.com" "\0holamundo"
 *	will produce: "[lukes357@gmail.com AGx1a2VzMzU3QGdtYWlsLmNvbQBob2xhbXVuZG8=]"
 *  which is an entry for smtpc.c
 *
 *	Also the Base64 part can be used as an authentication string on AUTH PLAIN 
 *	sequences to do this kind of smtp authentication.
 *
 *	Orestes Leal Rodriguez 2015 <lukes357@gmail.com>
 *
 *	-devlog
 *		created on Aug 3 2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
void catstr(char s1[], char s2[], char *o);
int main(int argc, char *argv[])
{
	if (argv[1] && argv[2]) {
		char outbuf[(strlen(argv[2])*2)+1 + (strlen(argv[1])*2)+1];
		char ibuf[strlen(argv[1]) + strlen(argv[2])]; 
		catstr(argv[1], argv[2], ibuf);
		base64_enc(ibuf, outbuf);
		printf("[%s %s]\n", argv[1][0] == '\\' ? argv[1]+=2 : argv[1], outbuf);
	}
	else {
		printf("%s \"\\0email@domain\" \"\\0password\"\n", argv[0]);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
/* a version of strcat */
void catstr(char s1[], char s2[], char *o)
{
	int i,j;
	for (i = 0; s1[i]; i++) {
		o[i] = s1[i];
	}
	for (j = 0; s2[j]; j++, i++) {
		o[i] = s2[j];
	}
	o[i] = '\0';
}
