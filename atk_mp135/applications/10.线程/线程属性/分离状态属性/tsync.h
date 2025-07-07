/**
 * @file    ttsync.h
 * @brief   线程同步接口封装头文件
 *
 * 本文件封装了基于 POSIX 的多种线程同步原语，包括：
 *   - 互斥锁（pthread_mutex）
 *   - 条件变量（pthread_cond）
 *   - 自旋锁（pthread_spinlock）
 *   - 读写锁（pthread_rwlock）
 *   - 信号量（sem_t）
 *
 * 提供统一的结构体封装和接口函数声明，方便在多线程环境下进行资源访问控制，
 * 并支持操作模式（如阻塞、非阻塞）抽象。
 *
 * 特点包括：
 * - 使用统一数据结构（如 __tsync_mutex_t、__tsync_sem_t）封装底层同步对象；
 * - 提供初始化、销毁、加锁、解锁等通用接口函数；
 * - 提供 __tsync_op_t 枚举统一描述操作行为（如 __wait、__trywait）；
 * - 支持属性自定义（如锁类型、共享方式）；
 *
 * @note
 * - 所有同步结构均支持 `__num` 编号与 `__data` 指针绑定业务资源；
 * - 本接口适用于 C 项目的线程安全封装、嵌入式 Linux 多线程同步等场景；
 * - 建议配套实现文件使用相同命名风格（如 ttsync.c）。
 */

#ifndef __Ttsync_H
#define __Ttsync_H

#include "file.h"
#include <pthread.h>
#include <semaphore.h>

/**
 * @enum __tsync_op
 * @brief 通用同步操作类型枚举
 *
 * 用于抽象同步原语（如互斥锁、读写锁、信号量等）的操作方式，
 * 表示调用时是阻塞式还是非阻塞式等待。
 */
enum __tsync_op
{
    __wait = 0,       /**< 阻塞等待操作：调用将挂起直到资源可用（如 pthread_mutex_lock、sem_wait） */
    __trywait = 1     /**< 非阻塞尝试操作：立即尝试获取资源，若不可用则返回失败（如 pthread_mutex_trylock） */
};
typedef enum __tsync_op __tsync_op_t;

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
typedef struct __mutex_struct __tsync_mutex_t;

/* 接口函数声明 */
int __tsync_get_mutexattr(__tsync_mutex_t *__mutex);
int __tsync_set_mutexattr(__tsync_mutex_t *__mutex ,int __type);
int __tsync_mutex_lock_op(__tsync_mutex_t *__mutex ,int __op);
int __tsync_mutex_unlock(__tsync_mutex_t *__mutex);
int __tsync_mutex_init(__tsync_mutex_t *__mutex, const int *__type 
    ,void *__data ,int __num);
int __tsync_mutex_destroy(__tsync_mutex_t *__mutex);

/**
 * @struct __cond_struct
 * @brief 条件变量封装结构体
 */
struct __cond_struct
{
    __tsync_mutex_t __mutex;           /**< 互斥锁，用于配合条件变量同步 */
    pthread_condattr_t __attr;        /**< 条件变量属性，NULL 表示默认属性 */
    pthread_cond_t __obj;             /**< 条件变量对象 */
};
typedef struct __cond_struct __tsync_cond_t;

/* 接口函数声明 */
int __tsync_get_condattr(__tsync_cond_t *__cond);
int __tsync_set_condattr(__tsync_cond_t *__cond ,int __pshared);
int __tsync_cond_init(__tsync_cond_t *__cond ,const int *__cond_pshared ,
    const int *__mutex_type ,void *__data ,int __num);
int __tsync_cond_wait(__tsync_cond_t *__cond);
int __tsync_cond_signal(__tsync_cond_t *__cond);
int __tsync_cond_broadcast(__tsync_cond_t *__cond);
int __tsync_cond_destroy(__tsync_cond_t *__cond);

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
typedef struct __spinlock_struct __tsync_spin_t;

/* 接口函数声明 */
int __tsync_spin_lock_op(__tsync_spin_t *__spin ,int __op);
int __tsync_spin_unlock(__tsync_spin_t *__spin);
int __tsync_spin_init(__tsync_spin_t *__spin ,int __pshared ,void *__data ,int __num);
int __tsync_spin_destroy(__tsync_spin_t *__spin);

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
    pthread_rwlock_t __lock;                  ///< 读写锁对象
    pthread_rwlockattr_t __attr;              ///< 读写锁属性对象，可为 NULL 表示默认属性
};
typedef struct __rwlock_struct __tsync_rwlock_t;

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
int __tsync_get_rwlockattr(__tsync_rwlock_t *__rwlock ,int *__pshared);
int __tsync_set_rwlockattr(__tsync_rwlock_t *__rwlock ,int __pshared);
int __tsync_rwlock_lock(__tsync_rwlock_t *__rwlock ,int __op);
int __tsync_rwlock_trylock(__tsync_rwlock_t *__rwlock ,int __op);
int __tsync_rwlock_unlock(__tsync_rwlock_t *__rwlock);
int __tsync_rwlock_init(__tsync_rwlock_t *__rwlock ,int *__pshared ,void *__data ,int __num);
int __tsync_rwlock_destroy(__tsync_rwlock_t *__rwlock);

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
typedef struct __sem_struct __tsync_sem_t;

/* 接口函数声明 */
int __tsync_sem_wait(__tsync_sem_t *__sem ,int __op);
int __tsync_sem_timedwait(__tsync_sem_t *__sem ,int __time);
int __tsync_sem_post(__tsync_sem_t *__sem);
int __tsync_sem_getvalue(__tsync_sem_t *__sem);
int __tsync_sem_init(__tsync_sem_t *__sem ,int __pshared ,unsigned int __val ,int __num);
int __tsync_sem_destory(__tsync_sem_t *__sem);

#endif