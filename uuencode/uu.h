void uuencode(char *s, char *b, unsigned int len);
int uudecode(char *s, char *b, unsigned int len);
unsigned int uuenc(char *s, char *b, unsigned int len, char *f);
int uudec(unsigned char *s, char *b, unsigned int len);
char *uualloc(unsigned int size, char *out);
char *eval_uu_file(char *b);
