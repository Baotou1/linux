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
#include <sys/time.h>
#include "time.h"
#include <utime.h>
#include <stdbool.h>
#include <pwd.h> 
#include <dirent.h>

#ifdef __cplusplus
#include <unistd.h>
extern "C" {
#endif
/**
 * @macro    __UMASK
 * @brief    设置进程的文件权限掩码（umask）并打印修改前后的值。
 */
#define __UMASK(__mode)\
                        do{\
                            mode_t __old = umask(__mode);\
                            printf("[UMASK] Changed umask: %04o → %04o\n", __old, __mode);\
                        }while(0)
/**
 * @macro    __ACCESS
 * @brief    检查文件是否存在及可访问权限（Exist、Read、Write、Execute），并在一行内输出所有拥有的权限。
 */
#define __ACCESS(__pathname)\
                            do{\
                                if ((__pathname) == NULL)                                     \
                                    break;                                                    \
                                                                                              \
                                int _modes[] = {F_OK, R_OK, W_OK, X_OK};                      \
                                const char *_types[] = {"Exist", "Read", "Write", "Execute"}; \
                                                                                              \
                                printf("[ACCESS] '%s' permission check: ", (__pathname));     \
                                for (int i = 0; i < 4; i++) {                                 \
                                    if (access((__pathname), _modes[i]) != -1)                \
                                        printf("%s ", _types[i]);                             \
                                }                                                             \
                                printf("\n");                                                 \
                            }while (0)

/**
 * @macro    __ACCESS_MODE
 * @brief    执行 access 检查，返回结果（0=可访问，-1=不可访问），失败时自动打印错误。
 */
#define __ACCESS_MODE(__pathname, __mode)\
                                         ((access((__pathname), (__mode)) != -1) ? 0 : (PRINT_ERROR(), -1))

/**
 * @macro    __CHMOD
 * @brief    修改文件权限，并在控制台打印结果，包含异常检查。
 *
 * 用于在运行时修改目标文件的权限。该宏内部会检查路径有效性、文件存在性及写权限。
 *
 * 检查顺序：
 * 1. 路径不能为空；
 * 2. 文件必须存在（F_OK）；
 * 3. 当前进程对其具有写权限（W_OK）；
 * 4. chmod() 调用失败时会输出错误信息。
 *
 * @param __pathname 目标文件路径
 * @param __mode     新权限（mode_t 类型，八进制如 0644）
 */
#define __CHMOD(__pathname, __mode)                                               \
    do {                                                                          \
        if ((__pathname) == NULL)                                                 \
            break;                                                                \
        if (__ACCESS_MODE((__pathname), F_OK) == -1)                              \
            break;                                                                \
        if (__ACCESS_MODE((__pathname), W_OK) == -1)                              \
            break;                                                                \
        if (chmod((__pathname), (__mode)) == -1) {                                \
            PRINT_ERROR();                                                        \
            break;                                                                \
        }                                                                         \
        printf("[CHMOD] '%s' permission changed to: %04o\n",                      \
               (__pathname), (__mode));                                           \
    } while (0)

/**
 * @macro    PRINT_ERROR
 * @brief    打印当前出错位置的详细信息（文件名、行号、错误码和错误描述）。
 */
#define PRINT_ERROR() \
    printf("Error at %s:%d, errno = %d: %s\n", __FILE__, __LINE__, errno, strerror(errno))

/**
 * @macro   FILE_TYPE_STR
 * @brief   将文件类型宏（mode 的 S_IFMT 部分）转换为可读字符串。
 */
#define FILE_TYPE_STR(__mode)\
                                (\
                                    ((__mode) & S_IFMT) == S_IFREG  ? "Regular File"    :         \
                                    ((__mode) & S_IFMT) == S_IFDIR  ? "Directory"       :         \
                                    ((__mode) & S_IFMT) == S_IFLNK  ? "Symbolic Link"   :         \
                                    ((__mode) & S_IFMT) == S_IFIFO  ? "FIFO"            :         \
                                    ((__mode) & S_IFMT) == S_IFCHR  ? "Character Device":         \
                                    ((__mode) & S_IFMT) == S_IFBLK  ? "Block Device"    :         \
                                    ((__mode) & S_IFMT) == S_IFSOCK ? "Socket"          :         \
                                                                      "Unknown"                   \
                                )

/**
 * @macro    PRINT_FILE_INFO
 * @brief    打印文件相关信息，格式化输出文件属性和操作详情。
 */
#define PRINT_FILE_INFO(action, pf)\
                            printf(\
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
                                FILE_TYPE_STR((pf)->fst->type),                               \
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
int _file_get_properties(char *__pathname ,struct __file_stat *fst);
int _file_chown(_file_t *__pathname , uid_t owner, gid_t group);
char* _file_normalize_path(const char *__pathname);
int _file_set_time(const char *__pathname ,const struct timespec __times[2] ,int __flag);
int _file_link(const char *__from, const char *__to);
int _file_symlink(const char *__from, const char *__to);
int _file_readlink(const char *__restrict __path ,char *__restrict __buf ,size_t __len);
void _file_close(_file_t *pf);
#define FILE_CLOSE(pf)\
                        do{\
                            _file_close(pf);\
                            pf = NULL;\
                        }while(0)
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
/**
 * @macro   CHECK_FOPEN_MODE
 * @brief   文件打开标志检查
 */
#define CHECK_FOPEN_MODE(md) (\
    (md != NULL) && \
        (\
            strcmp(md ,"r") == 0 || strcmp(md ,"r+") == 0 \
                || strcmp(md ,"w") == 0 || strcmp(md ,"w+") == 0 \
                    || strcmp(md ,"a") == 0 || strcmp(md ,"a+") == 0 \
        )\
    )
/**
 * @macro   FREE_SFILE
 * @brief   安全释放 _sfile_t 结构体及其内部成员
 */
#define FREE_SFILE(psf)\
    do {                                   \
        if ((psf) != NULL) {               \
            if ((psf)->name != NULL) {     \
                free((psf)->name);         \
                (psf)->name = NULL;        \
            }                              \
            if ((psf)->path != NULL) {     \
                free((psf)->path);         \
                (psf)->path = NULL;        \
            }                              \
            if ((psf)->ptr != NULL) {      \
                free((psf)->ptr);          \
                (psf)->ptr = NULL;         \
            }                              \
            free((psf));                   \
            (psf) = NULL;                  \
        }                                  \
    } while (0)
         
/**
 * @macro   PRINT_SFILE_INFO
 * @brief   文件信息打印
 */
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
/******************************************************************************************************************/
/**
 * @macro  CHECK_MKDIR_MODE
 * @brief  检查目录权限模式的有效性
 */
#define CHECK_MKDIR_MODE(md)    ((md) & 0777)

/**
 * @macro   FREE_DFILE
 * @brief   安全释放 _dfile_t 结构体及其内部成员
 */
#define FREE_DFILE(pdf)\
    do {                                        \
        if ((pdf) != NULL) {                    \
            if ((pdf)->__pathname != NULL) {    \
                free((pdf)->__pathname);        \
                (pdf)->__pathname = NULL;       \
            }                                   \
            if ((pdf)->__fst != NULL) {         \
                free((pdf)->__fst);             \
                (pdf)->__fst = NULL;            \
            }                                   \
            if ((pdf)->__cwd != NULL) {         \
                free((pdf)->__cwd);             \
                (pdf)->__cwd = NULL;            \
            }                                   \
            if((pdf)->__dirs != NULL) {         \
                _dfile_dirsfree(pdf);           \
            }                                   \
            free((pdf));                        \
            (pdf) = NULL;                       \
        }                                       \
    } while (0)

/**
 * @brief 打印目录详细信息
 *
 * 该宏用于打印给定目录文件结构体 `pdf` 中单个文件的完整信息，
 * 并显示当前操作动作 `action`。打印内容包括文件名、操作动作、
 * 文件大小、inode号、文件类型、权限、所有者UID及用户名、所属组GID，
 * 以及访问时间(Atime)、修改时间(Mtime)、状态改变时间(Ctime)。
 *
 */
#define PRINT_DIR_INFO(action, pdf)\
        printf(                                                                            \
            "[File Info]\n"                                                                \
            "├─ Pathname               : %s\n"                                             \
            "├─ Current Work Directory : %s\n"                                             \
            "├─ Action                 : %s\n"                                             \
            "├─ Size                   : %ld bytes\n"                                      \
            "├─ Inode                  : %ld\n"                                            \
            "├─ Type                   : %s\n"                                             \
            "├─ RWX                    : 0%o\n"                                            \
            "├─ UID                    : %d (%s)\n"                                        \
            "├─ GID                    : %d\n"                                             \
            "├─ Atime                  : %s\n"                                             \
            "├─ Mtime                  : %s\n"                                             \
            "└─ Ctime                  : %s\n\n",                                          \
            (pdf)->__pathname,                                                             \
            (pdf)->__cwd,                                                                  \
            action,                                                                        \
            (pdf)->__fst->st.st_size,                                                      \
            (pdf)->__fst->st.st_ino,                                                       \
            FILE_TYPE_STR((pdf)->__fst->type),                                             \
            (pdf)->__fst->rwx,                                                             \
            (pdf)->__fst->st.st_uid,                                                       \
            (pdf)->__fst->pw ? (pdf)->__fst->pw->pw_name : "unknown",                      \
            (pdf)->__fst->st.st_gid,                                                       \
            (pdf)->__fst->atim,                                                            \
            (pdf)->__fst->mtim,                                                            \
            (pdf)->__fst->ctim                                                             \
        );

/**
 * @brief 打印目录中所有文件的详细信息
 *
 * 该宏用于打印指定目录结构体 `pdf` 中所有目录项的详细信息，
 * 包括目录项数量、索引号、文件名以及 inode 编号，格式化输出对齐。
 *
 * @param[in] pdf 指向包含目录信息的结构体指针，必须包含有效的 __counts 和 __dirs 成员。
 *
 * 输出格式示例：
 * Directory contains 5 entries:
 * Index  Name                           Inode
 * ---------------------------------------------------
 * 1      filename1                      123456
 * 2      filename2                      234567
 * ...
 *
 * @note 该宏内部使用循环遍历所有目录项，并逐行打印对应信息。
 */
 #define PRINT_DIR_ALLFILE_INFO(pdf)\
                                do{\
                                    printf("Directory contains %d entries:\n", (pdf)->__counts);      \
                                    printf("%-6s %-30s %10s\n", "Index", "Name", "Inode");            \
                                    printf("---------------------------------------------------\n");  \
                                    for(int i = 0; i < (pdf)->__counts; i++){                         \
                                        printf("%-6d %-30s %10ld\n",                                  \
                                            i + 1, (pdf)->__dirs[i]->d_name, (pdf)->__dirs[i]->d_ino);\
                                    }                                                                 \
                                }while(0)
                        

/**
 * @struct _dfile_t
 * @brief 用于描述单个文件或目录及其相关元数据信息的结构体。
 *
 * 该结构体封装了路径、文件状态、目录流句柄和已读取目录项等信息，
 * 便于对文件或目录执行统一管理和操作，如读取属性、遍历子项等。
 */
typedef struct{
    char *__pathname;                 /**< 文件或目录的绝对路径字符串，需动态分配并释放。 */
    char *__cwd;
    struct __file_stat *__fst;       /**< 指向文件属性信息结构体的指针（如通过 stat 填充）。 */
    DIR *__dirp;                     /**< 目录流指针，用于目录遍历，来自 opendir。文件时为 NULL。 */
    struct dirent **__dirs;          /**< 指向已读取的目录项数组，每个元素为动态分配的 dirent 结构体指针。 */
    int __counts;                    /**< 当前已读取并存储在 __dirs 中的目录项数量。 */
}_dfile_t;

int _dfile_getcwd(char **__cwd ,const size_t __sz);
int _dfile_chdir(const char *__work_directory);
int _dfile_refresh_info(_dfile_t *__pdf ,char *__str);
_dfile_t* _dfile_init(const char *__pathname);
int _dfile_open(_dfile_t *__pdf);
int _dfile_close(DIR *__dir);
void _dfile_dirsfree(_dfile_t *__pdf);
int _dfile_allread(_dfile_t *__pdf);
int _dfile_mkdir(const char *__pathname ,const mode_t __md);
int _dfile_empty(const char *__pathname);
int _dfile_rmdir(const char *__pathname);
#ifdef __cplusplus
}
#endif

#endif /* __FILE_H */
