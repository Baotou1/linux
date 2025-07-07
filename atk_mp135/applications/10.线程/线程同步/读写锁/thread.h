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
#include <pthread.h>
#include <semaphore.h>
#include "log.h"

/**
 * @struct __mutex_struct
 * @brief 通用线程同步数据结构体
 *
 * 该结构体用于多线程环境中对共享资源进行同步保护。
 * 内部封装资源编号、共享数据指针、互斥锁及其属性，
 * 支持通过属性配置互斥锁行为（如递归锁、错误检查锁等）。
 *
 * @note
 * - __num: 实例编号，用于标识资源、任务或对象ID；
 * - __data: 指向受保护的共享资源，类型可灵活；
 * - __lock: 互斥锁，保证对 __data 的并发安全访问；
 * - __mutexattr: 互斥锁属性，用于初始化互斥锁时配置行为。
 */
struct __mutex_struct
{
    int __num;                        ///< 实例编号，用于标识结构体（如资源ID）
    void *__data;                     ///< 通用数据指针，指向受保护的共享资源
    pthread_mutex_t __lock;           ///< 互斥锁，用于保护共享数据的并发访问
    pthread_mutexattr_t __attr;       ///< 互斥锁属性，用于配置 __mutex 行为（可选）
};
typedef struct __mutex_struct __sync_mutex_t;

/* 接口函数声明 */
int __sync_get_mutexattr(__sync_mutex_t *__mutex);
int __sync_set_mutexattr(__sync_mutex_t *__mutex ,int __type);
int __sync_mutex_init(__sync_mutex_t *__mutex, const int *__type 
                                            ,void *__data ,int __num);
int __sync_mutex_trylock(__sync_mutex_t *__mutex);
int __sync_mutex_lock(__sync_mutex_t *__mutex);
int __sync_mutex_unlock(__sync_mutex_t *__mutex);
int __sync_mutex_destroy(__sync_mutex_t *__mutex);

/**
 * @struct __cond_struct
 * @brief 条件变量封装结构体
 */
struct __cond_struct
{
    __sync_mutex_t __mutex;           /**< 互斥锁，用于配合条件变量同步 */
    pthread_condattr_t __attr;        /**< 条件变量属性，NULL 表示默认属性 */
    pthread_cond_t __obj;             /**< 条件变量对象 */
};
typedef struct __cond_struct __sync_cond_t;

/* 接口函数声明 */
int __sync_get_condattr(__sync_cond_t *__cond);
int __sync_set_condattr(__sync_cond_t *__cond ,int __pshared);
int __sync_cond_init(__sync_cond_t *__cond ,const int *__cond_pshared ,
    const int *__mutex_type ,void *__data ,int __num);
int __sync_cond_wait(__sync_cond_t *__cond);
int __sync_cond_signal(__sync_cond_t *__cond);
int __sync_cond_broadcast(__sync_cond_t *__cond);
int __sync_cond_destroy(__sync_cond_t *__cond);

/**
 * @struct __spinlock_struct
 * @brief 自旋锁封装结构体
 *
 * @details
 * 用于线程间快速同步。封装了自旋锁对象及附加元数据（编号和用户数据指针）。
 * 适用于锁持有时间极短、线程间频繁竞争的场景。
 *
 * @note
 * - __num：用于标识锁对象，如资源编号、实例 ID；
 * - __data：指向受保护的数据，便于统一封装访问；
 * - __lock：pthread 提供的原生自旋锁对象。
 */
struct __spinlock_struct
{
    int __num;                 /**< 锁编号，用于识别资源或调试 */
    void *__data;              /**< 用户共享数据指针，可为空 */
    pthread_spinlock_t __lock;  /**< 自旋锁对象，需调用 pthread_spin_* 系列函数操作 */
};
typedef struct __spinlock_struct __sync_spin_t;

/* 接口函数声明 */
int __sync_spin_lock(__sync_spin_t *__spin);
int __sync_spin_trylock(__sync_spin_t *__spin);
int __sync_spin_unlock(__sync_spin_t *__spin);
int __sync_spin_init(__sync_spin_t *__spin ,int __pshared ,void *__data ,int __num);
int __sync_spin_destroy(__sync_spin_t *__spin);

/**
 * @struct __rwlock_struct
 * @brief  读写锁结构体，封装 pthread 读写锁及其属性
 * 
 * @details
 * - 封装了 POSIX 线程库中的读写锁（`pthread_rwlock_t`）及其属性对象；
 * - 提供线程间同步的能力，适用于读多写少的场景；
 * - 支持通过 `__num` 编号识别用途，通过 `__data` 关联用户数据。
 */
struct __rwlock_struct
{
    int __num;                                ///< 结构体编号，用于标识用途
    void* __data;                             ///< 用户绑定的共享数据指针，可为 NULL
    pthread_rwlock_t __lock;                 ///< 读写锁对象
    pthread_rwlockattr_t __attr;             ///< 读写锁属性对象，可为 NULL 表示默认属性
};
typedef struct __rwlock_struct __sync_rwlock_t;

/**
 * @enum   __rwlock_op
 * @brief  表示读写锁的操作类型
 * 
 * @typedef __rwlock_op_t
 * @brief   __rwlock_op 枚举的类型别名，用于简化函数参数书写
 * 
 * @details
 * 用于标识读写锁的操作模式，供统一接口选择加读锁或写锁使用。
 * 
 * 枚举值包括：
 * - wrlock：加写锁（独占）
 * - rdlock：加读锁（共享）
 */
enum __rwlock_op
{
    wrlock = 0,
    rdlock = 1
};
typedef enum __rwlock_op __rwlock_op_t;

/* 接口函数声明 */
int __sync_get_rwlockattr(__sync_rwlock_t *__rwlock ,int *__pshared);
int __sync_set_rwlockattr(__sync_rwlock_t *__rwlock ,int __pshared);
int __sync_rwlock_lock(__sync_rwlock_t *__rwlock ,int __mode);
int __sync_rwlock_trylock(__sync_rwlock_t *__rwlock ,int __mode);
int __sync_rwlock_unlock(__sync_rwlock_t *__rwlock);
int __sync_rwlock_init(__sync_rwlock_t *__rwlock ,int *__pshared ,void *__data ,int __num);
int __sync_rwlock_destroy(__sync_rwlock_t *__rwlock);

/**
 * @struct __sem_struct
 * @brief 内部同步信号量结构体封装
 *
 * 该结构体封装了一个信号量及其相关属性，
 * 用于同步控制。
 */
struct __sem_struct
{
    int __num;      /**< 信号量编号或标识（用于内部管理） */
    int __val;      /**< 信号量当前计数值（非原子，参考值） */
    sem_t __obj;    /**< POSIX信号量对象，用于实际同步 */
    int __pshared;  /**< 进程间共享标志：
                        - 0 表示线程间共享（同一进程内）
                        - 非0 表示进程间共享 */
};
typedef struct __sem_struct __sync_sem_t;


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
 * 封装线程名称、线程ID、线程属性和线程执行函数指针，
 * 用于线程的创建、管理和调度。
 */
struct __thread_struct
{
    char __name[20];                      ///< 线程名称，最多19字符+终止符
    pthread_t __id;                       ///< 线程ID，由 pthread_create 生成
    pthread_attr_t *__attr;               ///< 线程属性指针，可为 NULL 表示默认属性
    void *(*__start_routine) (void *);   ///< 线程入口函数指针，线程执行的函数
    void *__ret;                         ///< 线程退出时返回的值指针，可用于传递线程结果
    void *__data;                        ///< 线程函数参数指针，传递给线程入口函数的数据
};
typedef struct __thread_struct __thd_t;


/* 接口函数声明 */
pthread_t __thread_getid(void);
int __thread_join(pthread_t __id ,void **__tret);
int __thread_cancel(pthread_t __id);
int __thread_detach(pthread_t __id);
__thd_t *__thread_init(char *__name ,void *(*__start_routine)(void *) ,void *__data);
int __thread_create(__thd_t *__pthd);
void __thread_free(__thd_t **__pthd);
void __thread_exit(__proc_t *__proc ,__thd_t *__pthd ,void *__ret);

#endif

