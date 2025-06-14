
/**
 * @file    process.h
 * @brief   进程结构定义与辅助接口声明
 * 
 * 本头文件定义了用于管理和描述进程的结构体 `__proc_t`，并提供了一组与进程信息相关的辅助函数接口：
 * 包括进程初始化、注册退出回调函数、获取当前进程 PID 和父进程 PPID 等功能。
 * 
 * 同时提供了一个格式化宏 `PRINT_PROC_INFO` 用于在调试时以结构化格式打印进程相关信息，
 * 便于观察当前进程状态及其基本属性。
 * 
 * @note 
 * - `__proc_t` 结构体仅包含进程名、函数指针、PID 和 PPID 字段；
 * - 所有接口函数以 `_proc_` 为前缀，表示是内部模块使用；
 * - 若需要扩展多进程调度、执行等能力，建议在此基础上封装更高层接口。
 * 
 * @author  baotou
 * @date    2025-06-10
 */
#ifndef __PROCESS_H
#define __PROCESS_H

#include "file.h"

#define PROC_EOK   0x00  ///< 表示处理成功
#define PROC_ERROR 0x01  ///< 表示处理失败

/**
 * @struct __proc_struct
 * @brief  用于描述单个进程的基本信息结构体
 */
struct __proc_struct
{
    char *__name;                   ///< 进程名
    void (*__function)(void);       ///< 指向进程主函数的指针
    pid_t __pid;                    ///< 当前进程 ID
    pid_t __ppid;                   ///< 父进程 ID
};
typedef struct __proc_struct __proc_t;

/**
 * @def PRINT_PROC_INFO
 * @brief 打印进程基本信息到标准输出
 * 
 * 使用该宏可输出进程名、执行动作（字符串形式）、PID、PPID 及其函数指针地址，适合调试时使用。
 * 
 * @param action    表示当前执行的动作（如 "init", "start", "exit" 等）
 * @param proc      指向 __proc_t 类型的结构体指针
 */
#define PRINT_PROC_INFO(action, proc)\
                    printf(                                                                 \
                        "[Process Info]\n"                                                  \
                        "├─ name                     : %s\n"                                \
                        "├─ Action                   : %s\n"                                \
                        "├─ PID                      : %d\n"                                \
                        "├─ PPID                     : %d\n"                                \
                        "└─ Function                 : %p\n"                                \
                        ,                                                                   \
                        (proc)->__name ? (proc)->__name : "NULL",                           \
                        (action),                                                           \
                        (proc)->__pid,                                                      \
                        (proc)->__ppid,                                                     \
                        (void *)(proc)->__function                                          \
                    );

__proc_t* _proc_init(char *__name);
void _proc_free(__proc_t **__proc);
int _proc_atexit(void (*__fun)(void));
void __proc_getpid(pid_t *__pid);
void __proc_getppid(pid_t *__ppid);
#endif
