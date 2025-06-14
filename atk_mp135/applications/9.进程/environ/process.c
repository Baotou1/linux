/**
 * @file    process.c
 * @brief   进程结构体管理模块实现文件
 *
 * 本文件实现进程结构体（__proc_t）的初始化、释放、进程退出函数注册
 * 以及进程ID获取等功能，方便对进程相关信息进行管理和操作。
 *
 * 主要功能包括：
 *  - 初始化进程结构体并设置名称；
 *  - 释放进程结构体及其动态资源，并记录退出日志；
 *  - 注册进程退出时调用的函数（等同于标准库的 atexit）；
 *  - 获取当前进程ID和父进程ID。
 *
 * 依赖头文件：
 *  - "process.h" 进程结构体定义及函数声明；
 *  - "log.h" 日志写入接口。
 *
 * @note
 * - 所有接口需保证传入参数有效，避免空指针传入；
 * - 释放函数会将指针置空防止悬挂引用。
 *
 * @author  baotou
 * @date    2025-06-10
 */
#include "process.h"
#include "log.h"

extern char **environ; /* 申明外部全局变量 environ */
/**
 * @name _proc_init
 * @brief   初始化一个进程结构体
 * 
 * @param[in]  __name    进程名称字符串（不能为空）
 * 
 * @retval     非空      返回已初始化的 __proc_t 指针
 * @retval     NULL      内存分配失败或输入非法
 * 
 * @note 调用者需在不再使用时通过外部接口释放此结构体
 */
__proc_t* _proc_init(char *__name)
{
    if(__name == NULL)
        return NULL;

    __proc_t *__proc = (__proc_t *)calloc(1 ,sizeof(__proc_t));
    if(__proc == NULL){
        return NULL;
    }
        
    __proc->__name = strdup(__name);
    if (__proc->__name == NULL){
        free(__proc);
        return NULL;
    }
    
    __proc->__pid = 0;
    __proc->__ppid = 0;
    return __proc;
}

/**
 * @name _proc_free
 * @brief   释放进程结构体及其内部资源
 * 
 * @param[in,out]  __proc   指向要释放的 __proc_t 结构体指针的指针
 * 
 * @note 
 *  - 释放过程包括结构体内部动态分配的 __name 字符串；
 *  - 释放后将调用者传入的指针置为 NULL，防止悬空指针。
 *  - 释放前写日志记录退出信息。
 */
void _proc_free(__proc_t **__proc)
{
    if(__proc == NULL || (*__proc) == NULL)
        return;
    
    _log_write((*__proc)->__name ,"exit.");
    if((*__proc)->__name != NULL){
        free((*__proc)->__name);
        (*__proc)->__name = NULL;
    }

    free((*__proc));
    (*__proc) = NULL;
}

/**
 * @name    _proc_atexit
 * @brief   注册一个函数，在进程退出时调用（等价于标准库的 atexit）
 *
 * @param[in] __fun 要在退出时调用的函数指针，不能为空
 *
 * @retval    PROC_EOK     注册成功
 * @retval    -PROC_ERROR  注册失败（如参数为空或注册失败）
 */
int _proc_atexit(void (*__fun)(void))
{
    if(__fun == NULL)
        return -PROC_ERROR;

    if(atexit(__fun) != 0)
        return -PROC_ERROR;

    return -PROC_EOK;
}

/**
 * @name    __proc_getpid
 * @brief   获取当前进程的进程ID（PID）
 *
 * @param[out] __pid 用于存放当前进程ID的指针，不能为空
 *
 * @note 如果参数为空，则函数不执行任何操作
 */
void __proc_getpid(pid_t *__pid)
{
    if(__pid == NULL)
        return;

    *__pid = getpid();
}

/**
 * @name    __proc_getppid
 * @brief   获取当前进程的父进程ID（PPID）
 *
 * @param[out] __ppid 用于存放当前父进程ID的指针，不能为空
 *
 * @note 如果参数为空，则函数不执行任何操作
 */
void __proc_getppid(pid_t *__ppid)
{
    if(__ppid == NULL)
        return;

    *__ppid = getppid();    
}

/**
 * @name    __proc_dump_env
 * @brief   打印当前进程的全部环境变量
 *
 * @note
 *  - 遍历并逐行打印全局变量 environ 中的每个环境变量；
 *  - 输出格式为标准输出（printf），每行一个环境变量；
 *  - 通常用于调试或查看进程的环境配置。
 *
 * @warning
 *  本函数直接访问全局变量 environ，建议仅在调试或必要场景中使用。
 */
void __proc_dump_env(void)
{
    for (int i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }
}