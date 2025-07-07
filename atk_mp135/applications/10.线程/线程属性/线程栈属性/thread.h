/**
 * @file    thread.h
 * @brief   线程模块头文件，定义线程结构体及线程操作接口
 *
 * @details
 * 本文件定义线程相关的数据结构和函数接口，封装线程的基本信息
 * 及线程的创建、管理和退出操作。通过前向声明避免进程结构体
 * 的循环依赖问题，提升模块耦合度。
 *
 * 主要内容：
 *  - __proc_t 的前向声明，用于避免与进程模块的循环依赖；
 *  - __thread_struct 线程结构体定义，封装线程名称、线程ID、
 *    线程属性和线程执行入口函数；
 *  - 线程相关接口函数声明：
 *      - __thread_getid       : 获取当前线程ID；
 *      - __thread_init        : 初始化线程结构体；
 *      - __thread_create      : 创建线程（封装 pthread_create）；
 *      - __thread_free        : 释放线程结构体内存；
 *      - __thread_exit        : 线程退出并清理资源。
 *
 * @note
 * 线程入口函数的形参和返回值均为 void*，兼容 pthread 标准。
 */

#ifndef __THREAD_H
#define __THREAD_H

#include "file.h"
#include "log.h"
#include "tsync.h"

/* 
 * 前向声明及类型别名定义：
 * 告诉编译器 __proc_t 是 struct __proc_struct 的别名，
 * 但此时并未定义 struct __proc_struct 的具体内容。
 * 这样可以在不暴露结构体内部细节的情况下，
 * 使用指向该结构体的指针（如 __proc_t *）进行函数参数声明，
 * 以避免头文件之间的循环依赖和耦合。
 */
typedef struct __proc_struct __proc_t;

/**
 * @struct __thd_once_t
 * @brief  线程一次性初始化结构体
 *
 * @details
 * 该结构体用于封装 pthread_once 一次性初始化机制，确保某个初始化函数
 * 在多个线程中只被执行一次，通常用于延迟初始化或资源的单例模式创建。
 *
 * 成员说明：
 * - __once_control  : 控制标志，类型为 pthread_once_t，必须初始化为 PTHREAD_ONCE_INIT，
 *                     用于保证初始化函数在整个进程生命周期内只被调用一次；
 * - __init_routine  : 初始化函数指针，指向需要被“只执行一次”的函数；
 *                     该函数应无参数、无返回值。
 *
 * 示例：
 *   static __thd_once_t my_once = {
 *       .__once_control = PTHREAD_ONCE_INIT,
 *       .__init_routine = my_module_init
 *   };
 *   pthread_once(&my_once.__once_control, my_once.__init_routine);
 */
typedef struct
{
    pthread_once_t __once_control;
    void (*__init_routine)(void);
}__thd_once_t; 

int __thread_once(__thd_once_t *__once);

/**
 * @struct __thread_struct
 * @brief  线程结构体，封装线程相关信息
 * 
 * @details
 * 该结构体封装了线程的名称、线程ID、线程属性、调度策略及参数，
 * 线程栈的起始地址与大小，以及线程入口函数和执行参数。
 * 主要用于线程的创建、属性配置、调度控制和线程结果管理。
 *
 * 成员说明：
 * - __name          : 线程名称，最多19字符 + 字符串终止符 '\0'；
 * - __id            : 线程ID，由 pthread_create 创建后分配，唯一标识线程；
 * - __attr          : 线程属性对象（pthread_attr_t），用于配置线程栈大小、分离状态等；
 * - __policy        : 线程调度策略，如 SCHED_OTHER、SCHED_FIFO、SCHED_RR 等；
 * - __inheritsched  : 调度继承属性，取值为 PTHREAD_INHERIT_SCHED 或 PTHREAD_EXPLICIT_SCHED，
 *                     指示线程是否继承父线程的调度属性；
 * - __param         : 线程调度参数，包含线程优先级等调度信息；
 * - __op            : 操作标志，指示是否启用实时调度策略相关设置（0=默认，1=启用）；
 * - __stack_addr    : 线程栈的起始地址，若为 0 或 NULL，表示使用系统默认栈；
 * - __stack_sz      : 线程栈大小（字节数），需不小于系统定义的 PTHREAD_STACK_MIN；
 * - __start_routine : 线程入口函数指针，函数签名为 void* (*)(void*)；
 * - __data          : 传递给线程入口函数的参数指针。
 */
struct __thread_struct
{
    char __name[20];                   ///< 线程名称，最多19字符+终止符
    pthread_t __id;                    ///< 线程ID，由 pthread_create 生成

    pthread_attr_t __attr;             ///< 线程属性
    int __policy;                      ///< 线程调度策略，如 SCHED_OTHER、SCHED_FIFO、SCHED_RR 等
    int __inheritsched;                ///< 调度继承属性，PTHREAD_INHERIT_SCHED 或 PTHREAD_EXPLICIT_SCHED
    struct sched_param __param;        ///< 线程调度参数，包含优先级等信息
    int __op;                          ///< 操作标志，判断是否调用实时策略相关函数（0=默认，1=启用实时策略）

    void *__stack_addr;                ///< 线程栈起始地址，0/NULL 表示默认栈
    size_t __stack_sz;                 ///< 线程栈大小，单位字节，需≥ PTHREAD_STACK_MIN

    void *(*__start_routine) (void *); ///< 线程入口函数指针，线程执行的函数
    void *__data;                      ///< 线程函数参数指针，传递给线程入口函数的数据
};
typedef struct __thread_struct __thd_t;

/**
 * @enum   thread_op_t
 * @brief  线程创建操作类型标志，用于控制线程初始化行为
 *
 * @details
 * 该枚举定义了多个可组合的线程初始化选项，配合 __thd_t 结构体中的 __op 字段使用。
 * 每个位表示一个独立的线程配置操作，可按位或（|）组合使用：
 *   - 是否启用实时调度（设置显式调度策略与优先级）；
 *   - 是否设置线程为分离（detached）状态；
 *   - 是否显式设置线程栈大小；
 *   - 可拓展绑定 CPU、线程名称等其他功能。
 */
typedef enum 
{
    THREAD_OP_DEFAULT        = 0,         ///< 0b000：默认操作，使用系统默认调度策略与属性
    THREAD_OP_REALTIME       = (1 << 0),  ///< 0b001：启用实时调度策略（SCHED_FIFO / SCHED_RR），并设置优先级
    THREAD_OP_DETACHED       = (1 << 1),  ///< 0b010：将线程设置为分离状态，线程退出后自动释放资源
    THREAD_OP_STACKSIZE      = (1 << 2)   ///< 0b100：启用自定义栈大小，需设置 __stacksize 字段
    // 可拓展更多操作标志，如 THREAD_OP_CPUAFFINITY = (1 << 3) 表示绑定 CPU 等
}thread_op_t;


/* 接口函数声明 */
int __thread_attr_getstack(__thd_t *__pthd ,void **__stackaddr, size_t *__stacksize);
int __thread_attr_setstacksize(__thd_t *__pthd ,size_t __stacksize);
int __thread_attr_setstack(__thd_t *__pthd ,void *__stackaddr, size_t __stacksize);
int __thread_attr_getdetachstate(__thd_t *__pthd ,int *__detachstate);
int __thread_attr_setdetachstate(__thd_t *__pthd ,int __detachstate);

int __thread_getschedparam(pthread_t __id ,int *__policy ,struct sched_param *__param);
int __thread_setschedparam(pthread_t __id ,int __policy ,struct sched_param __param);
int __thread_attr_setinheritsched(__thd_t *__pthd ,int __inheritsched);
int __thread_attr_setschedpolicy(__thd_t *__pthd , int __policy);
int __thread_attr_setschedparam(__thd_t *__pthd , struct sched_param __param);
int __thread_attr_init(__thd_t *__pthd);
int __thread_attr_destroy(__thd_t *__pthd);

pthread_t __thread_getid(void);
int __thread_join(__thd_t *__pthd ,void *__ret);
int __thread_cancel(__thd_t *__pthd);
int __thread_detach(__thd_t *__pthd);
__thd_t *__thread_init(char *__name);
int __thread_create(__thd_t *__pthd);
void __thread_free(__thd_t **__pthd);
void __thread_exit(__proc_t *__proc ,__thd_t *__pthd ,int __ret);

/**
 * @brief 更新线程结构体中的线程ID、调度策略和栈信息
 *
 * 该宏执行以下操作：
 *  1. 调用 __thread_getid() 获取当前线程的 pthread_t 线程ID，并保存到线程结构体的 __id 成员；
 *  2. 调用 __thread_getschedparam() 获取该线程的调度策略（__policy）和调度参数（__param）；
 *  3. 调用 __thread_attr_getstack() 获取线程属性中栈的起始地址（__stack_addr）和大小（__stack_sz）。
 *
 * @param __pthd  指向自定义线程结构体 __thd_t 的有效指针，结构体必须已初始化。
 *
 * @note
 * - 调用前需确保 __pthd 指向合法且有效的线程结构体；
 * - __thread_getid() 应返回当前线程的 pthread_t 类型ID；
 * - __thread_getschedparam() 和 __thread_attr_getstack() 需保证正确实现；
 * - 本宏未对函数调用返回值进行错误检查，实际使用时可自行添加错误处理逻辑；
 * - 该宏可用于同步当前线程状态到结构体，方便后续调度和管理操作。
 */
#define THREAD_REFRESH_SCHED_INFO(__pthd)\
                        do{\
                            (__pthd)->__id = __thread_getid();\
                            __thread_getschedparam((__pthd)->__id, &(__pthd)->__policy, &(__pthd)->__param);\
                            __thread_attr_getstack((__pthd) ,&((__pthd)->__stack_addr) ,&(__pthd)->__stack_sz);\
                        }while(0)

#endif

