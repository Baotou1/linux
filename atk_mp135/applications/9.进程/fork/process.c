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
 * @name    __proc_atexit
 * @brief   注册一个函数，在进程退出时调用（等价于标准库的 atexit）
 *
 * @param[in] __fun 要在退出时调用的函数指针，不能为空
 *
 * @retval     0     注册成功
 * @retval    -1     注册失败（如参数为空或注册失败）
 */
int __proc_atexit(void (*__fun)(void))
{
    if(__fun == NULL)
        return -1;

    if(atexit(__fun) != 0)
        return -1;

    return 0;
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

/**
 * @name    __proc_getenv
 * @brief   获取指定名称的环境变量值
 *
 * @param[in] __name   环境变量的名称字符串，不能为空
 *
 * @retval    非空     返回对应环境变量的值字符串（由系统维护，不可修改）
 * @retval    NULL     指定名称不存在或参数非法
 *
 * @note
 *  - 本函数封装标准库 `getenv()`，用于获取当前进程的环境变量；
 *  - 返回的字符串指针指向系统内部数据，调用者不可释放或修改；
 *  - 适合用作进程参数解析、配置读取等场景。
 */
char* __proc_getenv(const char *__name)
{
    if(__name == NULL)
        return NULL;
 
    return getenv(__name);
}

/**
 * @name    __proc_putenv
 * @brief   设置一个新的环境变量或修改已有的环境变量（使用 putenv 接口）
 *
 * @param[in] __name  要设置的环境变量字符串，格式应为 "KEY=VALUE"，不能为空
 *
 * @retval    0       设置成功
 * @retval   -1       设置失败（如格式非法或系统错误）
 *
 * @note
 *  - 使用标准库函数 `putenv()` 实现，直接传入格式化好的字符串；
 *  - 传入字符串的生命周期需由调用者管理，不能为临时变量；
 *  - 不会自动拷贝参数，因此指针内容应保持有效。
 *
 * @warning
 *  若参数格式不为 "KEY=VALUE"，可能造成未定义行为。
 */
int __proc_putenv(char *__name)
{
    if(__name == NULL)
        return -1;

    if(putenv(__name) != 0){
        PRINT_ERROR();
        return -1;
    }
    
    return 0;
}

/**
 * @name    __proc_setenv
 * @brief   设置一个环境变量，如果已存在则可以选择是否覆盖
 *
 * @param[in] __name      环境变量名字符串，不能为空
 * @param[in] __value     环境变量值字符串，不能为空
 * @param[in] __replace   是否允许覆盖已存在的变量（非 0 表示允许覆盖）
 *
 * @retval    0           设置成功
 * @retval   -1           设置失败（参数非法或系统错误）
 *
 * @note
 *  - 封装标准库函数 `setenv()`；
 *  - 适用于需要动态设置或更新环境变量的场景；
 *  - 若设置失败，会输出错误日志。
 */
int __proc_setenv(const char *__name ,const char *__value ,int __replace)
{
    if(__name == NULL || __value == NULL)
        return -1;

    if(setenv(__name ,__value ,__replace) != 0){
        PRINT_ERROR();
        return -1;
    }
    
    return 0;
}

/**
 * @name    __proc_unsetenv
 * @brief   删除指定名称的环境变量
 *
 * @param[in] __name   环境变量名字符串，不能为空
 *
 * @retval    0        删除成功
 * @retval   -1        删除失败（如变量不存在或参数非法）
 *
 * @note
 *  - 封装标准库函数 `unsetenv()`；
 *  - 若变量不存在，不会报错；
 *  - 操作不会影响已从 `getenv()` 处获得的旧指针内容。
 */
int __proc_unsetenv(const char *__name)
{
    if(__name == NULL)
        return -1;

    if(unsetenv(__name) != 0){
        PRINT_ERROR();
        return -1;
    }
    
    return 0;
}

/**
 * @name    __proc_clearenv
 * @brief   清空当前进程的所有环境变量
 *
 * @retval    0         成功清除所有环境变量
 * @retval   -1         清除失败（如系统限制或运行时错误）
 *
 * @note
 *  - 调用标准库函数 `clearenv()`；
 *  - 清空后所有环境变量（如 PATH、HOME 等）都将不可用；
 *  - 常用于构建干净的执行环境。
 */
int __proc_clearenv(void)
{
    int __ret = clearenv();
    if(__ret == -1)
    {
        PRINT_ERROR();
    }

    return __ret;
}

/**
 * @name    __proc_fork
 * @brief   创建一个子进程（使用 fork 系统调用）
 *
 * @retval  >0         返回子进程的 PID（父进程中返回）
 * @retval   0         子进程中返回
 * @retval  -1         出错，fork 失败
 *
 * @note
 *  - fork 会复制父进程的所有资源，包括内存空间、文件描述符等；
 *  - 父子进程并发执行，必须通过返回值区分执行路径；
 *  - 子进程和父进程拥有独立的地址空间。
 */
pid_t __proc_fork(void)
{
    pid_t __ret = fork();
    if(__ret == -1)
    {
        PRINT_ERROR();
    }

    return __ret;
}

/**
 * @name    __proc_vfork
 * @brief   创建一个子进程（使用 vfork 系统调用）
 *
 * @retval  >0         返回子进程的 PID（父进程中返回）
 * @retval   0         子进程中返回
 * @retval  -1         出错，vfork 失败
 *
 * @note
 *  - vfork 与 fork 类似，但子进程与父进程共享地址空间，直到调用 exec 或 exit；
 *  - 通常用于在子进程立即调用 exec 的场景中，效率更高；
 *  - 在 vfork 返回前，父进程会阻塞，因此不能在子进程中访问父进程的栈或修改数据；
 *  - 使用不当可能导致不可预期行为，需特别小心。
 */
pid_t __proc_vfork(void)
{
    pid_t __ret = vfork();
    if(__ret == -1)
    {
        PRINT_ERROR();
    }

    return __ret;
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
void __proc_free(__proc_t **__proc)
{
    if(__proc == NULL || (*__proc) == NULL)
        return;
    
    if((*__proc)->__name != NULL){
        free((*__proc)->__name);
        (*__proc)->__name = NULL;
    }

    free((*__proc));
    (*__proc) = NULL;
}

/**
 * @name __proc_init
 * @brief   初始化一个进程结构体
 * 
 * @param[in]  __name    进程名称字符串（不能为空）
 * 
 * @retval     非空      返回已初始化的 __proc_t 指针
 * @retval     NULL      内存分配失败或输入非法
 * 
 * @note 调用者需在不再使用时通过外部接口释放此结构体
 */
__proc_t* __proc_init(char *__name)
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
    __proc->__cpid = 0;
    return __proc;
}
