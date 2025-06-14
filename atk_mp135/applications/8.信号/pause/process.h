
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
#include <sys/wait.h>

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
    pid_t __cpid;
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
                        "├─ CPID                     : %d\n"                                \
                        "└─ Function                 : %p\n"                                \
                        ,                                                                   \
                        (proc)->__name ? (proc)->__name : "NULL",                           \
                        (action),                                                           \
                        (proc)->__pid,                                                      \
                        (proc)->__ppid,                                                     \
                        (proc)->__cpid,                                                     \
                        (void *)(proc)->__function ? (void *)(proc)->__function : NULL      \
                    );

__proc_t* _proc_init(char *__name);
void _proc_free(__proc_t **__proc);
int _proc_atexit(void (*__fun)(void));
void __proc_getpid(pid_t *__pid);
void __proc_getppid(pid_t *__ppid);
void __proc_dump_env(void);
char* __proc_getenv(const char *__name);
int __proc_putenv(char *__name);
int __proc_setenv(const char *__name ,const char *__value ,int __replace);
int __proc_unsetenv(const char *__name);
int __proc_clearenv(void);
pid_t __proc_fork(void);
pid_t __proc_vfork(void);
/**
 * @name    PROCESS_DUMP_ENV
 * @brief   打印当前进程的所有环境变量并记录日志
 */
#define PROCESS_DUMP_ENV()\
                            do{\
                                __proc_dump_env();\
                                _log_write(__proc->__name ,"dump env.");\
                            }while(0);
/**
 * @macro PROCESS_SET_NAME
 * @brief  设置进程名称指针，释放旧名称并复制新名称。
 *
 * 该宏用于将传入的字符串 __name 复制一份，赋值给 __proc_name 指向的字符串指针。
 * 宏先检查 __name 是否为 NULL，避免空指针操作。
 * 如果 __proc_name 已指向有效字符串，会先释放，防止内存泄漏。
 * 最后调用 strdup 复制 __name 字符串，赋值给 __proc_name。
 *
 * @param __name       要设置的新名称字符串指针，不能为 NULL。
 * @param __proc_name  目标字符串指针变量（char * 类型），不能为 NULL。
 *
 * @note 该宏不检查 strdup 返回 NULL 的情况，调用者需自行处理。
 */
#define PROCESS_SET_NAME(__name ,__proc_name)\
                                            do{\
                                                if((__name) == NULL)\
                                                    break;\
                                                if((__proc_name) != NULL){\
                                                    free(__proc_name);\
                                                    (__proc_name) = NULL;\
                                                }\
                                                (__proc_name) = strdup(__name);\
                                            }while(0)
/**
 * @macro PROCESS_REFRESH_INFO
 * @brief  刷新并打印进程相关信息。
 *
 * 该宏用于更新传入进程结构体 (__proc) 中的当前进程 ID (__pid) 和父进程 ID (__ppid)。
 * 调用内部函数 __proc_getpid 和 __proc_getppid 来获取最新的进程和父进程 ID，
 * 然后通过 PRINT_PROC_INFO 宏打印带有指定操作动作 (action) 的进程信息。
 *
 * @param action  表示当前操作或状态的字符串，如 "init"、"start" 等。
 * @param __proc  指向进程结构体的指针，不能为空。
 *
 * @note 如果 __proc 为空指针，宏将直接跳过执行，避免空指针访问。
 */
#define PROCESS_REFRESH_INFO(action ,__proc)\
                                            do{\
                                                if((__proc) == NULL)\
                                                    break;\
                                                __proc_getpid(&(__proc)->__pid);\
                                                __proc_getppid(&(__proc)->__ppid);\
                                                PRINT_PROC_INFO(action ,(__proc));\
                                            }while(0)                
#endif
