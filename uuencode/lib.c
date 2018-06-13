/*
 *  This file contains a group of utilities for handling recurrent 
 *  tasks in C programming under Unix.	
 *
 *  Copyright Orestes Leal Rodríguez 2015 <lukes357@gmail.com>
 *
 *	-devlog
 *		2015-08-29:		[srch, dosrch, srcha] - string search 
 *						[get_file] - load files into memory
 *		2015-08-31:		[wrfile] - write files to disk
 *						[alloc] - wrapper around malloc(3)
 *					    [ulen] - a strlen(3) like function
 *		2015-09-02:		wrfile now returns the number of bytes
 *						written by write(2)
 *		2015-09-02:		lib.h now has 'rdfile' defined as an alias 
 *						to get_file	which is more coherent with his 
 *						task which is read and load files.
 *		2015-09-04:		[fsize] get file size of a file
 *						[getfd_rw, getfd_ro] get file descriptors
 *						from the system ready for read and write. 
 *		2015-09-12:		[nrd_file] read a file using a byte count
 *						for reading the number of specified bytes
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

/* search for a string, return a position on the buffer
   `src' where `s' begins, return -1 if nothing found */
int srch(const char *s, const char *src)
{
	int i,w;

	for (i = 0; s[i]; i++) {
		for (w = 0; (s[w+i] == src[w]) && src[w]; w++);
		if (src[w] == '\0') {
			return i;
		}
	}
	return -1;
}
/* search for a string, return a pointer to the buffer
   src in the position where the string `s' begins,
   returns NULL if nothing found */
char * dosrch(const char *s, const char *src)
{
	int i,w;

	for (i = 0; s[i]; i++) {
		for (w = 0; (s[w+i] == src[w]) && src[w]; w++);
		if (src[w] == '\0') {
			return (char *)s+i; 
		}
	}
	return NULL;
}
/* search and return the position after the string
   found, otherwise return null */
char *srcha(const char *s, const char *src)
{
	int i,w;

	for (i = 0; s[i]; i++) {
		for (w = 0; (s[w+i] == src[w]) && src[w]; w++);
		if (src[w] == '\0') {
			return (char *)s+w+i; 
		}
	}
	return NULL;
}
/* open a file, get file info, load it to memory and return file 
   information on memory as a pointer to a structure 'finfo', 
   returns NULL indicating some kind of error. 
   The caller is responsible for calling free(3) */
struct finfo *get_file(const char *f)
{
	int fd;
	struct stat fp;
	char *addr; 
	struct finfo *st_addr;
	size_t rd;

	if ((fd = open(f, O_RDONLY)) == -1) {
		fprintf(stderr, "open(2) failed for file %s\n", f);
		return NULL;
	}
	stat(f, &fp); /* get file info with stat(2) */
	if ((addr = (char *)malloc(fp.st_size)) == NULL) {
		fprintf(stderr, "malloc(3) failed allocating %d bytes\n", (int)fp.st_size);
		close(fd);
		return NULL;
	}
	if ((rd = read(fd, addr, fp.st_size)) == -1) {
		fprintf(stderr, "read(2) failed reading %d bytes from %s\n", (int)fp.st_size, f);
		close(fd);
		free(addr);
		return NULL;
	}
	if ((st_addr = (struct finfo *)malloc(sizeof(struct finfo))) == NULL) {
		fprintf(stderr, "malloc(3) failed allocating the returned structure\n");
		close(fd);
		free(addr);
		return NULL;
	}
	close(fd);
	st_addr->addr = addr;
	st_addr->size = fp.st_size;
	return st_addr;
}
/* write the file contents loaded in 'src' to 'dst'
   writing 'len' bytes */
int wrfile(const char *dst, const char *src, unsigned int len)
{
	int fd;
	size_t w;
	int flags = O_CREAT | O_RDWR;
	int perms = S_IRUSR | O_TRUNC | S_IWUSR;

	if ((fd = open(dst, flags, perms)) == -1) {
		fprintf(stderr, "open(2): error, can't create %s\n", dst);
		exit(EXIT_FAILURE);
	}
	if ((w = write(fd, src, len)) == -1) {
		fprintf(stderr, "write(2): error, can't write to %s\n", dst);
		exit(EXIT_FAILURE);
	}
	close(fd);
	return w;
}
/* if memory allocation is ok return the address
   if not inform and exit */
char *alloc(unsigned int size)
{
	char *ptr = (char *)malloc(size);
   	if (ptr == NULL) {
		fprintf(stderr, "malloc: error, can't allocate %d bytes\n", size);
		exit(EXIT_FAILURE);
	}
	return ptr;
}
unsigned int ulen(char *s)
{
	unsigned int i;
	for (i = 0; *s; i++, s++);
	return i;
}
/* get an open file descriptor for reading the file 
   whose path is passed as `f' */
int getfd_ro(const char *f) 
{
	int fd;

	if ((fd = open(f, O_RDONLY)) == -1) { 
		fprintf(stderr, "getfd_ro: open(2) failed opening %s\n",f);
		exit(EXIT_FAILURE);
	}
	return fd;
}
/* 	get an open file descriptor for RW on
	the file whose path is passed here in `f' */
int getfd_rw(const char *f) 
{
	int fd;
	int perms = S_IRUSR | O_TRUNC | S_IWUSR;
	int flags = O_RDWR | O_CREAT | O_TRUNC;

	if ((fd = open(f, flags, perms)) == -1) { 
		fprintf(stderr, "getfd_rw: failed doing open(2) on %s\n",f);
		exit(EXIT_FAILURE);
	}
	return fd;
}
/* get file size calling stat(2) */
size_t fsize(const char *p)
{
	struct stat fs;
	int r;

	if ((r = stat(p,&fs)) != 0) {
		fprintf(stderr, "fsize: failed doing stat(2) on %s\n",p);
		exit(EXIT_FAILURE);
	}
	return fs.st_size;
}

/* open and read at most `c' bytes from the file pointed
   by 'f', then return the memory address of the beginnning
   of the file */ 
struct finfo *nrd_file(const char *f, unsigned int c)
{
	int fd;
	struct stat fp;
	char *addr; 
	struct finfo *st_addr;
	size_t rd;

	if ((fd = open(f, O_RDONLY)) == -1) {
		fprintf(stderr, "open(2) failed for file %s\n", f);
		return NULL;
	}
	stat(f, &fp); /* get file info with stat(2) */

	if (c < fp.st_size) {
		fp.st_size = c;
	}

	if ((addr = (char *)malloc(fp.st_size)) == NULL) {
		fprintf(stderr, "malloc(3) failed allocating %d bytes\n", (int)fp.st_size);
		close(fd);
		return NULL;
	}
	if ((rd = read(fd, addr, fp.st_size)) == -1) {
		fprintf(stderr, "read(2) failed reading %d bytes from %s\n", (int)fp.st_size, f);
		close(fd);
		free(addr);
		return NULL;
	}
	if ((st_addr = (struct finfo *)malloc(sizeof(struct finfo))) == NULL) {
		fprintf(stderr, "malloc(3) failed allocating the returned structure\n");
		close(fd);
		free(addr);
		return NULL;
	}
	close(fd);
	st_addr->addr = addr;
	st_addr->size = fp.st_size;
	return st_addr;
}
