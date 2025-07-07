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
 * @struct __thread_struct
 * @brief  线程结构体，封装线程相关信息
 * 
 * @details
 * 该结构体封装了线程的名称、线程ID、线程属性、调度策略及参数，
 * 以及线程入口函数和线程执行时的参数与返回值。
 * 主要用于线程的创建、属性配置、调度控制以及线程结果的管理。
 *
 * 成员说明：
 * - __name          : 线程名称，最长19字符，最后一个字符为字符串终止符'\0'；
 * - __id            : 线程ID，由 pthread_create 创建后分配，唯一标识线程；
 * - __attr          : 线程属性，类型为 pthread_attr_t，用于配置线程栈大小、分离状态等属性；
 * - __policy        : 线程调度策略，如 SCHED_OTHER、SCHED_FIFO、SCHED_RR 等；
 * - __inheritsched  : 调度继承属性，取值为 PTHREAD_INHERIT_SCHED 或 PTHREAD_EXPLICIT_SCHED，
 *                     表示线程是否继承创建者线程的调度属性；
 * - __param         : 线程调度参数，包含线程优先级等调度信息；
 * - __op            : 操作标志，用于指示是否调用实时调度策略相关函数。
 *                     例如：
 *                       - 0 表示不调用，使用默认调度策略；
 *                       - 1 表示需要调用，设置实时调度策略和优先级；
 * - __start_routine : 线程入口函数指针，指向线程执行的函数，接受 void* 参数并返回 void*；
 * - __ret           : 线程退出时返回的指针，可用于存储线程函数执行结果；
 * - __data          : 传递给线程入口函数的参数指针。
 */
struct __thread_struct
{
    char __name[20];                     ///< 线程名称，最多19字符+终止符
    pthread_t __id;                      ///< 线程ID，由 pthread_create 生成

    pthread_attr_t __attr;               ///< 线程属性
    int __policy;                        ///< 线程调度策略，如 SCHED_OTHER、SCHED_FIFO、SCHED_RR 等
    int __inheritsched;                  ///< 调度继承属性，PTHREAD_INHERIT_SCHED 或 PTHREAD_EXPLICIT_SCHED
    struct sched_param __param;          ///< 线程调度参数，包含优先级等信息
    int __op;                           ///< 操作标志，判断是否调用实时策略相关函数（0=默认，1=启用实时策略）

    void *(*__start_routine) (void *);   ///< 线程入口函数指针，线程执行的函数
    void *__ret;                         ///< 线程退出时返回的值指针，可用于传递线程结果
    void *__data;                        ///< 线程函数参数指针，传递给线程入口函数的数据
};
typedef struct __thread_struct __thd_t;

/**
 * @enum thread_op_t
 * @brief  线程操作标志枚举，用于指示线程创建时是否设置实时调度策略等
 */
typedef enum 
{
    THREAD_OP_DEFAULT = 0,   ///< 默认操作，不设置实时调度策略
    THREAD_OP_REALTIME = 1  ///< 使用实时调度策略，调用调度策略及优先级设置函数
    // 可根据需求扩展其他操作类型
}thread_op_t;

/* 接口函数声明 */
int __thread_getschedparam(pthread_t __id ,int *__policy ,struct sched_param *__param);
int __thread_setschedparam(pthread_t __id ,int __policy ,struct sched_param __param);
int __thread_attr_setinheritsched(__thd_t *__pthd ,int __inheritsched);
int __thread_attr_setschedpolicy(__thd_t *__pthd , int __policy);
int __thread_attr_setschedparam(__thd_t *__pthd , struct sched_param __param);
int __thread_attr_init(__thd_t *__pthd);
int __thread_attr_destroy(__thd_t *__pthd);


pthread_t __thread_getid(void);
int __thread_join(pthread_t __id ,void **__tret);
int __thread_cancel(pthread_t __id);
int __thread_detach(pthread_t __id);
__thd_t *__thread_init(char *__name);
int __thread_create(__thd_t *__pthd);
void __thread_free(__thd_t **__pthd);
void __thread_exit(__proc_t *__proc ,__thd_t *__pthd ,void *__ret);

/**
 * @brief 更新线程结构体中的线程ID及调度策略信息
 *
 * 该宏从当前线程中获取线程ID，并调用__thread_getschedparam
 * 获取该线程的调度策略和优先级，保存到传入的线程结构体成员中。
 *
 * @param __pthd  指向线程结构体的指针，必须有效且已初始化。
 *
 * @note
 * - 调用前确保 __pthd 指向合法的线程结构体；
 * - __thread_getid() 应返回当前线程的 pthread_t ID；
 * - 该宏不检查函数调用的返回值，使用时可根据需求添加错误处理。
 */
#define THREAD_REFRESH_SCHED_INFO(__pthd)\
                        do{\
                            (__pthd)->__id = __thread_getid();\
                            __thread_getschedparam((__pthd)->__id, &(__pthd)->__policy, &(__pthd)->__param);\
                        }while(0)

#endif

