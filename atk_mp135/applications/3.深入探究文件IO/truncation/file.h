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
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>


#ifdef __cplusplus
#include <unistd.h>
extern "C" {
#endif

#define PRINT_ERROR() \
    printf("Error at %s:%d, errno = %d: %s\n", __FILE__, __LINE__, errno, strerror(errno))
//#define PRINT_ERROR(msg)  perror(msg)

//action 是你传给宏定义的一个字符串常量参数，代表当前执行的操作，比如 "read" 或 "write"
#define PRINT_FILE_INFO(action, pf) \
                                    printf( \
                                        "" action " %s .\n" \
                                        "  -> " action " bytes: %zd bytes\n" \
                                        "  -> file size: %ld bytes\n" \
                                        "  -> file offset: %ld bytes\n"\
                                        "  -> file flag: 0x%02x\n\n" ,\
                                        (pf)->name, (pf)->ret, (pf)->fs, (pf)->ofs, (pf)->fg \
                                    )


#define CREAT_NEWFILE   O_CREAT | O_EXCL
#define CP_FILE_DUP_1       0x01
#define CP_FILE_DUP_2       0x02
#define CP_FILE_FCNTL_3     0x03
#define FILE_TRUNCATE       0x01
#define FILE_F_TRUNCATE     0x02
#define HAS_INVALID_F_SETFL_FLAGS(flag) \
                                    ((flag) & (O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_EXCL | O_NOCTTY | O_TRUNC))

#define FILE_ERROR      0x01
#define FILE_EOK        0x00

/**< ssize_t 是有符号类型，size_t 是无符号类型 */
typedef struct {
    void *data;      /**< 数据缓冲区 */
    ssize_t ret;     /**< 读取的字节数 */
    size_t fs;       /**< 文件大小 */
    off_t ofs;       /**< 当前文件偏移量 */
    int fg;          /**< 文件打开标志 */
    int fd;          /**< 文件描述符 */
    char *name;      /**< 文件名 */
} _file_t;

/* API declarations */
int _file_get_size(_file_t *pf);
int _file_data_init(_file_t *pf ,size_t size);
void _file_close(_file_t *pf);
_file_t* _file_init(char *name);
int _file_open(_file_t *pf ,int fg ,mode_t md);
int _file_read(_file_t *pfr ,off_t ofs ,int whence ,size_t len);
int _file_write(_file_t *pfw ,void *data ,off_t ofs ,int whence ,size_t len);
int _file_pread(_file_t *pfr ,size_t len ,off_t ofs);
int _file_pwrite(_file_t *pfw ,void *data ,size_t len ,off_t ofs);
int _file_cpfd(_file_t *pf ,_file_t *cppf ,int flag ,int nfd);
int _file_status_fcntl(_file_t *pf ,int cmd, ...);
int _file_truncate(_file_t *pf ,off_t len ,off_t ofs ,int cmd ,...);
int _file_print(_file_t *pfp ,off_t ofs ,size_t len);
int _file_print_u16(_file_t *pfp ,off_t ofs ,size_t len);
#ifdef __cplusplus
}
#endif

#endif /* __FILE_H */
