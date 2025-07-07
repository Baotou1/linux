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

#define LOGFILE        ("/home/baotou/linux/atk_mp135/applications/run.log")  ///< 日志文件默认路径

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
int _log_write(const char *__name, const char *__fmt_str, ...);

/**
 * @brief 通用日志打印宏
 *
 * 该宏用于格式化并打印带有进程名和线程名标识的日志信息，支持不同日志等级，
 * 适用于进程和线程相关的日志记录。
 *
 * @param level  日志等级字符串，如 "INFO"、"WARN"、"ERROR" 等，打印时会加上中括号
 * @param proc   指向进程结构体的指针 (__proc_t * 类型)，支持 NULL，若为 NULL 则打印 "NULL"
 * @param thd    指向线程结构体的指针 (__thd_t * 类型)，支持 NULL，若为 NULL 则打印 "NULL"
 * @param fmt    格式化字符串，类似 printf 的格式
 * @param ...    格式化字符串对应的可变参数
 *
 * @details
 * - 根据传入的进程指针和线程指针，分别提取其 __name 字段，用于在日志中标识具体哪个进程、哪个线程
 * - 如果传入的指针为 NULL，会以字符串 "NULL" 代替，避免空指针访问导致崩溃
 * - 调用底层日志写入函数 `_log_write`，日志格式为：
 *     时间戳 [LEVEL] [proc_name][thread_name]: 格式化后的日志内容
 *
 * @note
 * - 宏内部使用 do{ ... }while(0) 结构保证单条语句的完整性，避免宏展开时产生语法问题
 * - __thd_t 类型转换强制写入线程指针，保证能访问 __name 字段
 * - 该宏依赖 `_log_write` 函数正确实现日志写入功能
 *
 * @example
 * LOG_PRINT("INFO", proc_ptr, thread_ptr, "create thread tid=%lu", thread_ptr->__id);
 * 输出示例：
 * 2025-06-24 14:30:00 [INFO] [proc1][main]: create thread tid=12345
 */
#define LOG_PRINT(level, proc, thd, fmt, ...)\
                                            do{\
                                                const char *__pname = (proc) ? (proc)->__name : "NULL";\
                                                const char *__tname = (thd) ? ((__thd_t *)(thd))->__name : "NULL";\
                                                _log_write("[" level "]", "[%s][%s]: " fmt, __pname, __tname, ##__VA_ARGS__);\
                                            }while(0)
#endif
