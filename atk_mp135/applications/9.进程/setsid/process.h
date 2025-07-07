
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
 *
 * @details
 *  本结构体用于封装一个用户进程在运行时的各种关键信息，便于进程的统一管理、调度与控制。
 *  它包含进程的标识信息（如 PID、PPID、PGID、SID）、进程入口函数指针、子进程信息、
 *  信号处理设置、exec 系列调用参数上下文、命令字符串等内容，适用于构建简化的用户进程抽象层。
 */
struct __proc_struct
{
    char *__name;                   ///< 进程名，用于标识当前进程的逻辑名称（非系统级别），便于日志与调试
    void (*__function)(void);       ///< 指向进程主函数的函数指针，可在需要时调用以执行该进程的主要逻辑
    pid_t __pid;                    ///< 当前进程 ID，由操作系统在 fork() 时分配，用于唯一标识进程
    pid_t __ppid;                   ///< 父进程 ID，表示该进程的直接父进程，由 getppid() 获取
    pid_t __pgid;                   ///< 进程组 ID，用于组织相关进程为一组（如前台/后台控制），由 getpgid() 设置/获取
    pid_t __sid;                    ///< 会话 ID（Session ID），用于标识一个终端会话，通常由 setsid() 创建
    __cproc_t __cproc;              ///< 子进程信息结构体，包含已创建的子进程数量、每个子进程的 PID 及其状态
    __sig_t *__sig;                 ///< 指向信号处理配置结构体的指针，用于注册/屏蔽/处理各类信号（如 SIGINT、SIGTERM）
    __exec_t __exec;                ///< exec 执行上下文结构体，封装 exec 系列系统调用所需的路径、参数数组、环境变量等
    char *__command;                ///< 用户传入的原始命令字符串（如用于 system() 调用或打印记录等用途）
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
                                        "├─ Count                    : 0\n"                             \
                                        "└─ PIDs                     : 0\n");                           \
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
                                    do{                                                              \
                                        printf(                                                      \
                                            "[Process Info]\n"                                       \
                                            "├─ name                     : %s\n"                     \
                                            "├─ Action                   : %s\n"                     \
                                            "├─ PID                      : %d\n"                     \
                                            "├─ PGID                     : %d\n"                     \
                                            "├─ PPID                     : %d\n"                     \
                                            "├─ SID                      : %d\n"                     \
                                            "└─ Function                 : %p\n"                     \
                                            ,                                                        \
                                            (proc)->__name ? (proc)->__name : "NULL",                \
                                            (action) ? (action) : "NULL",                            \
                                            (proc)->__pid,                                           \
                                            (proc)->__pgid,                                          \
                                            (proc)->__ppid,                                          \
                                            (proc)->__sid,                                           \
                                            (proc)->__function ? (void *)(proc)->__function : NULL   \
                                        );\
                                        PRINT_CPROC_INFO(proc);\
                                    }while(0);
int __proc_atexit(void (*__fun)(void));
void __proc_getpid(pid_t *__pid);
void __proc_getppid(pid_t *__ppid);
int __proc_getpgid(pid_t *__pgid ,pid_t __pid);
int __proc_setpgid(pid_t __pgid ,pid_t __pid);
int __proc_getsid(pid_t *__sid ,pid_t __pid);
pid_t __proc_setsid(pid_t *__sid);
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
int __proc_system(const char *__command);
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
 * @macro  PROCESS_REFRESH_INFO
 * @brief  刷新并打印当前进程的基本运行信息。
 *
 * @param  action  表示当前操作或上下文的说明字符串，例如 "init"、"start"、"fork" 等。
 * @param  __proc  指向进程信息结构体 (__proc_t *) 的指针，不能为空。
 *
 * @details
 *  该宏用于自动更新指定进程结构体中的以下成员信息：
 *    - `__pid`：当前进程 ID，通过调用 `__proc_getpid()` 获取；
 *    - `__ppid`：父进程 ID，通过 `__proc_getppid()` 获取；
 *    - `__pgid`：进程组 ID，通过 `__proc_getpgid()` 获取；
 *    - `__sid`：会话 ID，通过 `__proc_getsid()` 获取。
 *  
 *  同时，如果 `action` 字符串不为 "NULL"，则调用 `PRINT_PROC_INFO()` 宏打印刷新后的进程信息，
 *  以便调试或记录当前进程状态。
 *
 * @note
 *  - 若传入的 `__proc` 为 NULL，宏将跳过所有操作，避免空指针访问；
 *  - 建议在子进程创建、exec 前后或其他关键状态变更处调用本宏；
 *  - 若希望仅刷新信息而不打印，传入 `action` 为 "NULL" 即可；
 *  - 宏内部调用了多个封装函数，调用者需保证它们已正确定义。
 */
#define PROCESS_REFRESH_INFO(action ,__proc)\
                                            do{\
                                                if((__proc) == NULL)\
                                                    break;\
                                                __proc_getpid(&(__proc)->__pid);\
                                                __proc_getppid(&(__proc)->__ppid);\
                                                __proc_getpgid(&(__proc)->__pgid ,0);\
                                                __proc_getsid(&(__proc)->__sid ,0);\
                                                if(strcmp(action ,"NULL") != 0)\
                                                {\
                                                    PRINT_PROC_INFO(action ,(__proc));\
                                                }\
                                            }while(0)                
#endif
