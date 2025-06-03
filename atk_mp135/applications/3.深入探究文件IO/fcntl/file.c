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
 * @version 1.0.3 - 25/5/21
 *
 * @par 更新记录（Change Log）
 * - 2025-05-16：初始版本，实现基础的文件封装读写功能。 baotou
 * - 2025-05-20：重构_file_read/write/print/open/init函数，便于维护和移植。 baotou
 * - 2025-05-20：修改_file_print函数，增加_file_print_u16函数。 baotou
 * - 2025-05-21：增加_file_cpfd/_pread/_write/_status_fcntl函数。 baotou
 */

#include "file.h"
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
 * @name _file_get_size
 * @brief 获取文件大小并更新 _file_t 结构体中的 fs 字段。
 *
 * 使用 fstat 函数获取文件描述符对应文件的元信息，并从中提取文件大小（以字节为单位），
 * 存入 _file_t 结构体的 fs 成员变量中。
 *
 * @param[in,out] pf 指向 _file_t 结构体的指针，结构体中必须包含有效的文件描述符 fd。
 *
 * @return 成功返回 0，失败返回 -1 并打印错误信息。
 *
 * @note 如果 fstat 失败（例如 fd 非法），函数将输出错误信息，并返回 -1。
 */

int _file_get_size(_file_t *pf)
{
    struct stat st;
    if(fstat(pf->fd ,&st) == -1){
        perror("get file size error\n");
        return -1;
    }
    pf->fs = st.st_size;
    return 0;
}
 
/**
 * @name _file_data_init
 * @brief 初始化（或重新初始化）_file_t 结构中的数据缓冲区。
 *
 * 该函数首先释放原有的 data 缓冲区（若已存在），然后根据指定的大小分配一块新的内存区域，
 * 用于存储文件读取或写入的数据。分配的缓冲区大小为 `size` 字节，初始化为 0。
 *
 * @param[in,out] pf   指向 _file_t 结构体的指针，函数将对其 data 成员进行操作。
 * @param[in]     size 缓冲区大小（以字节为单位），即需要分配的内存长度。
 *
 * @return 成功返回 0，失败返回 -1。
 *
 * @note 如果分配失败，函数会打印错误提示，但不会终止程序。
 *       若 pf->data 原本为非空，将先释放旧缓冲区防止内存泄漏。
 */
int _file_data_init(_file_t *pf ,size_t size)
{
    if(pf->data != NULL || size == 0)
        free(pf->data);

    pf->data = calloc(size ,sizeof(char));
    if(pf->data == NULL){
        printf("init %s data.\n" ,pf->name);
        return -1;
    }
    return 0;
}
 
/**
 * @name _file_close
 * @brief 关闭文件并释放关联的内存资源。
 *
 * 此函数用于关闭文件描述符，并释放 _file_t 结构体中分配的内存资源，包括文件名和数据缓冲区。
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

    printf("%s file close.\n" ,pf->name);
    if(pf->fd >= 0)
        close(pf->fd);
    
    if(pf->name !=NULL)
        free(pf->name);
    
    if(pf->data != NULL)
        free(pf->data);
    
    free(pf);    
}
 
/**
 * @name _file_init
 * @brief 初始化一个 _file_t 结构体并设置初始状态。
 *
 * 此函数为指定文件名分配并初始化一个 _file_t 结构体，包含：
 * - 使用 calloc 分配并清零结构体内存；
 * - 使用 strdup 深拷贝文件名，确保独立性；
 * - 设置文件描述符为 -1，表示尚未打开。
 *
 * @param name 指向文件名字符串的指针，不能为空。
 *
 * @return 成功时返回初始化后的 _file_t 指针，失败时返回 NULL。
 */

_file_t* _file_init(char *name)
{
    if(name == NULL)
        return NULL;
        
    _file_t *pf = (_file_t *)calloc(1 ,sizeof(_file_t));
    if(pf == NULL)
        return NULL;

    //pf->name 分配一块新的内存，并复制 name 字符串的内容进去，避免直接使用外部传入的指针，保证文件名的独立性和安全性
    pf->name = strdup(name);
    if(pf->name == NULL){
        free(pf);
        return NULL;
    }

    pf->fd = -1;

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
    
    pf->fd = open(pf->name ,pf->fg ,md);
    if(pf->fd == -1){
        PRINT_ERROR();
        return -FILE_ERROR;
    }

    if(_file_get_size(pf) == -1)
        return -FILE_ERROR;

    if(_file_get_offset(pf) == -1)
        return -FILE_ERROR;

    if(_file_data_init(pf ,1) == -1)
        return -FILE_ERROR;

    printf(
        "%s open.\n"
        "  -> file size: %ld bytes\n"
        "  -> file offset: %ld bytes\n\n",
        pf->name, pf->fs, pf->ofs
    );
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
    printf("set %s file read offset: %ld bytes\n" ,pfr->name ,pfr->ofs);
    
    if(_file_get_size(pfr) == -1)
        return -FILE_ERROR;
    
    len = (len > (pfr->fs - pfr->ofs))?  (pfr->fs - pfr->ofs): len;

    if(_file_data_init(pfr ,len) == -1)
        return -FILE_ERROR;

    pfr->ret = read(pfr->fd ,pfr->data ,len);    
    if(pfr->ret < 0){
        PRINT_ERROR();
        return -FILE_ERROR;
    }
    
    if(_file_get_offset(pfr) == -1)
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
    printf("set %s file write offset: %ld bytes\n" ,pfw->name ,pfw->ofs);

    pfw->ret = write(pfw->fd ,data ,len);
    if(pfw->ret == -1){
        PRINT_ERROR();
        return -FILE_ERROR;
    }
    
    if(_file_get_size(pfw) == -1)
        return -FILE_ERROR;

    if(_file_get_offset(pfw) == -1)
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

#define __PRINT_FILEOFS
#ifdef __PRINT_FILEOFS
    if(_file_get_offset(pfr) == -1)
        return -FILE_ERROR;
    printf("get %s file offset: %ld bytes\n" ,pfr->name ,pfr->ofs);    
#endif

    if(_file_get_size(pfr) == -1)
        return -FILE_ERROR;

    len = (len > (pfr->fs - ofs))?  (pfr->fs - ofs): len;

    if(_file_data_init(pfr ,len) == -1)
        return -FILE_ERROR;

    pfr->ret = pread(pfr->fd ,pfr->data ,len ,ofs);    
    if(pfr->ret < 0){
        PRINT_ERROR();
        return -FILE_ERROR;
    }

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
    printf("get %s file offset: %ld bytes\n" ,pfw->name ,pfw->ofs);
#endif

    pfw->ret = pwrite(pfw->fd ,data ,len ,ofs);
    if(pfw->ret == -1){
        PRINT_ERROR();
        return -FILE_ERROR;
    }
    
    if(_file_get_size(pfw) == -1)
        return -FILE_ERROR;

    if(_file_get_offset(pfw) == -1)
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
    
    if(_file_get_size(pfp) == -1)
        return -FILE_ERROR;
    
    len = (len > (pfp->fs - pfp->ofs))?  (pfp->fs - pfp->ofs): len;

    if(_file_data_init(pfp ,len) == -1)
        return -FILE_ERROR;

    pfp->ret = read(pfp->fd ,pfp->data ,len);    
    if(pfp->ret < 0){
        PRINT_ERROR();
        return -FILE_ERROR;
    }

    printf("---------- print the contents of file: %s ----------\n", pfp->name);
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
    
    if(_file_get_size(pfp) == -1)
        return -FILE_ERROR;
    
    len = (len > (pfp->fs - pfp->ofs))?  (pfp->fs - pfp->ofs): len;

    if(_file_data_init(pfp ,len) == -1)
        return -FILE_ERROR;

    pfp->ret = read(pfp->fd ,pfp->data ,len);    
    if(pfp->ret < 0){
        PRINT_ERROR();
        return -FILE_ERROR;
    }

    printf("---------- print the contents of file: %s ----------\n", pfp->name);
    unsigned char *p = (unsigned char *)pfp->data;
    for (int i = 0; i < pfp->ret; ++i)
        printf("0x%02X ", p[i]);

    printf("\n");
    if(_file_set_offset(pfp ,ofs_k ,SEEK_SET) == -1)
        return -FILE_ERROR;
    
    PRINT_FILE_INFO("print" ,pfp);
    return FILE_EOK;
}
 