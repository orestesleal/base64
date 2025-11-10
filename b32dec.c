/*
 *      decode a base32 encoded file and write the decoded output
 *      to another file.
 *
 *  Copyright Orestes Leal Rodriguez 2015-2025 <lukes357@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "base64.h"
int main(int argc, char *argv[])
{
        struct finfo *fd;
        char *dec_buf;
        int ofd;
        ssize_t wr;
        unsigned int dec;

        if (argv[1]) {
                if ((fd = get_file(argv[1])) == NULL) {
                        perror("get_file");
                        return EXIT_FAILURE;
                }

                if (argv[2]) {
                        dec_buf = alloc(fd->size + 1);
                        if (dec_buf == NULL) {
                                perror("alloc");
                                free_finfo(fd);
                                return EXIT_FAILURE;
                        }
                        errno = 0;
                        dec = b32_dec((const unsigned char *)fd->addr, dec_buf, fd->size);
                        if (errno != 0) {
                                perror("b32_dec");
                                free(dec_buf);
                                free_finfo(fd);
                                return EXIT_FAILURE;
                        }

                        if ((ofd = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | O_TRUNC | S_IWUSR)) == -1) {
                                perror("open");
                                free(dec_buf);
                                free_finfo(fd);
                                return EXIT_FAILURE;
                        }
                        wr = write(ofd, dec_buf, dec);
                        if (wr == -1 || (unsigned int)wr != dec) {
                                perror("write");
                                close(ofd);
                                free(dec_buf);
                                free_finfo(fd);
                                return EXIT_FAILURE;
                        }
                        close(ofd);
                        free(dec_buf);
                }
                free_finfo(fd);
        }
        return EXIT_SUCCESS;
}
