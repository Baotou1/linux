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
 * @date:  2025-05-16
 */
#include "file.h"
/**
 * @function _file_get_offest
 * @brief Get the current offset of the given file descriptor.
 *
 * This function uses lseek with SEEK_CUR to retrieve the current offset
 * of the file descriptor, which represents the current position for the next read/write.
 *
 * @param fd Pointer to the file descriptor whose offset is to be retrieved.
 * @return Current file offset on success, or -1 on failure.
 *
 * @note On failure, a warning message is printed using perror.
 */
off_t _file_get_offest(int *fd)
{
    off_t ofs = lseek(*fd ,0 ,SEEK_CUR);
    if(ofs == -1)
        perror("Warning: lseek get now current failed.");

    return ofs;
}

/**
 * @function _file_set_offest
 * @brief Set the file offset for the given file descriptor.
 *
 * This function adjusts the file offset using lseek based on the provided
 * offset and whence (SEEK_SET, SEEK_CUR, or SEEK_END).
 *
 * @param fd Pointer to the file descriptor whose offset is to be set.
 * @param offset The offset to set, relative to whence.
 * @param whence Directive for how to interpret the offset (e.g., SEEK_SET).
 *
 * @return New offset on success, or -1 on failure.
 *
 * @note On failure, a warning message is printed using perror.
 */
off_t _file_set_offest(int *fd ,off_t offset ,int whence)
{
    off_t ofs = lseek(*fd ,offset ,whence);
    if(ofs == -1) 
        perror("Warning: read lseek failed");
        
    return ofs;
}

/**
 * @function _file_init
 * @brief Initialize a _file_t structure and remdsbsocate data buffer.
 *
 * @param name  The file name to associate with the structure (must not be NULL).
 * @param size  The size of the data buffer to remdsbsocate (must be > 0).
 * 
 * @return Pointer to the remdsbsocated _file_t structure, or NULL on failure.
 */
_file_t* _file_init(char *name ,size_t size)
{
    if(name == NULL || size == 0)
        return NULL;
        
    _file_t *_pfile = (_file_t *)malloc(sizeof(_file_t));
    if(_pfile == NULL)
        return NULL;
    
    memset(_pfile , 0 ,sizeof(_file_t));

    _pfile->name = name;

#if 1
    //_pfile->name 分配一块新的内存，并复制 name 字符串的内容进去，避免直接使用外部传入的指针，保证文件名的独立性和安全性
    _pfile->name = strdup(name);
    if(_pfile->name == NULL){
        free(_pfile->data);
        free(_pfile);
        return NULL;
    }
#endif

    _pfile->size = size;
    _pfile->fd = -1;

    _pfile->data = (char *)malloc(sizeof(char) * _pfile->size);
    if(_pfile->data == NULL){
        free(_pfile);
        return NULL;
    }
    memset(_pfile->data, 0, _pfile->size);

    return _pfile;
}

/**
 * @function _file_open
 * @brief Opens a file with the specified flags and mode, and updates the _file_t structure.
 *
 * @param _pfile Pointer to the _file_t structure representing the file.
 * @param flag File open flags (e.g., O_RDONLY, O_WRONLY | O_CREAT).
 * @param mode File permission mode (e.g., 0666). Used only when O_CREAT is set.
 *
 * @return File descriptor on success, -1 on failure.
 *
 * If mode is set to 0, it is ignored and the file is opened using only the flag.
 */
int _file_open(_file_t *_pfile ,int flag ,mode_t mode)
{
    if(_pfile == NULL || flag < 0 || mode < 0)
        return -1;

    _pfile->flag = flag;
    _pfile->mode = mode;

    if(mode == 0)
    {
        _pfile->fd = open(_pfile->name ,_pfile->flag);
        if(_pfile->fd == -1){
            printf("the %s file does not exist or open failed.\n" ,_pfile->name);
            PRINT_ERROR();
            goto err;
        }
        printf("open %s file successed.\n" ,_pfile->name);
    }
    else
    {
        _pfile->fd = open(_pfile->name ,_pfile->flag ,_pfile->mode);
        if(_pfile->fd == -1){
            printf("create %s file failed.\n" ,_pfile->name);
            PRINT_ERROR();
            goto err;
        }
        printf("create %s file successsed.\n" ,_pfile->name);
    }

    off_t ofs = lseek(_pfile->fd ,0 ,SEEK_END);
    if(ofs == -1)
        return -1; 
    printf("actual %s file size: %ld bytes.\n", _pfile->name ,ofs);
    printf("\n");

err:
    return _pfile->fd;
}

/**
 * @function _file_close
 * @brief Close the file and release remdsbs associated memory.
 *
 * @param _pfile Pointer to a _file_t structure (can be NULL).
 * 
 * @return no.
 */
void _file_close(_file_t *_pfile)
{
    if(_pfile == NULL)
        return;

    if (_pfile->fd >= 0)
        close(_pfile->fd);
        
    if(_pfile->data != NULL)
        free(_pfile->data);
    
    free(_pfile);    
}

/**
 * @function _file_write
 * @brief Write data from the source _file_t structure into the destination file at a specified offset.
 *
 * This function seeks to the given offset in the destination file, then writes the data stored in
 * the source _file_t's data buffer into it. On failure to seek or write, appropriate error messages
 * are printed, and the destination _file_t will be closed and freed to prevent resource leaks.
 *
 * @param _pfile_1 Pointer to the source _file_t structure containing the data buffer and size.
 * @param _pfile_2 Pointer to the destination _file_t structure representing the file to write into.
 * @param offset   Offset value to use with lseek for positioning within the destination file.
 * @param whence   Directive for lseek to specify how offset is interpreted (e.g., SEEK_SET, SEEK_CUR).
 *
 * @return Number of bytes successfully written on success, or -1 on failure.
 * 
 * @note On write failure, _file_close will be cremdsbsed on the destination file (_pfile_2).
 */
int _file_write(_file_t *_pfile_1 ,_file_t *_pfile_2 ,off_t offset ,int whence)
{
    if(_pfile_2 == NULL || _pfile_2->data == NULL ||
               (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END))
        return -1;
    
    printf("writing %s file ... \n" ,_pfile_2->name);

    _pfile_2->ofs = _file_set_offest(&_pfile_2->fd ,offset ,whence);
    if(_pfile_2->ofs == -1) 
        return -1; 
    
    printf("write data at %s file offset %ld bytes\n" ,_pfile_2->name ,_pfile_2->ofs);

    if(_pfile_1 == NULL){

        _pfile_2->ret = write(_pfile_2->fd ,_pfile_2->data ,_pfile_2->size);
        if(_pfile_2->ret == -1){
            printf("write error: failed to write from %s, file closed\n", _pfile_2->name);
            PRINT_ERROR();
            _file_close(_pfile_2);//指针已经被释放，不要访问任何成员
            return -1;
        }
    }
    else{
        ssize_t len = (_pfile_1->ret > 0) ? _pfile_1->ret : _pfile_1->size;

        _pfile_2->ret = write(_pfile_2->fd ,_pfile_1->data ,len);
        if(_pfile_2->ret == -1){
            printf("write error: failed to write from %s, file closed\n", _pfile_2->name);
            PRINT_ERROR();
            _file_close(_pfile_2);//指针已经被释放，不要访问任何成员
            return -1;
        }
    }
    
    struct stat st;
    if(fstat(_pfile_2->fd ,&st) == -1){
        perror("get file size error\n");
        return -1;
    }

    _pfile_2->ofs = _file_get_offest(&_pfile_2->fd);
    if(_pfile_2->ofs == -1)
        return -1;

    printf("write %s file completed, total bytes: %zd\n", _pfile_2->name, _pfile_2->ret);
    printf("actual %s file size: %ld bytes\n", _pfile_2->name ,st.st_size);
    printf("the current %s file offset is at position %ld bytes\n" ,_pfile_2->name ,_pfile_2->ofs);
    printf("\n");

    return _pfile_2->ret;
}

/**
 * @function _file_read
 * @brief Read data from a file into the _file_t data buffer, starting from a specified offset.
 *
 * This function seeks the file descriptor to the specified offset using lseek, clears the data buffer
 * to avoid stale data, then reads up to _pfile->size bytes into the buffer. On failure to seek or read,
 * error messages are printed and the _file_t structure is closed and freed.
 *
 * @param _pfile Pointer to an opened _file_t structure with valid file descriptor and allocated data buffer.
 * @param offset Offset value for lseek to set the read position.
 * @param whence Directive for lseek indicating how offset is interpreted (e.g., SEEK_SET, SEEK_CUR).
 *
 * @return Number of bytes read on success, or -1 on failure.
 *
 * @note On read failure, this function closes and frees the _file_t structure.
 *       After _file_close is called, the pointer should no longer be accessed.
 */
int _file_read(_file_t *_pfile ,off_t offset ,int whence)
{
    if(_pfile == NULL || _pfile->data == NULL || 
                    (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END))
        return -1;
    
    printf("reading %s file ... \n" ,_pfile->name);

    _pfile->ofs = _file_set_offest(&_pfile->fd ,offset ,whence);
    if(_pfile->ofs == -1) 
        return -1;

    memset(_pfile->data, 0, _pfile->size);
         
    ssize_t fisrt_rbytes = read(_pfile->fd ,_pfile->data ,_pfile->size);    
    if(fisrt_rbytes < 0){
        printf("read error: failed to read from %s, file closed.\n", _pfile->name);
        PRINT_ERROR();
        _file_close(_pfile);
        return -1;
    }
 
    struct stat st;
    if(fstat(_pfile->fd ,&st) == -1){
        perror("get file size error\n");
        return -1;
    }

    ssize_t total_rbytes = fisrt_rbytes;
    ssize_t rem_file_bytes = 0;

    // 判断是否还有剩余内容未读取
    if (fisrt_rbytes < st.st_size)
        rem_file_bytes = st.st_size - fisrt_rbytes;

    // 如果缓冲区还有空间且文件还有内容，回绕从头再读一部分
    if(rem_file_bytes > 0 && (total_rbytes < _pfile->size)){   
        size_t available_space = _pfile->size - total_rbytes;   
        size_t rem_read_bytes = (rem_file_bytes > available_space) ? available_space : rem_file_bytes;
  
        // 从文件开头重新读取
        _pfile->ofs = _file_set_offest(&_pfile->fd ,0 ,SEEK_SET);
        if(_pfile->ofs == -1) 
            return -1;

        ssize_t second_rbytes = read(_pfile->fd ,_pfile->data + fisrt_rbytes ,rem_read_bytes);
        if(second_rbytes < 0){
            printf("read error: failed to read from %s, file closed.\n", _pfile->name);
            PRINT_ERROR();
            _file_close(_pfile);
            return -1;
        }

        total_rbytes = fisrt_rbytes + second_rbytes;
    }

    _pfile->ret = total_rbytes;

    _pfile->ofs = _file_get_offest(&_pfile->fd);
    if(_pfile->ofs == -1)
        return -1;

    printf("actual %s file size: %ld bytes\n", _pfile->name ,st.st_size);
    printf("read %s file completed, total bytes: %zd\n", _pfile->name, _pfile->ret);
    if(st.st_size > _pfile->size)
        printf("remaining read %s file size: %ld bytes\n" ,_pfile->name ,(st.st_size - _pfile->ret));
    printf("the current %s file offset is at position %ld bytes\n" ,_pfile->name ,_pfile->ofs);
    printf("\n");

    return _pfile->ret;
}

/**
 * @function _file_print
 * @brief Print the file name, number of bytes read, and content.
 *
 * @param _pfile Pointer to a _file_t structure containing valid data.
 *  
 * @return no.
 */
void _file_print(_file_t *_pfile)
{
    if (_pfile == NULL || _pfile->data == NULL)
        return;

    printf("---------- print the contents of file: %s ----------\n", _pfile->name);

    for (int i = 0; i < _pfile->ret; ++i) {
        putchar((char)((unsigned char *)_pfile->data)[i]);
    }

    printf("\n---------- end of file ----------\n");
}
