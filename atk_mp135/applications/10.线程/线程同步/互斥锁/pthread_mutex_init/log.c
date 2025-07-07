/**
 * @file    log.c
 * @brief   日志模块实现文件
 *
 * 本文件实现日志功能的具体逻辑，主要包括日志模块的初始化、日志信息的写入、
 * 以及资源的释放等操作，适用于记录程序运行过程中的关键信息与错误提示。
 *
 * 日志文件路径由宏 `LOGFILE` 指定，默认路径为 `"./run.log"`。
 * 初始化时若文件存在，则以追加方式打开；否则新建文件并以独占方式打开。
 *
 * 本模块依赖文件操作接口 `_file_init()`、`_file_open()`、`_file_write()`、
 * `_file_close()` 以及路径访问宏 `__ACCESS_MODE()`。
 *
 * @note
 * - 所有接口需在调用 `_log_init()` 成功后再使用；
 * - 写入函数 `_log_write()` 支持自动添加时间戳；
 * - 所有资源在 `_log_free()` 中释放，避免内存泄漏。
 *
 * @author  baotou
 * @date    2025-06-9
 */
 #include "log.h"

 __log_t *__log = NULL;
 
/**
 * @brief   初始化日志模块
 *
 * 本函数完成以下工作：
 * 1. 分配并初始化日志结构体 `__log_t`；
 * 2. 初始化日志文件对象；
 * 3. 若日志文件已存在，以追加方式打开；否则以创建+独占方式新建；
 * 4. 分配内部缓冲区 `__str` 用于拼接日志输出内容；
 * 5. 初始化时间戳字段。
 *
 * 所有资源初始化失败时，均会清理资源避免内存泄漏。
 *
 * @retval 0       成功
 * @retval -1      失败（内存或文件操作失败）
 */
int _log_init(void)
{
    /* 分配日志结构体内存 */
    __log_t *__p_log = (__log_t *)calloc(1 ,sizeof(__log_t));
    if(__p_log == NULL)
        return -1;
    
    /* 初始化日志文件结构体 */
    _file_t *__pf =  _file_init(LOGFILE);
    if(__pf == NULL){
        free(__p_log);
        __p_log = NULL;
        return -1;
    }

    /* 判断日志文件是否存在，设置打开标志：
       存在则以读写+追加打开，
       不存在则读写+追加+创建+独占打开 */
    int __open_flags = (__ACCESS_MODE(LOGFILE ,F_OK) == 0) ? 
                                (O_RDWR | O_APPEND) : 
                                (O_RDWR | O_APPEND | O_CREAT | O_EXCL);

    /* 打开日志文件，失败则释放资源并返回 */
    if(_file_open(__pf ,__open_flags ,0774) == -FILE_ERROR){
        _file_close(__pf);
        free(__p_log);
        __p_log = NULL;
        return -1;
    }                             

    /* 分配内部缓冲区用于日志拼接 */
    char *__str = (char *)calloc(256 ,sizeof(char));
    if(__str == NULL){
        _file_close(__pf);
        free(__p_log);
        __p_log = NULL;
        return -1;
    }
    
    /* 赋值给全局日志结构体 */
    __log = __p_log;
    __log->__str = __str;
    __log->__pf = __pf;
    __log->__timer = 0;

    /* 截断日志文件为0长度，失败返回错误 */
    if(_file_truncate(__pf ,0 ,0 ,FILE_TRUNCATE) == -FILE_ERROR)
    {
        return -1;
    }
    
    return 0;
}

 
/**
 * @name    _log_free
 * @brief   释放日志相关资源。
 *
 * 该函数负责关闭日志文件并释放与日志相关的内存资源，
 * 包括关闭文件句柄 `__log->__log_pf` 和释放日志结构体 `__log` 的内存。
 *
 * 调用此函数后，日志相关的资源将被正确释放，避免内存泄漏。
 *
 * @return  无返回值。
 */
void _log_free(void)
{
    if(__log == NULL)
        return;

    if(__log->__pf != NULL)
        _file_close(__log->__pf);
        
    if(__log->__str != NULL){
        free(__log->__str);
        __log->__str = NULL;
    }    

    free(__log);
    __log = NULL;
}
 
/**
 * @name    _log_write
 * @brief   写入日志信息到日志文件。
 * 
 * @param[in]   __name      日志来源名称（如模块名、函数名），不能为空。
 * @param[in]   __fmt_str   要写入的日志格式字符串，类似 printf 格式。
 * @param[in]   ...         可变参数，用于格式化 __fmt_str。
 * 
 * @return      成功返回写入的字节数，失败返回 -1。
 * 
 * @details
 *  该函数会：
 *  1. 获取当前时间字符串；
 *  2. 将时间戳、__name 和格式化后的字符串拼接为完整日志；
 *  3. 追加写入日志文件末尾；
 *  4. 返回写入的字节长度。
 */
int _log_write(const char *__name, const char *__fmt_str, ...)
{
    /* 参数校验：全局日志指针、日志名称、格式字符串不能为空，日志文件必须存在 */
    if(__log == NULL || __name == NULL || __fmt_str == NULL || __ACCESS_MODE(LOGFILE ,F_OK) == -1)
        return -1;

    char __tim_str[32] = {0};
    /* 获取当前时间字符串，失败则返回 */
    size_t __tim_len = _file_get_time(&__log->__timer, __tim_str);
    if (__tim_len == 0)
        return -1;

    /* 格式化日志主体内容 */
    char __msg_buf[192] = {0};
    va_list args;
    va_start(args, __fmt_str);
    int __msg_len = vsnprintf(__msg_buf, sizeof(__msg_buf), __fmt_str, args);
    va_end(args);
    if(__msg_len < 0 || __msg_len >= sizeof(__msg_buf))
        return -1;

    /* 拼接完整日志字符串：时间戳 + 空格 + 名称 + 内容 + 换行 */
    snprintf(__log->__str, 256, "%s %s %s\n", __tim_str, __name, __msg_buf);

    int __len = strlen(__log->__str);
    /* 追加写入日志文件，失败返回 -1 */
    if(_file_write(__log->__pf, __log->__str, 0, SEEK_END, __len) == -1)
        return -1;

    /* 返回写入的字节数 */
    return __len;
}

 
