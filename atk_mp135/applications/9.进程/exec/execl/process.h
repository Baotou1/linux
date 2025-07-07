
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
#include "signal.h"

#define CHILD_PROCESS_MAX_SIZE  256
/**
 * @struct __cproc_struct
 * @brief  子进程信息结构体
 *
 * @details
 *  用于在父进程中记录多个子进程的信息，包括已创建的子进程数量、每个子进程的 PID 以及最近一次 wait() 的返回状态。
 */
struct __cproc_struct
{
    int __cnt;                                 ///< 已创建的子进程数量
    pid_t __pid[CHILD_PROCESS_MAX_SIZE];       ///< 子进程 PID 数组，最多 CHILD_PROCESS_MAX_SIZE 个
    int __sta;                                 ///< 最近一次 wait() 回收的子进程状态（即 exit status）
};
typedef struct __cproc_struct __cproc_t;

/**
 * @struct __exec_struct
 * @brief  exec 执行信息结构体
 *
 * @details
 *  用于封装执行 exec 系列系统调用时所需的信息，包括可执行文件路径、
 *  参数列表（argv）以及环境变量（envp），便于统一管理和传递执行上下文。
 */
struct __exec_struct
{
    char *__path;     ///< 要执行的程序路径（绝对路径或相对路径）
    char **__argv;    ///< 参数数组，格式与 main 的 argv 相同，必须以 NULL 结尾
    char **__envp;    ///< 环境变量数组，格式与 environ 相同，必须以 NULL 结尾
};
typedef struct __exec_struct __exec_t;

/**
 * @struct __proc_struct
 * @brief  用于描述单个进程的基本信息结构体
 */
struct __proc_struct
{
    char *__name;                   ///< 进程名，用于标识当前进程的名称
    void (*__function)(void);       ///< 指向进程主函数的指针，可在需要时调用以执行特定逻辑
    pid_t __pid;                    ///< 当前进程 ID，由操作系统分配
    pid_t __ppid;                   ///< 父进程 ID，表示创建该进程的父进程编号
    pid_t __pgid;                   ///< 进程组 ID，用于管理与该进程属于同一组的其他进程
    __cproc_t __cproc;              ///< 子进程信息结构体，记录子进程数量及其 PID 等状态信息
    __sig_t *__sig;                 ///< 信号处理结构体指针，用于封装信号注册、处理和屏蔽等功能
    __exec_t __exec;
};
typedef struct __proc_struct __proc_t;

/**
 * @macro  PRINT_CPROC_INFO
 * @brief  打印进程结构体中记录的子进程信息（数量 + PID 列表）
 *
 * @param[in] proc  指向 __proc_t 类型的指针
 */
#define PRINT_CPROC_INFO(proc)\
                            do{                                                                         \
                                if((proc) != NULL && (proc)->__cproc.__cnt != 0)                        \
                                {                                                                       \
                                    printf("[Child Process Info]\n"                                     \
                                        "├─ Count                   : %d\n"                             \
                                        "└─ PIDs                    : ", (proc)->__cproc.__cnt);        \
                                    for(int __i = 0; __i < (proc)->__cproc.__cnt; ++__i)                \
                                    {                                                                   \
                                        printf("%d ", (proc)->__cproc.__pid[__i]);                      \
                                    }                                                                   \
                                    printf("\n");                                                       \
                                }                                                                       \
                                else                                                                    \
                                {                                                                       \
                                    printf("[Child Process Info]\n"                                     \
                                        "├─ Count                   : 0\n"                              \
                                        "└─ PIDs                    : 0\n");                            \
                                }                                                                       \
                            }while(0)

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
                        "├─ PGID                     : %d\n"                                \
                        "├─ PPID                     : %d\n"                                \
                        "└─ Function                 : %p\n"                                \
                        ,                                                                   \
                        (proc)->__name ? (proc)->__name : "NULL",                           \
                        (action),                                                           \
                        (proc)->__pid,                                                      \
                        (proc)->__pgid,                                                     \
                        (proc)->__ppid,                                                     \
                        (void *)(proc)->__function ? (void *)(proc)->__function : NULL      \
                    );

int __proc_atexit(void (*__fun)(void));
void __proc_getpid(pid_t *__pid);
void __proc_getppid(pid_t *__ppid);
int __proc_getpgid(pid_t *__pgid ,pid_t __pid);
int __proc_setpgid(pid_t __pgid ,pid_t __pid);
void __proc_dump_env(void);
char* __proc_getenv(const char *__name);
int __proc_putenv(char *__name);
int __proc_setenv(const char *__name ,const char *__value ,int __replace);
int __proc_unsetenv(const char *__name);
int __proc_clearenv(void);
int __proc_execve(__exec_t *__exec);
int __proc_execv(__exec_t *__exec);
pid_t __proc_fork(__cproc_t *__cproc);
pid_t __proc_vfork(void);
void __cproc_reset(__cproc_t *__cproc ,pid_t __pid);
pid_t __proc_wait(int *__sta);
pid_t __proc_waitpid(pid_t __pid ,int *__sta ,int __opt);
void __proc_free(__proc_t **__proc);
__proc_t* __proc_init(char *__name);


/**
 * @macro  PROCESS_EXIT_COMMON
 * @brief  公共进程退出处理宏，用于释放资源并调用指定退出函数。
 *
 * @param[in] __proc     指向进程结构体指针的地址（二级指针），类型为 __proc_t **。
 * @param[in] __ret      退出码，作为退出函数的参数。
 * @param[in] __exit_fn  退出函数，可为 exit 或 _exit。
 *
 * @details
 *  此宏完成通用的资源释放流程，适用于作为其它具体退出宏的基础：
 *    1. 判断 __proc 和 *__proc 是否非空；
 *    2. 若存在信号字段 __sig，调用 _sig_free() 释放信号资源；
 *    3. 调用 __proc_free() 释放进程结构体；
 *    4. 最后通过 __exit_fn(__ret) 退出进程（exit 或 _exit）。
 *
 * @note 
 *  本宏不直接使用，请使用 PROCESS_EXIT_FLUSH 或 PROCESS_EXIT_FAST。
 */
#define PROCESS_EXIT_COMMON(__proc ,__ret ,__exit_fn)\
                                                    do{\
                                                        if((__proc) != NULL && (*__proc) != NULL)\
                                                        {\
                                                            _log_write((*__proc)->__name, "process exit(%d)", __ret);\
                                                            if((*__proc)->__sig != NULL)\
                                                                _sig_free(&(*__proc)->__sig);\
                                                            __proc_free(__proc);\
                                                        }\
                                                        __exit_fn(__ret);\
                                                    }while(0)
/**
 * @macro  PROCESS_EXIT_FLUSH
 * @brief  安全退出当前进程并释放资源，会执行缓冲区刷新等清理操作。
 *
 * @param[in] __proc     指向进程结构体指针的地址（二级指针）。
 * @param[in] __ret      进程退出码。
 *
 * @details
 *  本宏调用 exit(__ret)，适用于主进程或希望触发标准 C 库清理（如刷新 stdout/stderr、
 *  执行 atexit 注册函数等）场景。调用前会释放进程结构体与信号资源。
 */
#define PROCESS_EXIT_FLUSH(__proc, __ret)\
                                        PROCESS_EXIT_COMMON(__proc, __ret, exit)
/**
 * @macro  PROCESS_EXIT_FAST
 * @brief  快速退出当前进程并释放资源，不进行缓冲刷新或清理函数调用。
 *
 * @param[in] __proc     指向进程结构体指针的地址（二级指针）。
 * @param[in] __ret      进程退出码。
 *
 * @details
 *  本宏调用 _exit(__ret)，适用于子进程或 fork 后的分支退出，
 *  避免多次刷新或执行 atexit 函数造成副作用。调用前会释放进程结构体与信号资源。
 */
#define PROCESS_EXIT_FAST(__proc, __ret)\
                                        PROCESS_EXIT_COMMON(__proc, __ret, _exit)
/**
 * @name    PROCESS_DUMP_ENV
 * @brief   打印当前进程的所有环境变量并记录日志
 */
#define PROCESS_DUMP_ENV(__proc)\
                                do{\
                                    __proc_dump_env();\
                                    if(__proc != NULL && (__proc)->__name != NULL)\
                                    {\
                                        _log_write(__proc->__name ,"dump env.");\
                                    }\
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
                                                __proc_getpgid(&(__proc)->__pgid ,0);\
                                                if(strcmp(action ,"NULL") != 0)\
                                                {\
                                                    PRINT_PROC_INFO(action ,(__proc));\
                                                }\
                                            }while(0)                
#endif
