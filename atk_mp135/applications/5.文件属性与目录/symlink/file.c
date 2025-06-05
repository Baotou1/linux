/**
 * @file file.c
 * @brief 封装的文件操作模块，用于简化文件读写、偏移管理及内存管理等功能。
 *
 * 本模块实现了一个结构化的文件操作接口，基于 `_file_t` 结构体抽象了文件打开、关闭、
 * 读、写、打印及偏移设置等常见操作，增强了代码的可读性和可维护性。
 *
 * 模块功能包括：
 * - 初始化 `_file_t` 文件对象并分配缓冲区内存
 * - 打开/创建文件并记录文件描述符
 * - 设置或获取文件偏移量（通过 lseek 实现）
 * - 从文件中读取数据至缓冲区，支持回绕读取剩余部分
 * - 将数据写入文件，支持从指定偏移开始写入
 * - 打印文件内容（按字符逐个输出）
 * - 自动处理文件关闭与资源释放
 *
 * 所有操作均带有详细的日志输出和错误提示，有助于调试和文件状态跟踪。
 *
 * @note 
 * - 文件读取/写入使用的是 `_file_t` 的内部缓冲区和文件描述符。
 * - 错误处理较为完善，失败后一般会关闭文件并释放内存。
 * - 若 `_file_close()` 被调用，原指针不可再访问。
 *
 * @author baotou
 * @date 2025-05-16
 *
 * @version 3.3.0 - 25/6/5
 *
 * @par 更新记录（Change Log）
 * - 2025-05-16：初始版本，实现基础的文件封装读写功能。 baotou
 * - 2025-05-20：重构_file_read/write/print/open/init函数，便于维护和移植。 baotou
 * - 2025-05-20：修改_file_print函数，增加_file_print_u16函数。 baotou
 * - 2025-05-21：增加_file_cpfd/_pread/_write/_status_fcntl函数。 baotou
 * - 2025-05-22: 增加_file_truncate函数。 baotou
 * - 2025-05-26: 修改_file_data_init和_file_get_size函数，增加_sfile_init/open/close/write等函数。 baotou
 * - 2025-06-01: 修改_file_get_size函数为_file_get_properties。 baotou
 * - 2025-06-02: 增加_file_get_type/get_rwx/get_time。 baotou
 * - 2025-06-02: 增加_file_chown。 baotou
 * - 2025-06-05: 增加_file_normalize_path和——file_set_time。 baotou
 */

#include "file.h"
int _file_status_fcntl(_file_t *pf ,int cmd, ...);
/**
 * @name _file_get_offset
 * @brief 获取文件当前偏移量。
 *
 * 此函数使用 lseek 函数（配合 SEEK_CUR）获取文件描述符当前的偏移位置，
 * 即下次读取或写入时的位置，并将该值更新到 _file_t 结构体中的 ofs 字段中。
 *
 * @param[in,out] pf 指向 _file_t 结构体的指针，结构体中需包含有效的文件描述符 fd。
 *
 * @return 成功返回 0，失败返回 -1，并使用 perror 打印警告信息。
 */
static int _file_get_offset(_file_t *pf)
{
    pf->ofs = lseek(pf->fd ,0 ,SEEK_CUR);
    if(pf->ofs == -1){
        perror("Warning: lseek get now current failed.");
        return -1;
    }
    return 0;
}
  
/**
 * @name _file_set_offset
 * @brief 设置文件偏移量。
 *
 * 此函数使用 lseek 设置文件描述符的读写位置（偏移量），偏移方式由 whence 参数指定。
 * 设置成功后，会将新的偏移量写入 _file_t 结构体中的 ofs 字段中。
 *
 * @param[in,out] pf 指向 _file_t 结构体的指针，结构体中需包含有效的文件描述符 fd。
 * @param offset 偏移量值，结合 whence 决定新的读写位置。
 * @param whence 偏移方式（如 SEEK_SET 表示从文件开头偏移，SEEK_CUR 表示从当前位置偏移等）。
 *
 * @return 成功返回 0，失败返回 -1，并使用 perror 打印警告信息。
 */
static int _file_set_offset(_file_t *pf ,off_t offset ,int whence)
{
    pf->ofs = lseek(pf->fd ,offset ,whence);
    if(pf->ofs == -1){
        perror("Warning: read lseek failed");
        return -1;
    }      
    return 0;
}
 
/**
 * @name  _file_get_type
 * @brief 根据文件的 st_mode 字段解析其类型。
 *
 * 该函数通过检查 `struct stat` 中的 `st_mode` 字段，判断文件类型，
 * 并将对应的类型宏（如 `__S_IFREG`、`__S_IFDIR` 等）写入传出参数 `type`。
 *
 * @param[in]  st    指向有效的 struct stat 结构体，用于获取文件信息。
 * @param[out] type  指向 int 类型的输出变量，用于接收文件类型标志。
 *
 * @note 若 st 为空，则函数直接返回。未识别的类型将被设置为 0。
 */
static void _file_get_type(struct stat *st ,int *type)
{
    if(st == NULL)
        return;

    /* 管道文件 */
    if(S_ISFIFO(st->st_mode) == true){
        *type = __S_IFIFO;
    }
    /* 字符设备设备文件 */
    else if(S_ISCHR(st->st_mode) == true){
        *type = __S_IFCHR;
    }
    /* 目录文件 */
    else if(S_ISDIR(st->st_mode) == true){
        *type = __S_IFDIR;
    }
    /* 块设备文件 */
    else if(S_ISBLK(st->st_mode) == true){
        *type = __S_IFBLK;
    }
    /* 普通文件 */
    else if(S_ISREG(st->st_mode) == true){
        *type = __S_IFREG;
    }
    /* 链接文件 */
    else if(S_ISLNK(st->st_mode) == true){
        *type = __S_IFLNK;
    }
    /* 套接字文件 */
    else if(S_ISSOCK(st->st_mode) == true){
        *type = __S_IFSOCK;
    }
    else{
        *type = 0;
    }
} 
 
/**
 * @name  _file_get_rwx
 * @brief 提取文件权限（读/写/执行）位。
 *
 * 该函数从 `struct stat` 中提取最低 9 位权限标志（即 rwxrwxrwx），
 * 并将结果写入输出参数 `rwx`。
 *
 * @param[in]  st   指向有效的 struct stat 结构体。
 * @param[out] rwx  指向 int 类型的输出变量，用于接收权限值（八进制表示）。
 *
 * @note 若 st 为 NULL，函数将直接返回。
 */
static void _file_get_rwx(struct stat *st ,int *rwx)
{
    if(st == NULL)
        return;
    
    *rwx = st->st_mode & 0777;
}

/**
 * @name  _file_get_time
 * @brief 将 time_t 时间格式转换为格式化的字符串时间表示。
 *
 * 该函数使用 `localtime_r` 将给定的 `time_t` 时间转换为本地时间的 `struct tm`，
 * 并通过 `strftime` 将时间格式化为“YYYY-MM-DD HH:MM:SS”格式字符串，
 * 写入到调用者提供的缓冲区 `__buf` 中。
 *
 * @param[in]  __timer  指向有效的 time_t 类型时间变量。
 * @param[out] __buf    用于存放格式化时间字符串的字符缓冲区，建议大小至少为 100 字节。
 *
 * @note 函数不做缓冲区长度检查，调用者需保证 `__buf` 有足够空间。
 *       如果 `__timer` 为空指针，函数行为未定义，应避免传入 NULL。
 */
static void _file_get_time(const time_t *__timer ,char *__buf)
{
    struct tm _tm;

    localtime_r(__timer ,&_tm);
    strftime(__buf, 100,\
                        "%Y-%m-%d %H:%M:%S", &_tm);
}

/**
 * @name  _file_get_pw
 * @brief 根据 UID 获取用户的 passwd 结构体信息。
 *
 * 此函数通过调用 `getpwuid` 系统接口，根据指定的用户 ID（uid）获取对应的用户信息，
 * 并将结果赋值给传入的 passwd 结构体指针指针。
 *
 * @param[out] ppw  指向 `struct passwd*` 的指针（即二级指针），用于返回获取到的用户信息。
 * @param[in]  uid  要查询的用户 ID（User ID）。
 *
 * @return 成功返回 0，失败返回 -1。
 *
 * @note
 * - `getpwuid` 返回的是一个指向静态内存的指针，因此不应对 `*ppw` 结果进行释放。
 * - 函数不分配新内存，只是将 `getpwuid` 的结果赋给 `*ppw`。
 * - 调用方需保证 `ppw` 本身是有效的地址。
 */

static int _file_get_pw(struct passwd **ppw ,uid_t uid)
{
    if(ppw == NULL)
        return -1;

    *ppw = getpwuid(uid);

    if(*ppw == NULL)
        return -1;
    return 0;
}

/**
 * @name  _file_get_properties
 * @brief 获取并更新指定文件的属性信息，包括类型、权限及时间戳。
 *
 * 该函数使用 `stat()` 系统调用获取指定路径文件的元数据，并更新传入的
 * `__file_stat` 结构体中的 `st`（文件状态信息）、`type`（文件类型）、
 * `rwx`（权限信息）以及访问时间（atim）、修改时间（mtim）和更改时间（ctim）。
 *
 * @param[in]  __pathname  文件路径字符串，指向需要获取属性的文件。
 * @param[out] fst         指向 `__file_stat` 结构体的有效指针，用于接收文件属性信息。
 *
 * @retval  0        成功获取文件属性。
 * @retval -1        获取失败（如参数无效或 stat 出错），并输出错误信息。
 *
 * @note  成功调用后，`fst->st`, `fst->type`, `fst->rwx`, `fst->atim`, `fst->mtim`, `fst->ctim` 均被更新。
 */
int _file_get_properties(char *__pathname ,struct __file_stat *fst)
{
    if(__pathname == NULL || fst == NULL)
        return -1;
#if 0
    if(fstat(pf->fd ,&pf->st) == -1){
        perror("get file size error.");
        return -1;
    }
#else
    if(stat(__pathname ,&fst->st) == -1){
        perror("get file size error.");
        return -1;
    }
#endif
    _file_get_type(&fst->st ,&fst->type);
    _file_get_rwx(&fst->st ,&fst->rwx);
    _file_get_time(&fst->st.st_atim.tv_sec ,fst->atim);
    _file_get_time(&fst->st.st_mtim.tv_sec ,fst->mtim);
    _file_get_time(&fst->st.st_ctim.tv_sec ,fst->ctim);
    _file_get_pw(&fst->pw ,fst->st.st_uid);
    return 0;
}

/**
 * @name  _file_data_init
 * @brief 初始化或重置数据缓冲区。
 *
 * 该函数用于初始化或重新初始化指针所指向的数据缓冲区。若指针当前非空，
 * 会先释放已有内存，防止内存泄漏。随后分配指定大小的缓冲区，并将其内容清零。
 *
 * @param[in,out] pptr 指向需要初始化的指针的地址（即二级指针），
 *                     函数将修改该指针指向新分配的缓冲区。
 * @param[in]     size 需要分配的缓冲区大小（单位：字节）。
 *
 * @return 成功返回 0，失败返回 -1。
 *
 * @note  - 如果输入指针为空或 size 为 0，函数直接返回失败。
 *        - 分配失败时，会打印错误信息，但不会终止程序。
 *        - 调用前应确保 pptr 指向有效地址。
 */
static int _file_data_init(void **pptr ,size_t size)
{
    if(pptr == NULL || size == 0)
        return -1;

    if(*pptr != NULL){
        free(*pptr);
        *pptr = NULL;
    }

    *pptr = calloc(size ,sizeof(char));
    if(*pptr == NULL){
        printf("calloc error...\n");
        return -1;
    }

    return 0;
}

/**
 * @name  __file_chown
 * @brief 修改指定路径文件的属主和属组。
 *
 * 该函数封装了对指定路径文件调用 chown() 系统调用，
 * 用于修改文件的属主（owner）和属组（group）。
 *
 * @param[in] _pt   需要修改属主属组的文件路径字符串指针。
 * @param[in] _own  新的属主用户ID。
 * @param[in] _grp  新的属组组ID。
 *
 * @return 成功返回 0，失败返回 -1 并打印错误信息。
 *
 * @note  - 如果路径指针为空，函数直接返回失败。
 *        - 调用失败时，会打印错误信息。
 */
static int __file_chown(const char *__pathname, uid_t _own, gid_t _grp)
{
    if(__pathname == NULL)
        return -1;

    if(chown(__pathname ,_own ,_grp) == -1){
        PRINT_ERROR();
        return -1;
    }
    
    return 0;
}

/**
 * @name  _file_chown
 * @brief 修改 _file_t 结构体对应文件的属主和属组，并更新文件属性信息。
 *
 * 该函数调用内部的 __file_chown 来修改文件的属主和属组，随后调用
 * _file_get_properties 更新文件属性信息，最后打印文件信息。
 *
 * @param[in,out] pf    指向已打开文件的 _file_t 结构体指针，必须包含有效文件名和文件状态结构。
 * @param[in]     owner 新的属主用户ID。
 * @param[in]     group 新的属组组ID。
 *
 * @return 成功返回 FILE_EOK，失败返回负错误码 -FILE_ERROR。
 *
 * @note  - 调用失败时，函数会返回相应错误码。
 *        - 确保 pf 结构体内部字段已正确初始化。
 */
int _file_chown(_file_t *pf , uid_t owner, gid_t group)
{
    if(__file_chown(pf->__pathname ,owner ,group) == -1)
        return -FILE_ERROR;

    if(_file_get_properties(pf->__pathname ,pf->fst) == -1)
        return -FILE_ERROR;

    PRINT_FILE_INFO("chown" ,pf);
    return FILE_EOK;
}

/**
 * @name  _file_normalize_path
 * @brief 获取给定路径的规范化（绝对）路径。
 *
 * 该函数使用 realpath() 将传入的路径转换为绝对路径，并解析其中的符号链接、"." 和 ".." 等。
 * 注意：realpath() 会在堆上分配内存，返回的路径字符串需要由调用者手动 free() 释放，以防内存泄漏。
 *
 * @param __pathname 要规范化的路径（可以是相对路径或绝对路径）
 * @return char* 返回规范化后的绝对路径字符串，失败返回 NULL
 */
char* _file_normalize_path(const char *__pathname)
{
    /* realpath会在堆上分配一个内存 */
    return realpath(__pathname ,NULL);
}

/**
 * @name  _file_set_time
 * @brief 设置文件的访问时间和修改时间。
 * 
 * @param __pathname 文件路径（可以是相对路径）
 * @param __times    两个时间点的数组（访问时间和修改时间），可为 NULL 表示使用当前时间
 * @param __flag     标志，比如 0 或 AT_SYMLINK_NOFOLLOW
 *
 * @return 成功返回 0，失败返回 -1
 */
int _file_set_time(const char *__pathname ,const struct timespec __times[2] ,int __flag)
{
    if(__pathname == NULL)
        return -1;

    /* 转化为绝对路径 */
    char *__res = _file_normalize_path(__pathname);
    if(__res == NULL){
        PRINT_ERROR();
        return -1;
    }

    /* 当pathname为绝对路径，忽略第一个参数 */
    if(utimensat(-1 ,__res ,__times ,__flag) == -1)
    {
        PRINT_ERROR();
        free(__res);
        return -1;
    }
    free(__res);
    return 0;      
}

/**
 * @name  _file_link
 * @brief 创建一个新的硬链接，指向已有的文件。
 * 
 * @param __from 源文件路径（已有文件）
 * @param __to   新链接的路径（硬链接名称）
 * 
 * @return 成功返回 0，失败返回 -1
 */
int _file_link(const char *__from, const char *__to)
{
    if(link(__from ,__to) == -1){
        PRINT_ERROR();
        return -1;
    }

    return 0;
}

/**
 * @name  _file_symlink
 * @brief 创建一个新的软链接，指向已有的文件。
 * 
 * @param __from 源文件路径（已有文件）
 * @param __to   新链接的路径（软链接名称）
 * 
 * @return 成功返回 0，失败返回 -1
 */
int _file_symlink(const char *__from, const char *__to)
{
    if(symlink(__from ,__to) == -1){
        PRINT_ERROR();
        return -1;
    }

    return 0;
}
/******************************************************************************************************************/ 
/**
 * @name  _file_get_info
 * @brief 获取并更新文件结构体中的文件大小与当前偏移信息。
 *
 * 此函数内部依次调用 `_file_get_properties()` 和 `_file_get_offset()`，
 * 用于更新 `_file_t` 结构体中的 `st` 和 `ofs` 字段。
 *
 * @param[in,out] pf   文件结构体指针，不能为空，且内部包含有效的文件描述符。
 *
 * @retval 0           成功更新文件大小和偏移信息。
 * @retval -1          任意一个调用失败（如文件未打开、fd 无效等）。
 *
 * @note 调用本函数可确保 `pf->size` 与 `pf->ofs` 字段为最新状态。
 */
static int _file_get_info(_file_t *pf)
{
    if(_file_get_properties(pf->__pathname ,pf->fst) == -1)
        return -1;

    if(_file_get_offset(pf) == -1)
        return -1;

    if(_file_status_fcntl(pf ,F_GETFL) == -1)
        return -1;

    return 0;
}
 
/**
 * @name _file_close
 * @brief 关闭文件并释放关联的内存资源。
 *
 * 此函数用于关闭文件描述符，并释放 _file_t 结构体中分配的内存资源，包括文件名和数据缓冲区。
 * 需要注意的是，pf->pw 指针通常指向由 getpwuid() 返回的静态内存，不能调用 free。
 * 即使传入的指针为 NULL，也不会导致程序崩溃。
 *
 * @param[in] pf 指向 _file_t 结构体的指针，可以为 NULL。
 *
 * @return 无。
 */
void _file_close(_file_t *pf)
{
    if(pf == NULL)
        return;

    printf("%s file close.\n" ,pf->__pathname);
    if(pf->fd >= 0)
        close(pf->fd);
    
    if(pf->__pathname !=NULL)
        free(pf->__pathname);
    
    if(pf->data != NULL)
        free(pf->data);

    if(pf->fst != NULL)
        free(pf->fst);

    free(pf);    
}
  
/**
 * @name   _file_init
 * @brief  初始化一个 _file_t 文件对象。
 *
 * 该函数会为 `_file_t` 结构体及其内部使用的 `__file_stat` 结构体分配内存，
 * 并复制传入的文件名字符串，确保其独立性和安全性。
 * 
 * 注意：pf->pw 指针初始化为 NULL，使用时应调用 getpwuid() 获取，不需预分配内存。
 *
 * @param[in] __pathname 文件路径名，不能为 NULL。
 *
 * @return 成功返回指向 `_file_t` 结构体的指针，失败返回 NULL。
 *
 * @note
 * - 使用完毕后需调用对应的释放函数释放资源，防止内存泄漏。
 * - 返回的结构体中 `fd = -1` 表示尚未打开文件。
 */
_file_t* _file_init(char *__pathname)
{
    if(__pathname == NULL)
        return NULL;
        
    _file_t *pf = (_file_t *)calloc(1 ,sizeof(_file_t));
    if(pf == NULL)
        return NULL;

    pf->fst = (struct __file_stat*)calloc(1 ,sizeof(struct __file_stat));
    if(pf->fst == NULL){
        free(pf);
        return NULL;
    }
    // 不再给 pf->pw 分配内存，改为初始化为 NULL
    pf->fst->pw = NULL;

    pf->fd = -1;
    pf->data = NULL;
    pf->fg = 0;
    pf->ofs = 0;
    pf->ret = 0;
    pf->fst->type = 0;
    pf->fst->rwx = 0;
    //pf->__pathname 分配一块新的内存，并复制 name 字符串的内容进去，避免直接使用外部传入的指针，保证文件名的独立性和安全性
    pf->__pathname = strdup(__pathname);
    if(pf->__pathname == NULL){
        free(pf->fst);
        free(pf);
        return NULL;
    }

    return pf;
}

 /**
  * @name _file_open
  * @brief 打开或创建文件，并初始化 _file_t 结构体中的相关字段。
  *
  * 根据传入的文件打开标志 `fg` 和权限模式 `md` 打开或创建文件，更新 `_file_t` 结构体中的文件描述符、文件大小、偏移量等信息。
  * 同时为文件数据分配缓冲区。
  *
  * 说明：
  * - 当 `fg` 包含 O_CREAT 时，`md` 用作新文件权限；否则 `md` 参数无效。
  * - 使用 open() 打开文件，出错时打印错误信息并返回错误码。
  * - 使用 fstat 获取文件大小。
  * - 初始化文件当前偏移量。
  * - 为数据缓冲区分配 1 字节内存（可根据需求调整）。
  *
  * @param[in,out] pf 指向 _file_t 结构体的指针，不能为空。
  * @param[in] fg 文件打开标志（如 O_RDONLY、O_WRONLY | O_CREAT 等）。
  * @param[in] md 文件权限模式（如 0666），仅在创建新文件时有效。
  *
  * @return 成功返回 FILE_EOK（通常为 0），失败返回负错误码。
  */
 int _file_open(_file_t *pf ,int fg ,mode_t md)
 {
     if(pf == NULL || fg < 0)
         return -FILE_ERROR;
     
     pf->fg = fg;
     
     pf->fd = open(pf->__pathname ,pf->fg ,md);
     if(pf->fd == -1){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     if(_file_get_properties(pf->__pathname ,pf->fst) == -1)
         return -1;
 
     if(_file_get_offset(pf) == -1)
         return -FILE_ERROR;
 
     if(_file_data_init(&pf->data ,1) == -1)
         return -FILE_ERROR;
 
     PRINT_FILE_INFO("open" ,pf);
     return FILE_EOK;
 }
 
 /**
  * @name _file_read
  * @brief 从指定偏移位置开始，读取数据到 _file_t 的 data 缓冲区中。
  *
  * 本函数会根据传入的偏移量和 whence 参数，先调用 lseek 设置读取起始位置，
  * 然后计算实际可读长度（防止越界），初始化缓冲区大小，最后读取数据。
  *
  * @param pfr     指向 _file_t 文件结构体的指针，必须已打开文件并分配 fd。
  * @param ofs     偏移量，表示从文件哪个位置开始读。
  * @param whence  lseek 的方式：SEEK_SET / SEEK_CUR / SEEK_END。
  * @param len     想要读取的最大字节数。
  *
  * @return 实际读取的字节数（>=0），失败返回 -FILE_ERROR。
  *
  * @note 若 len 超过剩余文件大小，会自动缩减至剩余部分。
  */
 
 int _file_read(_file_t *pfr ,off_t ofs ,int whence ,size_t len)
 {
     if(pfr == NULL || pfr->data == NULL || len <= 0 ||
             (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END))
         return -1;
 
     if(_file_set_offset(pfr ,ofs ,whence) == -1)
         return -FILE_ERROR;
     printf("set %s file read offset: %ld bytes\n" ,pfr->__pathname ,pfr->ofs);
     
     if(_file_get_properties(pfr->__pathname ,pfr->fst) == -1)
         return -1;
     
     len = (len > (pfr->fst->st.st_size - pfr->ofs))?  (pfr->fst->st.st_size - pfr->ofs): len;
 
     if(_file_data_init(&pfr->data ,len) == -1)
         return -FILE_ERROR;
 
     pfr->ret = read(pfr->fd ,pfr->data ,len);    
     if(pfr->ret < 0){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
     
     if(_file_get_info(pfr) == -1)
         return -FILE_ERROR;
 
     PRINT_FILE_INFO("read" ,pfr);
     return pfr->ret;
 }
  
 /**
  * @name  _file_write
  * @brief 向指定文件写入一段数据。
  *
  * 本函数会首先根据传入的 offset 和 whence 参数设置文件偏移，
  * 然后从指定的数据缓冲区写入 len 字节到文件中。
  * 写入完成后会更新文件的大小（fs）和当前偏移（ofs）信息。
  *
  * - 若设置偏移失败、写入失败或更新元信息失败，则返回 -1。
  * - 写入失败不会自动关闭文件（建议交由调用者处理资源释放）。
  *
  * @param[in,out] pfw     指向目标文件结构体 _file_t 的指针。
  * @param[in]     data    指向要写入的数据缓冲区。
  * @param[in]     ofs     写入时设置的偏移值。
  * @param[in]     whence  偏移设置方式（SEEK_SET、SEEK_CUR、SEEK_END）。
  * @param[in]     len     要写入的数据字节数，必须 > 0。
  *
  * @return 成功返回实际写入的字节数，失败返回 -FILE_ERROR。
  */
 int _file_write(_file_t *pfw ,void *data ,off_t ofs ,int whence ,size_t len)
 {
     if(pfw == NULL || data == NULL || len <= 0 ||
         (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END))
         return -FILE_ERROR;
 
     if(_file_set_offset(pfw ,ofs ,whence) == -1)
         return -FILE_ERROR;
     printf("set %s file write offset: %ld bytes\n" ,pfw->__pathname ,pfw->ofs);
 
     pfw->ret = write(pfw->fd ,data ,len);
     if(pfw->ret == -1){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
     
     if(_file_get_info(pfw) == -1)
         return -FILE_ERROR;
 
     PRINT_FILE_INFO("write" ,pfw);
     return pfw->ret;
 }
 
 /**
  * @name  _file_pread
  * @brief 读取文件内容（基于偏移，非移动式读取）。
  *
  * 从指定文件pfr偏移ofs开始读取len字节数据到pfr->data中，。
  * 使用pread()实现，读取不会影响文件当前偏移。若len超过文件结尾，则自动裁剪读取长度为：文件大小 - ofs`。
  *
  * - 若输入参数非法、写入失败或更新元信息失败，则返回 -FILE_ERROR。
  * - 写入失败不会自动关闭文件，资源释放建议由调用者负责。
  *
  * @param[in,out] pfw    指向目标文件结构体 _file_t 的指针。
  * @param[in]     data   指向要写入的数据缓冲区。
  * @param[in]     len    要写入的数据字节数，必须 > 0。
  * @param[in]     ofs    写入时设置的文件偏移。
  *
  * @return 成功返回实际写入的字节数，失败返回 -FILE_ERROR。
  * 
  * @note: 与read()不同，pread()不会修改文件描述符对应的当前偏移量,
  *        因此本函数调用后，文件偏移量（file offset）保持不变。
  */
 int _file_pread(_file_t *pfr ,size_t len ,off_t ofs)
 {
     if(pfr == NULL || pfr->data == NULL || len <= 0)
         return -1;
 
 //#define __PRINT_FILEOFS
 #ifdef __PRINT_FILEOFS
     if(_file_get_offset(pfr) == -1)
         return -FILE_ERROR;
     printf("get %s file offset: %ld bytes\n" ,pfr->__pathname ,pfr->ofs);    
 #endif
 
     if(_file_get_properties(pfr->__pathname ,pfr->fst) == -1)
         return -1;
 
     len = (len > (pfr->fst->st.st_size - ofs))?  (pfr->fst->st.st_size - ofs): len;
 
     if(_file_data_init(&pfr->data ,len) == -1)
         return -FILE_ERROR;
 
     pfr->ret = pread(pfr->fd ,pfr->data ,len ,ofs);    
     if(pfr->ret < 0){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     if(_file_get_info(pfr) == -1)
         return -FILE_ERROR;
 
     PRINT_FILE_INFO("read" ,pfr);
     return pfr->ret;
 }
 
 /**
  * @name  _file_pwrite
  * @brief 向文件指定偏移写入数据（基于偏移，非移动式写入）。
  *
  * 使用 pwrite() 向文件 pfw 指定的偏移 ofs 处写入 len 字节数据，
  * 数据来源于指针 data。写入操作不会修改文件的当前偏移。
  * 写入完成后会更新文件大小（fs）和当前偏移（ofs）信息。
  *
  * - 若输入参数非法、写入失败或更新元信息失败，则返回 -FILE_ERROR。
  * - 写入失败不会自动关闭文件，资源释放建议由调用者负责。
  *
  * @param[in,out] pfw    指向目标文件结构体 _file_t 的指针。
  * @param[in]     data   指向要写入的数据缓冲区。
  * @param[in]     len    要写入的数据字节数，必须 > 0。
  * @param[in]     ofs    写入时设置的文件偏移。
  *
  * @return 成功返回实际写入的字节数，失败返回 -FILE_ERROR。
  * 
  * @note 与 write() 不同，pwrite() 不会修改文件描述符对应的当前偏移量，
  *       因此本函数调用后，文件偏移量（file offset）保持不变。
  */
 int _file_pwrite(_file_t *pfw ,void *data ,size_t len ,off_t ofs)
 {
     if(pfw == NULL || data == NULL || len <= 0)
         return -FILE_ERROR;
 
 #ifdef __PRINT_FILEOFS
     if(_file_get_offset(pfw) == -1)
         return -FILE_ERROR;
     printf("get %s file offset: %ld bytes\n" ,pfw->__pathname ,pfw->ofs);
 #endif
 
     pfw->ret = pwrite(pfw->fd ,data ,len ,ofs);
     if(pfw->ret == -1){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
     
     if(_file_get_info(pfw) == -1)
         return -FILE_ERROR;
 
     PRINT_FILE_INFO("write" ,pfw);
     return pfw->ret;
 }
 
 /**
  * @name  _file_cpfd
  * @brief 复制源文件结构体的文件描述符到目标结构体，支持 dup/dup2/fcntl 模式。
  *
  * 本函数用于将 `pf` 中的文件描述符复制到 `cppf` 中，封装了 dup、dup2 和 fcntl(F_DUPFD) 三种方式。
  * 支持用户指定是否由系统自动分配描述符，或手动指定目标描述符号。
  *
  * @param[in]  pf     原始文件 `_file_t` 结构体指针，需已打开。
  * @param[out] cppf   目标文件 `_file_t` 结构体指针，复制后的 fd 存入此结构体中。
  * @param[in]  flag   指定复制方式：
  *                    - CP_FILE_DUP_1：使用 dup()，系统分配新描述符；
  *                    - CP_FILE_DUP_2：使用 dup2()，目标描述符为 nfd；
  *                    - CP_FILE_FCNTL_3：使用 fcntl(F_DUPFD)，从 nfd 起分配可用描述符。
  * @param[in]  nfd    当 flag 为 CP_FILE_DUP_2 或 CP_FILE_FCNTL_3 时有效，表示目标 fd 或起始 fd。
  *
  * @retval FILE_EOK     成功（0）
  * @retval -FILE_ERROR  失败（如参数无效或系统调用出错）
  *
  * @note
  * - 所有方式创建的新描述符与原描述符共享同一个文件描述符表项（open file description），
  *   包括文件状态标志，但每个描述符具有独立的文件描述符号。
  * - dup2() 可以覆盖已有的 nfd，无需显式关闭旧描述符。
  * - fcntl(F_DUPFD) 会从 nfd 起查找可用文件描述符。
  */
 int _file_cpfd(_file_t *pf ,_file_t *cppf ,int flag ,int nfd)
 {
     if(pf == NULL || cppf == NULL || 
                 (flag != CP_FILE_DUP_1 && flag != CP_FILE_DUP_2 && flag != CP_FILE_FCNTL_3))
         return -FILE_ERROR;
     
     if(flag == CP_FILE_DUP_1){
         cppf->fd = dup(pf->fd);
     }
     else if(flag == CP_FILE_DUP_2){
         if(fcntl(nfd ,F_GETFD) == -1){
             cppf->fd = dup2(pf->fd ,nfd);
         }else{
             printf("the new file descriptor is already in use\n");
             return -FILE_ERROR;
         }
     }
     else if(flag == CP_FILE_FCNTL_3){
         cppf->fd = fcntl(pf->fd ,F_DUPFD ,nfd);
     }
 
     if(cppf->fd ==-1){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
     return FILE_EOK;
 }
 
 /**
  * @name  _file_status_fcntl
  * @brief 获取或设置文件状态标志（仅支持 F_GETFL / F_SETFL）。
  *
  * 本函数封装 fcntl 系统调用，仅支持获取和设置文件状态标志。
  * 当 cmd 为 F_SETFL 时，调用者需额外提供 int 类型的 flag 参数，
  * 不能包含 open() 专用标志，如 O_CREAT、O_EXCL 等。
  *
  * @param[in,out] pf    文件结构体指针，需已打开
  * @param[in]     cmd   F_GETFL 或 F_SETFL
  * @param[in]     ...   若 cmd 为 F_SETFL，需传入 int flag，表示待设置标志
  *
  * @retval  FILE_EOK     成功
  * @retval -FILE_ERROR   失败（错误信息已打印）
  *
  * @note 成功时会更新 pf->fg 成员。
  */
 int _file_status_fcntl(_file_t *pf ,int cmd, ...){
     if(pf == NULL || (cmd != F_GETFL && cmd != F_SETFL))
         return -FILE_ERROR;
 
     va_list args;
     int nfg = 0;
 
     if(cmd == F_GETFL){
         nfg = fcntl(pf->fd ,F_GETFL);
     }
     else if(cmd == F_SETFL){
         va_start(args, cmd);
         int flag = va_arg(args, int);
         va_end(args);
 
         if(HAS_INVALID_F_SETFL_FLAGS(flag) == 0){
 
             int ofg = fcntl(pf->fd ,F_GETFL);
             if(ofg == -1){
                 PRINT_ERROR();
                 return -FILE_ERROR; 
             }
 
             if(fcntl(pf->fd ,F_SETFL ,ofg | flag) == -1){
                 nfg = -1;
             }else{
                 nfg = ofg | flag;
             }
         }else{
             printf("invalid flag: cannot use open() flags (e.g., O_CREAT) with fcntl(F_SETFL).\n");
             return -FILE_ERROR;     
         }
     }
 
     if(nfg == -1){
         PRINT_ERROR();
         return -FILE_ERROR;  
     }
 
     pf->fg = nfg;
     return FILE_EOK;
 }
 
 /**
  * @brief     截断文件内容
  * 
  * @details   根据指定的截断方式（通过 cmd 控制），使用 ftruncate 或 truncate 
  *            将文件截断到指定长度 len，并设置文件偏移到 ofs。
  * 
  * @param[in] pf   文件控制结构体指针（必须已打开）
  * @param[in] len  目标截断长度（字节），不能为负
  * @param[in] ofs  截断后希望设置的文件偏移量
  * @param[in] cmd  截断方式：
  *                 - FILE_F_TRUNCATE ：对已打开文件描述符使用 ftruncate
  *                 - FILE_TRUNCATE   ：对指定路径使用 truncate（路径通过可变参数传入）
  * @param[in] ...  可变参数，仅当 cmd 为 FILE_TRUNCATE 时，需传入 const char* 类型路径
  * 
  * @retval    FILE_EOK    成功
  * @retval    FILE_ERROR  失败（参数错误或系统调用失败）
  * 
  * @note      成功后 pf->ret 记录截断前后的大小差异（单位：字节），并打印文件信息。
  */
 int _file_truncate(_file_t *pf ,off_t len ,off_t ofs ,int cmd ,...)
 {
     if(pf == NULL || len < 0 ||
         (cmd != FILE_F_TRUNCATE && cmd != FILE_TRUNCATE))
         return -FILE_ERROR;
 
     if(_file_get_properties(pf->__pathname ,pf->fst)  == -1)
         return -1;
     
     size_t fs = pf->fst->st.st_size;
     int ret = 0;
     va_list args;
 
     if(cmd == FILE_F_TRUNCATE){
         ret = ftruncate(pf->fd ,len);
     }
     else if(cmd == FILE_TRUNCATE){
         va_start(args, cmd);
         const char *path = va_arg(args, const char *);
         va_end(args);
 
         if (access(path, F_OK) != 0) {
             perror("truncate path access error");
             return -FILE_ERROR;
         }
 
         ret = truncate(path ,len);
     }
 
     if(ret == -1){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     if(_file_set_offset(pf ,ofs ,SEEK_SET) == -1)
         return -FILE_ERROR;
 
     if(_file_get_info(pf) == -1)
         return -FILE_ERROR;
 
     pf->ret = (len > (fs))?  (len - fs): (fs - len);
     PRINT_FILE_INFO("truncate" ,pf);
     return FILE_EOK;
 }
 
 /**
  * @name _file_print
  * @brief 从指定偏移位置开始读取并打印文件内容（以文本形式输出）。
  *
  * 该函数会先保存当前文件偏移位置，然后从指定偏移 `ofs` 开始，最多读取 `len` 字节内容。
  * 内容读取成功后，按字符方式输出到标准输出（适用于打印文本类文件）。
  * 最后恢复原始偏移位置，并打印基本信息（读取字节数、文件大小、最终偏移）。
  *
  * @param pfp   指向已打开的 _file_t 结构体的指针。
  * @param ofs   从该偏移位置开始读取（相对于文件开头，SEEK_SET）。
  * @param len   要读取并打印的最大字节数。
  *
  * @return 成功返回 FILE_EOK;，失败返回 -FILE_ERROR。
  *
  * @note
  * - 如果请求打印的长度超过文件末尾，则会自动调整为剩余可读字节数；
  * - 如果读取失败，函数将立即返回错误，不会修改原文件偏移；
  * - 适用于文本内容打印，如需打印二进制建议使用十六进制打印方式；
  * - 打印完成后恢复原始偏移位置，避免影响后续读写。
  */
 int _file_print(_file_t *pfp ,off_t ofs ,size_t len)
 {
     if(pfp == NULL)
         return -FILE_ERROR;
 
     off_t ofs_k = pfp->ofs;
 
     if(_file_set_offset(pfp ,ofs ,SEEK_SET) == -1)
         return -FILE_ERROR;
     
     if(_file_get_properties(pfp->__pathname ,pfp->fst) == -1)
         return -1;
     
     len = (len > (pfp->fst->st.st_size - pfp->ofs))?  (pfp->fst->st.st_size - pfp->ofs): len;
 
     if(_file_data_init(&pfp->data ,len) == -1)
         return -FILE_ERROR;
 
     pfp->ret = read(pfp->fd ,pfp->data ,len);    
     if(pfp->ret < 0){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     printf("---------- print the contents of file: %s ----------\n", pfp->__pathname);
     for (int i = 0; i < pfp->ret; ++i) {
         putchar((char)((unsigned char *)pfp->data)[i]);
     }
 
     printf("\n");
     if(_file_set_offset(pfp ,ofs_k ,SEEK_SET) == -1)
         return -FILE_ERROR;
     
     PRINT_FILE_INFO("print" ,pfp);
     return FILE_EOK;
 }
 
 /**
  * @name _file_print_u16
  * @brief 以十六进制格式打印文件中指定位置的数据（按字节输出）。
  *
  * 该函数从指定偏移位置开始读取文件内容，并以十六进制形式（0xXX）逐字节打印，适用于调试二进制文件内容。
  * 打印完成后会恢复文件原始偏移位置，确保调用前后的状态一致。
  *
  * @param pfp  指向 _file_t 文件结构体的指针，需确保文件已打开。
  * @param ofs  打印起始偏移位置（从 ofs 字节处开始读取）。
  * @param len  要读取并打印的最大字节数（若超出文件剩余大小则截断）。
  *
  * @return FILE_EOK 成功；-FILE_ERROR 失败（包括偏移设置失败、读取失败等）。
  *
  * @note
  * - 本函数不改变文件内容，仅用于打印查看。
  * - 打印格式为 “0xXX”，每字节占一个输出项。
  * - 文件偏移在打印后会被还原，确保后续读写操作不受影响。
  */
 
 int _file_print_u16(_file_t *pfp ,off_t ofs ,size_t len)
 {
     if(pfp == NULL)
         return -FILE_ERROR;
 
     off_t ofs_k = pfp->ofs;
 
     if(_file_set_offset(pfp ,ofs ,SEEK_SET) == -1)
         return -FILE_ERROR;
     
     if(_file_get_properties(pfp->__pathname ,pfp->fst)  == -1)
         return -1;
     
     len = (len > (pfp->fst->st.st_size - pfp->ofs))?  (pfp->fst->st.st_size - pfp->ofs): len;
 
     if(_file_data_init(&pfp->data ,len) == -1)
         return -FILE_ERROR;
 
     pfp->ret = read(pfp->fd ,pfp->data ,len);    
     if(pfp->ret < 0){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     printf("---------- print the contents of file: %s ----------\n", pfp->__pathname);
     unsigned char *p = (unsigned char *)pfp->data;
     for (int i = 0; i < pfp->ret; ++i)
         printf("0x%02X ", p[i]);
 
     printf("\n");
     if(_file_set_offset(pfp ,ofs_k ,SEEK_SET) == -1)
         return -FILE_ERROR;
     
     PRINT_FILE_INFO("print" ,pfp);
     return FILE_EOK;
 }
 
 /******************************************************************************************************************/
 /**
  * @name _sfile_get_ofs
  * @brief 获取与给定文件流关联的当前文件偏移量。
  *
  * 该函数使用 `ftell` 获取文件流的当前偏移量。如果文件流无效或获取偏移量失败，
  * 则返回错误码 `-1`。此函数用于获取文件当前的读写位置。
  *
  * @param[in] pf 指向 `FILE` 类型的文件流指针，不能为空。
  *
  * @return 成功时返回当前的文件偏移量，失败时返回 `-1`。
  */
 static long _sfile_get_ofs(FILE *pf)
 {
     if(pf == NULL)
         return -1;
 
     long ofs = ftell(pf);
     if(ofs == EOF)
         PRINT_ERROR();
 
     return ofs;
 }
 
 /**
  * @name _sfile_set_ofs
  * @brief 设置文件流的偏移量。
  *
  * 该函数使用 `fseek` 设置文件流的偏移量。通过设置新的偏移位置，它可以修改文件流的
  * 当前读写位置。该函数支持三种模式：`SEEK_SET`、`SEEK_CUR` 和 `SEEK_END`。如果文件流无效，
  * 或者设置偏移量失败，将返回错误码 `-1`。
  *
  * @param[in] pf     指向 `FILE` 类型的文件流指针，不能为空。
  * @param[in] ofs    偏移量，表示新的文件位置。
  * @param[in] whence 偏移量的基准位置，可以是 `SEEK_SET`、`SEEK_CUR` 或 `SEEK_END`。
  *
  * @return 设置成功时返回新的文件偏移量，失败时返回 `-1`。
  */
 static long _sfile_set_ofs(FILE *pf ,long ofs, int whence)
 {
     if(pf == NULL)
         return -1;
 
     int ret = fseek(pf ,ofs ,whence);
     if(ret == EOF){
         PRINT_ERROR();
         return -1;
     }
 
     return _sfile_get_ofs(pf);
 }
 
 /**
  * @name _sfile_get_sz
  * @brief 获取文件流的文件大小。
  *
  * 该函数通过设置文件流的偏移量到文件末尾（`SEEK_END`），获取文件的大小。文件流的当前偏移
  * 量会在获取文件大小前后被保存并恢复。如果文件流无效或发生错误，返回 `-1`。
  *
  * @param[in] pf 指向 `FILE` 类型的文件流指针，不能为空。
  *
  * @return 成功时返回文件的大小，失败时返回 `-1`。
  */
 static off_t _sfile_get_sz(FILE *pf)
 {
     if(pf == NULL)
         return -1;
 
     long cur_ofs = _sfile_get_ofs(pf);
     if(cur_ofs == -1)
         return -1;
 
     long ofs = cur_ofs;
     cur_ofs = _sfile_set_ofs(pf ,0 ,SEEK_END);
     if(cur_ofs == -1)
         return -1;
 
     if(_sfile_set_ofs(pf ,ofs ,SEEK_SET) == -1)
         return -1;
 
     return cur_ofs;
 }
 
 /**
  * @name _sfile_fflush
  * @brief 刷新 _sfile_t 结构体中关联文件流的缓冲区。
  * 
  * 该函数会调用标准库 `fflush` 函数，确保刷新文件流的缓冲区，
  * 将缓冲区中的数据写入目标文件。它用于确保文件中的数据不会停留在内存缓冲区中。
  * 如果文件流无效或刷新失败，将返回错误码。
  * 
  * @param[in] psf 指向 `_sfile_t` 文件结构体的指针，不能为空。
  * 
  * @return 刷新成功返回 `FILE_EOK`，失败返回 `-FILE_ERROR`。
  */
 int _sfile_fflush(_sfile_t *psf)
 {
     if(psf == NULL || psf->pf == NULL)
         return -FILE_ERROR;
 
     if(fflush(psf->pf) == EOF){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     return FILE_EOK;
 }
 
 /**
  * @name _sfile_finit
  * @brief 初始化一个 _sfile_t 结构体并加载基本信息。
  *
  * 此函数为指定路径和文件名分配并初始化一个 _sfile_t 结构体，包含：
  * - 使用 access 检查路径是否有效（文件必须存在）；
  * - 使用 calloc 分配并清零结构体内存；
  * - 使用 strdup 深拷贝路径和文件名，确保结构体成员独立；
  *
  * @param path 指向文件路径字符串的指针，不能为空，且文件需存在；
  * @param name 指向文件名字符串的指针，不能为空；
  *
  * @return 成功时返回初始化后的 _sfile_t 指针，失败时返回 NULL。
  */
 _sfile_t* _sfile_finit(const char *path ,const char *name ,const char *md)
 {
     if(path == NULL || name == NULL || !CHECK_FOPEN_MODE(md))
         return NULL;
 
     //判断是否为文件而不是目录
     //判断文件是否存在
     if(access(path ,F_OK) != 0){
         perror("truncate path access error.");
         return NULL;
     }
 
     _sfile_t *psf = (_sfile_t*)calloc(1, sizeof(_sfile_t));
     if(psf == NULL)
         return NULL;
     
     psf->pf = NULL;
     psf->name = strdup(name);
     psf->path = strdup(path);
     psf->md = strdup(md);
     psf->ptr = NULL;
     psf->ofs = 0;
     if(psf->name == NULL || psf->path == NULL){
         FREE_SFILE(psf);
         psf = NULL;
         return NULL;
     }
 
     return psf;
 }
 
 /**
  * @brief 打开 _sfile_t 表示的文件。
  *
  * 使用指定的模式（如 "r", "w+", "a" 等）调用 `fopen()` 打开 `_sfile_t` 结构体中的文件路径，
  * 并初始化结构体中的 `FILE*` 成员（`pf`）和文件大小（`fsz`）。
  *
  * 本函数将检查：
  * - 参数 `psf` 是否有效；
  * - 模式 `md` 是否为合法的 `fopen` 模式；
  * - 文件能否成功打开。
  *
  * 若打开失败，将打印错误信息（包括 `errno`），但不会释放结构体资源。
  *
  * @param[in,out] psf 指向 `_sfile_t` 结构体的指针，结构体必须已通过 `_sfile_finit()` 初始化。
  * @param[in]     md  文件打开模式字符串（合法 `fopen` 模式，例如 "r", "w", "a+" 等）。
  *
  * @return 成功返回 `FILE_EOK`（0），失败返回 `-FILE_ERROR`（-1）。
  */
 int _sfile_fopen(_sfile_t *psf)
 {
     if(psf == NULL)
         return -FILE_ERROR;
 
     psf->pf = fopen(psf->path ,psf->md);    
     if(psf->pf == NULL){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     psf->fd = psf->pf->_fileno;
     psf->fsz = _sfile_get_sz(psf->pf);
     if(psf->fsz == (size_t)-1){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     PRINT_SFILE_INFO("open" ,psf);
     return FILE_EOK;
 }
 
 /**
  * @name _sfile_fcloses
  * @brief 关闭 _sfile_t 结构体中打开的文件并释放相关资源。
  *
  * 该函数会检查传入指针的有效性，若文件指针非空则关闭文件，
  * 并打印关闭信息。关闭失败时打印错误信息。
  * 关闭后将文件指针置空，最后释放结构体的 name、path 及结构体本身。
  *
  * @param psf 指向已打开文件的 _sfile_t 结构体指针，不能为空。
  */
 void _sfile_fclose(_sfile_t *psf)
 {
     if(psf == NULL || psf->pf == NULL)
         return;
     
     if(psf->pf != NULL){
         printf("%s file close..." ,psf->name);
 
         int ret = fclose(psf->pf);
             
         if(ret == EOF){
             printf(" error\n");
             PRINT_ERROR();
         }
         else{
             printf(" ok\n");
         }
         psf->pf = NULL;
     }
 
     FREE_SFILE(psf);
 }
 
 /**
  * @name _sfile_write
  * @brief 向 _sfile_t 结构体中关联的文件写入数据。
  *
  * 该函数会检查输入参数的有效性，初始化数据缓冲区，
  * 通过 fwrite 将指定数据写入文件。
  * 写入失败时打印错误信息并返回错误码。
  * 写入成功后，更新文件大小信息。
  *
  * @param[in,out] psfw 指向 _sfile_t 文件结构体的指针，不能为空。
  * @param[in]     ptr  指向待写入数据的缓冲区，不能为空。
  * @param[in]     sz   每个元素的字节大小，不能为0。
  * @param[in]     nmemb 元素个数，不能为0。
  *
  * @return 写入成功返回 FILE_EOK，失败返回 -FILE_ERROR。
  */
 int _sfile_fwrite(_sfile_t *psfw ,const void *ptr ,size_t sz, size_t nmemb ,long ofs ,int whence)
 {
     if(psfw == NULL  || psfw->pf == NULL || ptr == NULL || sz == 0 || nmemb == 0
                         || (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END))
         return -FILE_ERROR;
 
     psfw->ofs = _sfile_set_ofs(psfw->pf ,ofs ,whence);
     if(psfw->ofs == -1)
         return -FILE_ERROR;
     
     psfw->ret =  fwrite(ptr ,sz ,nmemb ,psfw->pf);
     if(psfw->ret < nmemb){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     psfw->fsz = _sfile_get_sz(psfw->pf);
     if(psfw->fsz == (size_t)-1){
         PRINT_ERROR();
         return -FILE_ERROR;
     }
 
     psfw->ofs = _sfile_get_ofs(psfw->pf);
     if(psfw->ofs == -1)
         return -FILE_ERROR;
 
     PRINT_SFILE_INFO("write" ,psfw);
     return FILE_EOK;
 }
 
 /**
  * @name _sfile_fread
  * @brief 从 `_sfile_t` 结构体中关联的文件读取数据。
  *
  * 该函数首先验证输入参数的有效性，然后初始化数据缓冲区。接着，通过 `fseek` 函数
  * 设置文件的偏移量，并调用 `fread` 从文件中读取数据。如果读取的数据小于预期，
  * 会检查是否由于文件结束（EOF）或读取错误导致。若有错误，则通过 `clearerr` 清除
  * 错误标志。读取成功后，更新文件的当前偏移量，并打印文件信息。
  *
  * @param[in,out] psfr 指向 `_sfile_t` 文件结构体的指针，不能为空。
  * @param[in]     sz   每个元素的字节大小，不能为 0。
  * @param[in]     nmemb 元素的个数，不能为 0。
  * @param[in]     ofs  设置文件的偏移量，基于 `whence` 参数。
  * @param[in]     whence 偏移量的起始位置，支持 `SEEK_CUR`, `SEEK_SET`, `SEEK_END`。
  *
  * @return 读取成功返回 `FILE_EOK`，失败返回 `-FILE_ERROR`。
  */
 int _sfile_fread(_sfile_t *psfr ,size_t sz, size_t nmemb ,long ofs ,int whence)
 {
     if(psfr == NULL  || psfr->pf == NULL || sz == 0 || nmemb == 0
                     || (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END))
         return -FILE_ERROR;
     
     if(_file_data_init(&psfr->ptr ,sz * nmemb) == -1)
         return -FILE_ERROR;
   
     psfr->ofs = _sfile_set_ofs(psfr->pf ,ofs ,whence);
     if(psfr->ofs == -1)
         return -FILE_ERROR;   
 
     psfr->ret = fread(psfr->ptr ,sz ,nmemb ,psfr->pf);
     if(psfr->ret < nmemb)
     {
         if(feof(psfr->pf))
             printf("End-of-file flag is set, reached the end of the file.\n");
 
         if(ferror(psfr->pf))
             printf("read file error.\n");
 
         clearerr(psfr->pf);
     }
     
     psfr->ofs = _sfile_get_ofs(psfr->pf);
     if(psfr->ofs == -1)
         return -FILE_ERROR;
 
     PRINT_SFILE_INFO("read" ,psfr);
     return FILE_EOK;
 }
 
 /**
  * @name _sfile_print
  * @brief 从 `_sfile_t` 结构体中关联的文件读取数据并打印到标准输出。
  *
  * 该函数会先保存当前文件偏移量，然后通过 `fseek` 设置文件的偏移位置。接着，
  * 使用 `fread` 从文件中读取数据到缓冲区，并打印读取的数据。读取数据后，会
  * 恢复文件的原始偏移量，并打印文件信息。如果读取过程中遇到文件结束标志（EOF）
  * 或错误，会分别进行处理并打印相关提示。
  *
  * @param[in,out] psfp 指向 `_sfile_t` 文件结构体的指针，不能为空。
  * @param[in]     sz   每个元素的字节大小，不能为 0。
  * @param[in]     nmemb 元素的个数，不能为 0。
  * @param[in]     ofs  设置文件的偏移量，基于 `whence` 参数。
  * @param[in]     whence 偏移量的起始位置，支持 `SEEK_CUR`, `SEEK_SET`, `SEEK_END`。
  *
  * @return 打印成功返回 `FILE_EOK`，失败返回 `-FILE_ERROR`。
  */
 int _sfile_print(_sfile_t *psfp ,size_t sz, size_t nmemb ,long ofs ,int whence)
 {
     if(psfp == NULL || psfp->pf == NULL)
         return -FILE_ERROR;
     
     off_t ofs_k = psfp->ofs;
     psfp->ofs = _sfile_set_ofs(psfp->pf ,ofs ,whence);
     if(psfp->ofs == -1)
         return -FILE_ERROR; 
 
     if(_file_data_init(&psfp->ptr ,sz * nmemb) == -1)
         return -FILE_ERROR; 
         
     psfp->ret = fread(psfp->ptr ,sz ,nmemb ,psfp->pf);
     if(psfp->ret < nmemb)
     {
         if(feof(psfp->pf))
             printf("End-of-file flag is set, reached the end of the file.\n");
 
         if(ferror(psfp->pf))
             printf("read file error.\n");
 
         clearerr(psfp->pf);
     }
 
     printf("\n*----------------------------------------------------*\n");
     for (int i = 0; i < psfp->ret; ++i) {
         putchar((char)((unsigned char *)psfp->ptr)[i]);
     }
     printf("\n*----------------------------------------------------*\n");
 
     psfp->ofs = _sfile_set_ofs(psfp->pf ,ofs_k ,SEEK_SET);
     if(psfp->ofs == -1)
         return -FILE_ERROR;
     
     PRINT_SFILE_INFO("print" ,psfp);
     return FILE_EOK;
 }
 