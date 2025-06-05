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
#include <pwd.h> 

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

#define __UMASK(__mode)\
                        do{\
                            mode_t __old = umask(__mode);\
                            printf("[UMASK] Changed umask: %04o → %04o\n", __old, __mode);\
                        }while(1)
/* 文件信息打印宏 */
/* action 是你传给宏定义的一个字符串常量参数，代表当前执行的操作，比如 "read" 或 "write" */
#define PRINT_FILE_INFO(action, pf)                                   \
                            printf(                                                           \
                                "[File Info]\n"                                               \
                                "├─ File Name    : %s\n"                                      \
                                "├─ Action       : %s\n"                                      \
                                "├─ Action Bytes : %zd bytes\n"                               \
                                "├─ Size         : %ld bytes\n"                               \
                                "├─ Inode        : %ld\n"                                     \
                                "├─ Type         : %s\n"                                      \
                                "├─ RWX          : 0%o\n"                                     \
                                "├─ UID          : %d (%s)\n"                                 \
                                "├─ GID          : %d\n"                                      \
                                "├─ Flags        : 0x%02x\n"                                  \
                                "├─ Offset       : %ld bytes\n"                               \
                                "├─ Atime        : %s\n"                                      \
                                "├─ Mtime        : %s\n"                                      \
                                "└─ Ctime        : %s\n"                                      \
                                ,                                                             \
                                (pf)->__pathname,                                             \
                                action,                                                       \
                                (pf)->ret,                                                    \
                                (pf)->fst->st.st_size,                                        \
                                (pf)->fst->st.st_ino,                                         \
                                file_type_str((pf)->fst->type),                               \
                                (pf)->fst->rwx,                                               \
                                (pf)->fst->st.st_uid,                                         \
                                (pf)->fst->pw ? (pf)->fst->pw->pw_name : "unknown",           \
                                (pf)->fst->st.st_gid,                                         \
                                (pf)->fg,                                                     \
                                (pf)->ofs,                                                    \
                                (pf)->fst->atim,                                              \
                                (pf)->fst->mtim,                                              \
                                (pf)->fst->ctim                                               \
                            );



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

/**
 * @struct __file_stat
 * @brief 封装文件的元信息，包括 stat 结构、类型、权限及时间戳。
 *
 * 用于存储通过 stat 系统调用获取的文件状态信息，并进一步解析出文件类型、
 * 权限（rwx）及可读字符串格式的访问、修改和更改时间。
 */
struct __file_stat {
    struct stat st;   /**< 由 stat() 填充的标准文件状态信息结构体 */
    int type;         /**< 文件类型（如普通文件、目录、符号链接等） */
    int rwx;          /**< 文件权限信息（可按位存储 r/w/x 权限） */
    char atim[100];   /**< 上次访问时间的字符串表示 */
    char mtim[100];   /**< 上次修改时间的字符串表示 */
    char ctim[100];   /**< 上次状态更改时间的字符串表示 */
    struct passwd *pw;
};
/*
struct passwd {
    char   *pw_name;   // 用户名
    char   *pw_passwd; // 用户密码（现在一般为 "x"）
    uid_t   pw_uid;    // 用户ID
    gid_t   pw_gid;    // 组ID
    char   *pw_gecos;  // 用户信息（全名等）
    char   *pw_dir;    // 主目录
    char   *pw_shell;  // 默认shell
};
*/
/**
 * @typedef _file_t
 * @brief 封装文件描述符操作的结构体，用于表示一个打开的文件及其元信息。
 *
 * 此结构体抽象出文件的基本操作相关字段，包括读写缓冲区、读取结果、
 * 偏移量、文件标志、描述符、文件名以及文件状态信息。
 */
typedef struct {
    void *data;                 /**< 数据缓冲区，用于读写文件内容 */
    ssize_t ret;                /**< 实际读取或写入的字节数（有符号） */
    off_t ofs;                  /**< 当前文件偏移量（与 lseek 操作相关） */
    int fg;                     /**< 文件打开标志（如 O_RDONLY, O_RDWR） */
    int fd;                     /**< 文件描述符（由 open 系统调用返回） */
    char *__pathname;           /**< 文件路径名 */
    struct __file_stat *fst;    /**< 指向文件属性信息结构体的指针 */
} _file_t;

/* 相关函数声明 */
void _file_access(const char *__pathname);
int _file_chmod(const char *__pathname, __mode_t __mode);
int _file_get_properties(char *__pathname ,struct __file_stat *fst);
int _file_chown(_file_t *__pathname , uid_t owner, gid_t group);
void _file_close(_file_t *pf);
_file_t* _file_init(char *__pathname);
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

/**
 * @typedef _sfile_t
 * @brief 封装基于标准 I/O 的文件操作结构体。
 *
 * 本结构体用于封装使用标准库函数（如 fopen/fread/fseek 等）进行文件操作时所需的信息。
 * 同时保留了底层文件描述符 `fd`，可支持混合使用低级和高级文件接口。
 */
typedef struct
{
    FILE *pf;       /**< 标准 I/O 文件指针，由 fopen 返回 */
    int fd;         /**< 文件描述符，由 fileno(pf) 或 open 返回 */
    char *path;     /**< 文件路径（可包含完整目录） */
    char *name;     /**< 文件名（可与 path 分离存储） */
    char *md;       /**< 打开模式字符串，如 "r", "w+", "rb" 等 */
    void *ptr;      /**< 用户自定义数据指针，可用于扩展用途 */
    off_t fsz;      /**< 文件大小（单位：字节） */
    size_t ret;     /**< 实际读取或写入的字节数（无符号） */
    long ofs;       /**< 当前偏移量（相对文件开头） */
} _sfile_t;

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
