/**
 * @file    log.h
 * @brief   日志模块接口定义
 *
 * 本头文件定义了用于日志输出的结构体 `__log_t`，以及日志模块的初始化、
 * 写入、释放等函数接口，适用于在应用程序中进行结构化日志记录。
 *
 * 日志功能依赖于 `file.h` 提供的文件操作接口，通过封装 `_file_t` 类型对日志文件进行读写。
 * 日志格式由调用 `_log_write()` 函数动态生成，支持时间戳与模块名拼接，便于调试和回溯。
 *
 * @note
 * - 日志文件路径固定为 `./run.log`，由宏 `LOGFILE` 定义；
 * - 调用 `_log_init()` 进行初始化后，方可写入日志；
 * - 写入完成后应调用 `_log_free()` 释放资源，避免内存泄漏。
 *
 * @author  baotou
 * @date    2025-06-9
 */
#ifndef __LOG_H
#define __LOG_H

#include "file.h"

#define LOGFILE        ("./run.log")  ///< 日志文件默认路径

/**
 * @struct _log_struct
 * @brief  日志模块内部结构体
 */
struct _log_struct
{
    _file_t *__pf;       ///< 日志文件对象指针
    time_t __timer;      ///< 上次记录的时间戳（秒级）
    char *__str;         ///< 日志输出临时缓冲区（拼接使用）
};
typedef struct _log_struct __log_t;

extern __log_t *__log;

int _log_init(void);
void _log_free(void);
int _log_write(const char *__pr_n_str, const char *__fmt_str, ...);

#endif
