#ifndef __FILE_H
#define __FILE_H
/**
 * @file    file.h
 * @brief   File utility interface for encapsulated file I/O operations.
 *
 * This header defines the `_file_t` structure and provides utility functions 
 * for initializing, opening, reading, writing, printing, and closing files.
 * 
 * The interface simplifies file management by wrapping system calls like 
 * `open`, `read`, `write`, and `lseek`, while tracking metadata such as file
 * name, size, offset, return count, and a dynamic data buffer.
 *
 * @author  baotou
 * @date    2025-05-16
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define PRINT_ERROR() \
    //printf("Error at %s:%d, errno = %d: %s\n", __FILE__, __LINE__, errno, strerror(errno))
#define PRINT_ERROR(msg)  perror(msg)

#define CREAT_NEWFILE   O_CREAT | O_EXCL

typedef struct {
    char *name;      /**< File name */
    int fd;          /**< File descriptor */
    void *data;      /**< Read/write buffer */
/* ssize_t - signed, size_t - unsigned */
    ssize_t ret;     /**< Number of bytes read or written */
    size_t size;     /**< Buffer size */
    off_t ofs;       /**< Current file offset */
    int flag;        /**< File open flags */
    mode_t mode;     /**< File permission mode */
} _file_t;

/* API declarations */
_file_t* _file_init(char *name, size_t size);
int _file_open(_file_t *pfile, int flag, mode_t mode);
void _file_close(_file_t *pfile);
int _file_read(_file_t *pfile, off_t offset, int whence);
int _file_write(_file_t *src, _file_t *dst, off_t offset, int whence);
void _file_print(_file_t *pfile);

/* Optional utility */
off_t _file_get_offset(int *fd);
off_t _file_set_offset(int *fd, off_t offset, int whence);

#ifdef __cplusplus
}
#endif

#endif /* __FILE_H */
