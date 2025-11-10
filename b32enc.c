/*
 *      encode an input file using base32 and write the encoded output
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
#include "base64.h"
int main(int argc, char *argv[])
{
        struct finfo *fd;
        char *enc_buf;
        int ofd;
        ssize_t wr;
        size_t out_len;

        if (argv[1]) {
                if ((fd = get_file(argv[1])) == NULL) {
                        perror("get_file");
                        return EXIT_FAILURE;
                }
                if (argv[2]) {
                        size_t groups = (fd->size + 4) / 5;
                        size_t buf_len = groups * 8 + 1;
                        enc_buf = alloc(buf_len);
                        if (enc_buf == NULL) {
                                perror("alloc");
                                free_finfo(fd);
                                return EXIT_FAILURE;
                        }
                        b32_enc((const unsigned char *)fd->addr, (unsigned char *)enc_buf, fd->size);

                        if ((ofd = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | O_TRUNC | S_IWUSR)) == -1) {
                                perror("open");
                                free(enc_buf);
                                free_finfo(fd);
                                return EXIT_FAILURE;
                        }
                        out_len = strlen(enc_buf);
                        wr = write(ofd, enc_buf, out_len);
                        if (wr == -1 || (size_t)wr != out_len) {
                                perror("write");
                                close(ofd);
                                free(enc_buf);
                                free_finfo(fd);
                                return EXIT_FAILURE;
                        }
                        close(ofd);
                        free(enc_buf);
                }
                free_finfo(fd);
        }
        return EXIT_SUCCESS;
}
