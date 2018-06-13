struct finfo {
	char *addr;  /* file starting address */
	size_t size; /* size of file */
};
int srch(const char *s, const char *src);
char * dosrch(const char *s, const char *src);
char * srcha(const char *s, const char *src);
struct finfo *get_file(const char *f);
struct finfo *nrd_file(const char *f, unsigned int c);
int wrfile(const char *dst, const char *src, unsigned int len);
char *alloc(unsigned int size);
unsigned int ulen(char *s);
#define rdfile get_file /* alias for get_file */
int getfd_ro(const char *f);
int getfd_rw(const char *f);
size_t fsize(const char *p);
