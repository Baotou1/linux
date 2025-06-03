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
#define _XOPEN_SOURCE 700
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
#include "time.h"
#include <stdbool.h>

#ifdef __cplusplus
#include <unistd.h>
extern "C" {
#endif

/* 错误打印宏 */
#define PRINT_ERROR() \
    printf("Error at %s:%d, errno = %d: %s\n", __FILE__, __LINE__, errno, strerror(errno))

/**
 * @brief 将文件类型（mode）转换为可读字符串。
 *
 * @param mode 文件的 st_mode 字段或文件类型宏（如 S_IFREG）
 * @return 对应的英文类型字符串（如 "Regular File"）
 */
static const char* file_type_str(int mode) {
    switch (mode & S_IFMT) {
        case S_IFREG:  return "Regular File";
        case S_IFDIR:  return "Directory";
        case S_IFLNK:  return "Symbolic Link";
        case S_IFIFO:  return "FIFO";
        case S_IFCHR:  return "Character Device";
        case S_IFBLK:  return "Block Device";
        case S_IFSOCK: return "Socket";
        default:       return "Unknown";
    }
}
/* 文件信息打印宏 */
/* action 是你传给宏定义的一个字符串常量参数，代表当前执行的操作，比如 "read" 或 "write" */
#define PRINT_FILE_INFO(action, pf) \
                                    printf( \
                                        "file info:\n"\
                                        "" action " %s .\n" \
                                        "  -> " action " bytes: %zd bytes\n" \
                                        "  -> size: %ld bytes\n" \
                                        "  -> inode: %ld \n" \
                                        "  -> type: %s \n" \
                                        "  -> rwx: 0%o \n" \
                                        "  -> flag: 0x%02x\n" \
                                        "  -> now offset: %ld bytes\n"\
                                        "  -> atim: %s\n" \
                                        "  -> mtim: %s\n" \
                                        "  -> ctim: %s\n" \
                                        "\n"\
                                        ,\
                                        (pf)->name, (pf)->ret, (pf)->st.st_size, (pf)->st.st_ino, file_type_str((pf)->type)\
                                        ,(pf)->rwx ,(pf)->fg ,(pf)->ofs\
                                        ,(pf)->atim ,(pf)->mtim ,(pf)->ctim\
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

/* _file_t结构体，底层fd操作封装 */
/**< ssize_t 是有符号类型，size_t 是无符号类型 */
typedef struct {
    void *data;      /**< 数据缓冲区 */
    ssize_t ret;     /**< 读取的字节数 */
    off_t ofs;       /**< 当前文件偏移量 */
    int fg;          /**< 文件打开标志 */
    int fd;          /**< 文件描述符 */
    char *name;      /**< 文件名 */
    struct stat st;  /**< 文件属性 */
    int type;        /**< 文件类型 */
    int rwx;         /**< 文件权限 */
    char atim[100];
    char mtim[100];
    char ctim[100];
} _file_t;

/* 相关函数声明 */
int _file_get_properties(_file_t *pf);
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

/******************************************************************************************************************/
/* 文件打开标志检查宏示例 */
#define CHECK_FOPEN_MODE(md) (\
    (md != NULL) && \
        (\
            strcmp(md ,"r") == 0 || strcmp(md ,"r+") == 0 \
                || strcmp(md ,"w") == 0 || strcmp(md ,"w+") == 0 \
                    || strcmp(md ,"a") == 0 || strcmp(md ,"a+") == 0 \
        )\
    )

/* 释放_sfile_t的宏 */
#define FREE_SFILE(psf) \
    do{\
        if((psf) != NULL){\
            free((psf)->name);\
            free((psf)->path);\
            free((psf)->ptr);\
            free((psf));\
        }\
    }while(0)
         
/* 文件信息打印宏 */
#define PRINT_SFILE_INFO(action, psf) \
                                        printf( \
                                            "file info:\n" \
                                            "" action " %s ok:\n" \
                                            "  -> file fd: %d\n" \
                                            "  -> file offset: %ld bytes\n" \
                                            "  -> file mode: %s \n" \
                                            "  -> file size: %ld bytes\n" \
                                            "  -> " action " bytes: %zd bytes\n\n", \
                                            (psf)->name, (psf)->fd, (psf)->ofs, (psf)->md, (psf)->fsz, (psf)->ret \
                                         )

/* _sfile_t结构体，基于标准IO的封装 */
typedef struct
{
    FILE *pf;
    int fd;
    char *path;
    char *name;
    char *md;
    void *ptr;
    off_t fsz;
    size_t ret;
    long ofs;
}_sfile_t;

/* 相关函数声明 */
int _sfile_fflush(_sfile_t *psf);
_sfile_t* _sfile_finit(const char *path ,const char *name ,const char *md);
int _sfile_fopen(_sfile_t *psf);
void _sfile_fclose(_sfile_t *psf);
int _sfile_fwrite(_sfile_t *psfw ,const void *ptr ,size_t sz, size_t nmemb ,long ofs ,int whence);
int _sfile_fread(_sfile_t *psfr ,size_t sz, size_t nmemb ,long ofs ,int whence);
int _sfile_print(_sfile_t *psfp ,size_t sz, size_t nmemb ,long ofs ,int whence);
#ifdef __cplusplus
}
#endif

#endif /* __FILE_H */
