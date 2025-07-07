/**
 * @file    tsync.c
 * @brief   线程同步模块实现文件
 *
 * 本文件实现线程同步相关功能，包括互斥锁、信号量、条件变量等同步机制的创建、
 * 销毁、加锁、解锁和等待等操作，确保多线程环境下的资源安全访问。
 *
 * @author  您的名字
 * @date    2025-06-30
 * @version 1.0
 *
 * @note
 * - 线程同步机制封装，提升代码复用性和易维护性。
 * - 支持多种同步原语，满足不同场景需求。
 *
 * @see tsync.h
 */
#include "tsync.h"

/**
 * @function __tsync_get_mutexattr
 * @brief 获取同步结构体中互斥锁的锁类型属性
 *
 * @param __tsync  指向同步锁结构体的指针，不能为空
 *
 * @return
 *  - 成功返回锁类型值，如 PTHREAD_MUTEX_NORMAL、PTHREAD_MUTEX_RECURSIVE 等
 *  - 失败返回负值（如 -1），表示获取失败或参数非法
 *
 * @note
 *  - 返回值既表示锁类型，也可能是错误码，调用时应判断返回值是否小于 0
 *  - 该函数直接调用 pthread_mutexattr_getattr 来获取属性
 */
int __tsync_get_mutexattr(__tsync_mutex_t *__mutex)
{
    /* 参数检查，防止空指针访问 */
    if(__mutex == NULL)
        return -1;

    int __type = 0;
    /* 调用 pthread_mutexattr_gettype 获取锁类型 */
    int __ret = pthread_mutexattr_gettype(&__mutex->__attr, &__type);
    if(__ret != 0)
        /* 获取失败，返回错误码 __ret */
        return __ret;

    /* 成功返回锁类型 */
    return __type;
}

/**
 * @function __tsync_set_mutexattr
 * @brief 设置同步结构体中互斥锁的锁类型属性
 *
 * @param __mutex 互斥锁指针，不能为空
 * @param __attr  要设置的互斥锁类型，如 PTHREAD_MUTEX_NORMAL、PTHREAD_MUTEX_RECURSIVE 等
 *
 * @return
 *  - 成功返回 0；
 *  - 失败返回非零错误码；
 *  - 参数非法时返回 -1。
 *
 * @note
 *  - 该函数直接调用 pthread_mutexattr_settype 来设置属性；
 *  - 设置属性后，需重新初始化互斥锁以使属性生效。
 */
int __tsync_set_mutexattr(__tsync_mutex_t *__mutex ,int __type)
{
    /* 参数检查，防止空指针访问 */
    if(__mutex == NULL)
        return -1;

    /* 调用 pthread_mutexattr_gettype 获取锁类型 */
    return pthread_mutexattr_settype(&__mutex->__attr, __type);
}

/**
 * @function __tsync_mutex_lock_op
 * @brief 对互斥锁进行加锁操作，支持阻塞和非阻塞两种模式
 *
 * @param __mutex   互斥锁指针，不能为空
 * @param __op      加锁操作类型，支持以下取值：
 *                  - __wait：阻塞等待加锁
 *                  - __trywait：非阻塞尝试加锁
 *
 * @retval 0       加锁成功
 * @retval -1      参数非法（如 __mutex 为 NULL 或 __op 无效）
 * @retval >0      pthread_mutex_lock 或 pthread_mutex_trylock 返回的错误码
 *
 * @note
 * - 调用成功后，必须调用 __tsync_mutex_unlock 解锁；
 * - 阻塞模式下，若互斥锁被其他线程持有，当前线程将阻塞等待；
 * - 非阻塞模式下，若互斥锁已被占用，立即返回失败；
 */
int __tsync_mutex_lock_op(__tsync_mutex_t *__mutex ,int __op)
{
    if(!__mutex)
        return -1;
    
    /* 检查操作类型是否合法 */
    if(__op != __wait && __op != __trywait)
        return -1;

    if(__op == __wait)
    {
        /* 阻塞等待直到获得互斥锁 */
        return pthread_mutex_lock(&__mutex->__lock);
    }
    else if(__op == __trywait)
    {
        /* 非阻塞尝试获取互斥锁 */
        return pthread_mutex_trylock(&__mutex->__lock);
    }
}

/**
 * @function __tsync_mutex_unlock
 * @brief 解锁互斥锁
 *
 * @param __mutex   互斥锁指针，不能为空
 *
 * @retval 0       成功
 * @retval -1      参数非法（__mutex 为 NULL）
 * @retval >0      pthread_mutex_unlock 错误码
 *
 * @note
 * - 解锁前必须持有该锁；
 * - 解锁失败常见于未加锁或锁已被破坏；
 * - 建议配合 LOG_PTHREAD_ERROR 宏做统一日志输出。
 */
int __tsync_mutex_unlock(__tsync_mutex_t *__mutex)
{
    if(!__mutex)
        return -1;
    
    return pthread_mutex_unlock(&__mutex->__lock);
}

/**
 * @function __tsync_mutex_init
 * @brief 初始化线程同步互斥结构体
 *
 * @param __mutex    互斥锁指针，不能为空
 * @param __type     可选的互斥锁类型（如 PTHREAD_MUTEX_RECURSIVE），传 NULL 使用默认类型
 * @param __data     用户自定义的共享数据指针，可为 NULL，仅保存引用
 * @param __num      同步结构体编号，用于标识资源用途
 *
 * @retval 0          初始化成功
 * @retval -1         参数非法（如 __mutex 为 NULL）
 * @retval >0         pthread_* 系列返回的错误码
 *
 * @note
 * - 若初始化失败，结构体内部状态会被重置；
 * - 成功后建议使用 __tsync_mutex_destroy 释放资源；
 * - 本函数不申请结构体内存，仅初始化其成员。
 */
int __tsync_mutex_init(__tsync_mutex_t *__mutex, const int *__type 
    ,void *__data ,int __num)
{
    if(!__mutex || __data == NULL)
        return -1;

    /* 保存用户数据指针和编号 */
    __mutex->__data = __data;
    __mutex->__num  = __num;

    /* 初始化互斥锁属性对象 */
    int __ret = pthread_mutexattr_init(&__mutex->__attr);
    if(__ret != 0){
        /* 初始化失败，重置结构体状态 */
        __mutex->__data = NULL;
        __mutex->__num  = 0;
        return __ret; 
    }

    /* 如果指定了互斥锁类型，则设置该类型 */
    if(__type != NULL)
    {
        __ret = pthread_mutexattr_settype(&__mutex->__attr ,*__type);
        if(__ret != 0){
            /* 设置类型失败，销毁属性对象，重置状态 */
            pthread_mutexattr_destroy(&__mutex->__attr);
            __mutex->__data = NULL;
            __mutex->__num  = 0;
            return __ret; 
        }
    }

    /* 使用已初始化的属性对象初始化互斥锁 */
    __ret = pthread_mutex_init(&__mutex->__lock, &__mutex->__attr);

    if(__ret != 0){
        /* 初始化互斥锁失败，销毁属性对象，重置状态 */
        pthread_mutexattr_destroy(&__mutex->__attr);
        __mutex->__data = NULL;
        __mutex->__num  = 0;
    }

    return __ret;
}

/**
 * @function __tsync_mutex_destroy
 * @brief 销毁同步结构体中的互斥锁和属性
 *
 * @param __mutex  同步结构体指针，不能为空
 *
 * @retval 0       成功销毁
 * @retval -1      参数非法或销毁失败
 *
 * @note
 * - 销毁前应确保锁未处于加锁状态；
 * - 销毁后结构体不可再使用，需重新初始化；
 * - 如果结构体或其 __data 成员由 malloc 分配，可在此释放；
 * - 不应重复销毁或销毁未初始化的锁。
 */
int __tsync_mutex_destroy(__tsync_mutex_t *__mutex)
{
    if(__mutex == NULL)
        return -1;

    /* 先销毁互斥锁本身 */
    if(pthread_mutex_destroy(&__mutex->__lock) != 0)
    {
        return -1;
    }

    /* 再销毁锁属性 */
    if(pthread_mutexattr_destroy(&__mutex->__attr) != 0)
    {
        return -1;
    }
    /* 如果 __data 是动态分配的，视情况释放 */
    // free(__mutex->__data);

    /* 如果结构体本身是动态分配的，也可以在此释放 */
    // free(__mutex);

    __mutex->__num = 0;
    __mutex->__data = NULL;

    return 0;
}

/**
 * @function __tsync_get_condattr
 * @brief 获取条件变量的 pshared 属性（进程共享标志）
 *
 * @param __cond    条件变量封装结构体指针，不能为空
 *
 * @retval 0        成功
 * @retval -1       参数非法（如 __cond 为 NULL）
 * @retval >0       pthread_condattr_getpshared 返回的错误码
 *
 * @return 成功时返回条件变量的 pshared 属性值（如 PTHREAD_PROCESS_PRIVATE 或 PTHREAD_PROCESS_SHARED）
 *         失败时返回错误码（负值或 pthread_condattr_getpshared 的错误码）
 */
int __tsync_get_condattr(__tsync_cond_t *__cond)
{
    if(__cond == NULL)
        return -1;
    
    int __pshared = 0;
    int __ret = pthread_condattr_getpshared(&__cond->__attr ,&__pshared);
    if(__ret != 0)
        return __ret;
 
    return __pshared;
}

/**
 * @function __tsync_set_condattr
 * @brief 设置条件变量的 pshared 属性（进程间共享标志）
 *
 * @param __cond    条件变量封装结构体指针，不能为空
 * @param __pshared pshared 属性值，通常为 PTHREAD_PROCESS_PRIVATE 或 PTHREAD_PROCESS_SHARED
 *
 * @retval 0        成功
 * @retval -1       参数非法（如 __cond 为 NULL）
 * @retval >0       pthread_condattr_setpshared 返回的错误码
 *
 * @note
 * - 设置条件变量是否可用于多个进程间的同步；
 * - 通常默认值为 PTHREAD_PROCESS_PRIVATE。
 */
int __tsync_set_condattr(__tsync_cond_t *__cond ,int __pshared)
{
    if(__cond == NULL)
        return -1;

    return pthread_condattr_setpshared(&__cond->__attr ,__pshared);
}

/**
 * @function __tsync_cond_wait
 * @brief 在条件变量上等待（阻塞直到被唤醒）
 *
 * @param __cond 条件变量结构体指针，不能为空
 *
 * @retval 0      等待成功
 * @retval -1     参数非法
 * @retval >0     pthread_cond_wait 返回的错误码
 *
 * @note
 * - 该函数会阻塞调用线程，直到条件变量被其他线程唤醒；
 * - 调用前应已持有对应的互斥锁；
 * - 被唤醒后，线程会重新持有互斥锁；
 * - 使用时应配合 __tsync_cond_signal 或 __tsync_cond_broadcast。
 */
int __tsync_cond_wait(__tsync_cond_t *__cond)
{
    /* 参数检查 */
    if(__cond==NULL)
        return -1;

    /* 调用系统条件等待函数，阻塞等待条件变量被唤醒 */
    return pthread_cond_wait(&__cond->__obj, &__cond->__mutex.__lock);
}

/**
 * @function __tsync_cond_signal
 * @brief 唤醒等待条件变量的一个线程
 *
 * @param __cond 条件变量结构体指针，不能为空
 *
 * @retval 0      成功唤醒一个线程
 * @retval -1     参数非法（__cond 为 NULL）
 * @retval >0     pthread_cond_signal 返回的错误码
 *
 * @note
 * - 只唤醒一个正在等待的线程(多个线程唤醒的可能不是想要的那个线程)；
 * - 通常配合 __tsync_cond_wait 使用；
 * - 必须在线程持有与条件变量关联的互斥锁时调用；
 * - 不保证立即唤醒线程，仅通知条件已满足。
 */
int __tsync_cond_signal(__tsync_cond_t *__cond)
{
    /** 参数检查 */
    if(__cond==NULL)
        return -1;

    /* 唤醒等待该条件变量的一个线程 */
    return pthread_cond_signal(&__cond->__obj);
}

/**
 * @function __tsync_cond_broadcast
 * @brief 广播唤醒所有等待该条件变量的线程
 *
 * @param __cond 条件变量结构体指针，不能为空
 *
 * @retval 0      成功唤醒所有等待线程
 * @retval -1     参数非法（__cond 为 NULL）
 * @retval >0     pthread_cond_broadcast 返回的错误码
 *
 * @note
 * - 唤醒所有因 __tsync_cond_wait 阻塞的线程；
 * - 一般用于资源状态大范围变化场景；
 * - 调用时需持有对应的互斥锁；
 * - 广播不保证线程立即抢占执行。
 */
int __tsync_cond_broadcast(__tsync_cond_t *__cond)
{
    /** 参数检查 */
    if(__cond==NULL)
        return -1;

    /* 广播唤醒所有等待该条件变量的线程 */
    return pthread_cond_broadcast(&__cond->__obj);
}

/**
 * @function __tsync_cond_init
 * @brief 初始化条件变量结构体及其内部互斥锁
 *
 * @param __cond           条件变量结构体指针，不能为空
 * @param __cond_pshared   条件变量共享属性（如 PTHREAD_PROCESS_PRIVATE），可为 NULL 表示默认
 * @param __mutex_type     互斥锁类型（如 PTHREAD_MUTEX_RECURSIVE），可为 NULL 表示默认
 * @param __data           用户自定义共享数据指针，仅保存引用，可为 NULL
 * @param __num            同步结构体编号，用于标识资源
 *
 * @retval 0               成功初始化
 * @retval -1              参数非法（如 __cond 为 NULL）
 * @retval >0              pthread_* 系列函数返回的错误码
 *
 * @note
 * - 初始化顺序：先内部互斥锁，再条件变量属性，再条件变量；
 * - 条件变量属性（pthread_condattr_t）初始化后必须在失败路径中销毁；
 * - 条件变量使用自定义属性初始化，不支持传 NULL，因此始终传 &__attr；
 * - 使用完毕请调用 __tsync_cond_destroy 释放资源，防止内存/锁泄露。
 */
int __tsync_cond_init(__tsync_cond_t *__cond ,const int *__cond_pshared ,
    const int *__mutex_type ,void *__data ,int __num)
{
    if(__cond == NULL)
        return -1;

    /* 初始化内部互斥锁 */
    int __ret = __tsync_mutex_init(&__cond->__mutex ,__mutex_type ,__data ,__num);
    if(__ret != 0)
        return __ret;

    /* 初始化条件变量属性 */
    __ret = pthread_condattr_init(&__cond->__attr);
    if(__ret != 0)
    {
        __tsync_mutex_destroy(&__cond->__mutex);
        return __ret;
    }

    /* 设置条件变量属性 */
    if(__cond_pshared != NULL)
    {
        __ret = pthread_condattr_setpshared(&__cond->__attr ,*__cond_pshared);
        if(__ret != 0)
        {
            pthread_condattr_destroy(&__cond->__attr);
            __tsync_mutex_destroy(&__cond->__mutex);
            return __ret;
        }
    }

    /* 初始化条件变量，注意传入属性指针（可能为 NULL） */
    __ret = pthread_cond_init(&__cond->__obj , &__cond->__attr);
    if(__ret != 0)
    {
        pthread_condattr_destroy(&__cond->__attr);
        /* 条件变量初始化失败，销毁互斥锁防止资源泄漏 */
        __tsync_mutex_destroy(&__cond->__mutex);
        return __ret;
    }
    return 0;
}

/**
 * @function __tsync_cond_destroy
 * @brief 销毁条件变量结构体中的资源
 *
 * @param __cond 条件变量结构体指针，不能为空
 *
 * @retval 0      成功销毁
 * @retval -1     参数非法
 * @retval >0     pthread_* 系列函数的错误码
 */
int __tsync_cond_destroy(__tsync_cond_t *__cond)
{
    /* 参数检查 */
    if(__cond == NULL)
        return -1;

    int __ret;

    /* 销毁条件变量对象 */
    __ret = pthread_cond_destroy(&__cond->__obj);
    if(__ret != 0)
        return __ret;

    /* 销毁条件变量属性 */
    __ret = pthread_condattr_destroy(&__cond->__attr);
    if(__ret != 0)
        return __ret;

    /* 销毁互斥锁结构体 */
    __ret = __tsync_mutex_destroy(&__cond->__mutex);
    return __ret;
}

/**
 * @function __tsync_spin_lock_op
 * @brief 对自旋锁进行加锁操作，支持阻塞和非阻塞两种方式
 *
 * @param __spin   自旋锁结构体指针，不能为空
 * @param __op     加锁操作类型，支持以下取值：
 *                 - __wait：阻塞方式加锁（自旋等待）
 *                 - __trywait：非阻塞方式尝试加锁
 *
 * @retval 0       加锁成功
 * @retval -1      参数非法（如 __spin 为 NULL 或 __op 无效）
 * @retval >0      pthread_spin_lock 或 pthread_spin_trylock 返回的错误码
 *
 * @note
 * - 阻塞方式下，若自旋锁被其他线程持有，当前线程会持续自旋等待直至获得锁；
 * - 自旋锁加锁期间会持续占用 CPU 资源，不适合长时间等待场景；
 * - 成功加锁后，必须调用 __tsync_spin_unlock 解锁。
 */
int __tsync_spin_lock_op(__tsync_spin_t *__spin ,int __op)
{
    if(__spin == NULL)
        return -1;

    /* 检查操作类型是否合法 */
    if(__op != __wait && __op != __trywait)
        return -1;

    if(__op == __wait)
    {
        /* 阻塞式自旋等待加锁 */
        return pthread_spin_lock(&__spin->__lock);
    }
    else if(__op == __trywait)
    {
        /* 非阻塞尝试加锁 */
        return pthread_spin_trylock(&__spin->__lock);
    }
}

/**
 * @function __tsync_spin_unlock
 * @brief 解锁自旋锁
 *
 * @param __spin   自旋锁结构体指针，不能为空
 *
 * @retval 0       解锁成功
 * @retval -1      参数非法
 * @retval >0      pthread_spin_unlock 返回的错误码
 *
 * @note
 * - 必须在当前线程已持有该锁的前提下调用；
 * - 与 __tsync_spin_lock / __tsync_spin_trylock 成对使用；
 * - 解锁后，其他等待线程可获取该锁。
 */
int __tsync_spin_unlock(__tsync_spin_t *__spin)
{
    if(__spin == NULL)
        return -1;

    return pthread_spin_unlock(&__spin->__lock);
}

/**
 * @function __tsync_spin_init
 * @brief 初始化自旋锁结构体
 *
 * @param __spin     自旋锁结构体指针，不能为空
 * @param __pshared  自旋锁共享属性，可选值为：
 *                   - PTHREAD_PROCESS_PRIVATE：当前进程内线程可见（默认）
 *                   - PTHREAD_PROCESS_SHARED：多进程间可共享
 * @param __data     用户自定义共享数据指针，不能为空（仅保存引用）
 * @param __num      结构体编号，用于标识资源用途
 *
 * @retval 0         初始化成功
 * @retval -1        参数非法
 * @retval >0        pthread_spin_init 返回的错误码
 *
 * @note
 * - 使用后应调用 __tsync_spin_destroy 释放资源；
 * - 本函数不申请结构体内存，仅初始化其成员；
 * - __data 不能为 NULL，使用前需由用户分配。
 */
int __tsync_spin_init(__tsync_spin_t *__spin ,int __pshared ,void *__data ,int __num)
{
    if(!__spin || __data == NULL)
        return -1;
    
    if(__pshared != PTHREAD_PROCESS_SHARED && __pshared != PTHREAD_PROCESS_PRIVATE)
        return -1;

    int __ret = pthread_spin_init(&__spin->__lock ,__pshared);
    if(__ret != 0)
        return __ret;

    __spin->__data = __data;
    __spin->__num = __num;
    return 0;
}

/**
 * @function __tsync_spin_destroy
 * @brief 销毁自旋锁结构体
 *
 * @param __spin  自旋锁结构体指针，不能为空
 *
 * @retval 0      成功销毁
 * @retval -1     参数非法
 * @retval >0     pthread_spin_destroy 返回的错误码
 *
 * @note
 * - 调用前确保未有线程持有该锁；
 * - 销毁后结构体内容会被重置；
 * - 如果 __data 是动态分配的资源，应由调用方负责释放。
 */
int __tsync_spin_destroy(__tsync_spin_t *__spin)
{
    if(!__spin)
        return -1;

    int __ret = pthread_spin_destroy(&__spin->__lock);
    if(__ret != 0)
        return __ret;

    __spin->__data = NULL;
    __spin->__num = 0;
    return 0;
}

/**
 * @function __tsync_get_rwlockattr
 * @brief 获取读写锁的 pshared 属性值
 *
 * @param __rwlock  读写锁结构体指针，不能为空
 * @param __pshared 指向存放 pshared 值的整数指针，不能为空
 *
 * @retval 0       获取成功
 * @retval -1      参数非法
 * @retval >0      pthread_rwlockattr_getpshared 返回的错误码
 *
 * @note
 * - pshared 用于表示读写锁是否可跨进程共享，取值一般为
 *   PTHREAD_PROCESS_SHARED 或 PTHREAD_PROCESS_PRIVATE；
 */
int __tsync_get_rwlockattr(__tsync_rwlock_t *__rwlock ,int *__pshared)
{
    if(__rwlock == NULL)
        return -1;

    return pthread_rwlockattr_getpshared(&__rwlock->__attr ,__pshared);
}

/**
 * @function __tsync_set_rwlockattr
 * @brief 设置读写锁的 pshared 属性值
 *
 * @param __rwlock  读写锁结构体指针，不能为空
 * @param __pshared 设定的 pshared 值，一般为
 *                  PTHREAD_PROCESS_SHARED 或 PTHREAD_PROCESS_PRIVATE
 *
 * @retval 0       设置成功
 * @retval -1      参数非法
 * @retval >0      pthread_rwlockattr_setpshared 返回的错误码
 *
 * @note
 * - 设置该属性决定读写锁是否可跨进程共享；
 * - 调用本函数后应确保属性生效前，读写锁未被初始化或使用。
 */
int __tsync_set_rwlockattr(__tsync_rwlock_t *__rwlock ,int __pshared)
{
    if(__rwlock == NULL)
        return -1;

    return pthread_rwlockattr_setpshared(&__rwlock->__attr ,__pshared);
}

/**
 * @function __tsync_rwlock_lock
 * @brief    加锁读写锁，根据指定模式加读锁或写锁
 *
 * @param[in] __rwlock  指向读写锁结构体的指针，不能为空
 * @param[in] __op      加锁模式：`wrlock` 表示写锁，`rdlock` 表示读锁
 *
 * @retval 0            加锁成功
 * @retval -1           参数非法（空指针或模式错误）
 * @retval >0           加锁失败，返回 pthread_rwlock_* 接口的错误码
 *
 * @note
 * - 写锁（wrlock）是独占锁，同时只允许一个线程持有；
 * - 读锁（rdlock）是共享锁，多个线程可同时持有；
 * - 调用前应先使用 `__tsync_rwlock_init()` 完成锁的初始化；
 * - 解锁请调用 `pthread_rwlock_unlock(&__rwlock->__lock)`；
 * - 建议使用 `enum __rwlock_op` 类型（wrlock/rdlock）代替魔法数字传参。
 */
int __tsync_rwlock_lock(__tsync_rwlock_t *__rwlock ,int __op)
{
    /* 参数检查：结构体指针和模式必须有效 */
    if(__rwlock == NULL)
        return -1;
    
    if(__op != wrlock && __op != rdlock)
        return -1;

    int __ret;
    /* 写加锁 */
    if(__op == wrlock)
    {
        __ret = pthread_rwlock_wrlock(&__rwlock->__lock);
        if(__ret != 0)
            return __ret;// 返回错误码，如 EBUSY
    }
    /* 读加锁 */
    else if(__op == rdlock)
    {
        __ret = pthread_rwlock_rdlock(&__rwlock->__lock);
        if(__ret != 0)
            return __ret;
    }

    return 0;
}

/**
 * @function __tsync_rwlock_trylock
 * @brief    尝试以非阻塞方式加锁读写锁（读或写模式）
 *
 * @param[in] __rwlock  指向读写锁结构体的指针，不能为空
 * @param[in] __op      加锁模式：`wrlock` 表示写锁，`rdlock` 表示读锁
 *
 * @retval 0            加锁成功
 * @retval -1           参数非法（空指针或非法模式）
 * @retval >0           加锁失败，返回 pthread_rwlock_try*lock 的错误码（如 EBUSY）
 *
 * @note
 * - 本函数使用非阻塞接口 `pthread_rwlock_tryrdlock` 和 `pthread_rwlock_trywrlock`；
 * - 若锁当前被其他线程占用，会立即返回失败而不是等待；
 * - 可用于尝试性进入临界区，避免线程长时间阻塞；
 * - 初始化读写锁请使用 `__tsync_rwlock_init()`；
 * - 解锁请使用 `pthread_rwlock_unlock(&__rwlock->__lock)`。
 */
int __tsync_rwlock_trylock(__tsync_rwlock_t *__rwlock ,int __op)
{
    /* 参数检查：结构体指针和模式必须有效 */
    if(__rwlock == NULL)
        return -1;
    
    if(__op != wrlock && __op != rdlock)
        return -1;

    int __ret;
    /* 非阻塞写加锁 */
    if(__op == wrlock)
    {
        __ret = pthread_rwlock_trywrlock(&__rwlock->__lock);
        if(__ret != 0)
            return __ret;// 返回错误码，如 EBUSY
    }
    /* 非阻塞读加锁 */
    else if(__op == rdlock)
    {
        __ret = pthread_rwlock_tryrdlock(&__rwlock->__lock);
        if(__ret != 0)
            return __ret;
    }

    return 0;
}

/**
 * @function __tsync_rwlock_unlock
 * @brief    解锁读写锁（无论是读模式还是写模式）
 *
 * @param[in] __rwlock  指向读写锁结构体的指针，不能为空
 *
 * @retval 0            解锁成功
 * @retval -1           参数非法（空指针）
 * @retval >0           解锁失败，返回 pthread_rwlock_unlock 的错误码
 *
 * @note
 * - 无论当前线程持有的是读锁还是写锁，该函数都可用于释放；
 * - 必须确保当前线程已成功加锁，否则行为未定义；
 * - 若多次加读锁，需匹配相同次数的解锁操作；
 * - 本函数是对 pthread_rwlock_unlock 的简单封装。
 */
int __tsync_rwlock_unlock(__tsync_rwlock_t *__rwlock)
{
    if(__rwlock == NULL)
        return -1;

    return pthread_rwlock_unlock(&__rwlock->__lock);
}

/**
 * @function __tsync_rwlock_init
 * @brief 初始化读写锁结构体及其属性
 *
 * @param __rwlock  读写锁结构体指针，不能为空
 * @param __pshared 指向 int 的指针，设置锁的进程共享属性，NULL 表示默认（进程私有）
 * @param __data    用户自定义数据指针，保存引用，可为 NULL
 * @param __num     结构体编号，用于标识资源或用途
 *
 * @retval 0       成功初始化
 * @retval -1      参数非法
 * @retval >0      pthread_rwlockattr_init、pthread_rwlockattr_setpshared、pthread_rwlock_init 返回的错误码
 *
 * @note
 * - 调用前应确保 __rwlock 指针有效；
 * - 初始化时先初始化读写锁属性，再根据 __pshared 设置共享属性；
 * - 若初始化任一步骤失败，会释放已申请的资源并重置结构体状态；
 * - 成功初始化后，需调用对应的销毁函数释放资源；
 * - __data 的内存管理由调用者负责，不在此函数内释放。
 */
int __tsync_rwlock_init(__tsync_rwlock_t *__rwlock ,int *__pshared ,void *__data ,int __num)
{
    if(__rwlock == NULL)
        return -1;

    __rwlock->__num = __num;
    __rwlock->__data = __data;

    /* 初始化读写锁属性 */
    int __ret = pthread_rwlockattr_init(&__rwlock->__attr);
    if(__ret != 0)
    {
        __rwlock->__num = 0;
        __rwlock->__data = NULL;
        return __ret;
    }

    /* 设置读写锁属性 */
    if(__pshared != NULL)
    {
        __ret = pthread_rwlockattr_setpshared(&__rwlock->__attr ,*__pshared);
        if(__ret != 0)
        {
            pthread_rwlockattr_destroy(&__rwlock->__attr);
            __rwlock->__num = 0;
            __rwlock->__data = NULL;
            return __ret;
        }
    }

     /* 初始化读写锁对象 */
    __ret = pthread_rwlock_init(&__rwlock->__lock ,&__rwlock->__attr);
    if(__ret != 0)
    {
        pthread_rwlockattr_destroy(&__rwlock->__attr);
        __rwlock->__num = 0;
        __rwlock->__data = NULL;
        return __ret; 
    }

    return 0;
}

/**
 * @function __tsync_rwlock_destroy
 * @brief 销毁读写锁结构体及其属性
 *
 * @param __rwlock  读写锁结构体指针，不能为空
 *
 * @retval 0       成功销毁
 * @retval -1      参数非法
 * @retval >0      pthread_rwlock_destroy 或 pthread_rwlockattr_destroy 返回的错误码
 *
 * @note
 * - 调用前应确保没有线程持有该读写锁，否则行为未定义；
 * - 先销毁读写锁对象，再销毁读写锁属性对象；
 * - 销毁成功后，结构体内成员被重置，避免悬空指针；
 * - 如果 __data 指向动态分配资源，释放责任由调用者负责；
 */
int __tsync_rwlock_destroy(__tsync_rwlock_t *__rwlock)
{
    if(__rwlock == NULL)
        return -1;

    /* 摧毁读写锁 */
    int __ret = pthread_rwlock_destroy(&__rwlock->__lock);
    if(__ret != 0)
        return __ret;

    /* 摧毁读写锁属性 */
    __ret = pthread_rwlockattr_destroy(&__rwlock->__attr);
    if(__ret != 0)
        return __ret;
    
    __rwlock->__num = 0;
    __rwlock->__data = NULL;
    return 0;
}

/*
 * 函数名: __tsync_sem_wait
 * 功  能: 封装对信号量的等待操作（支持阻塞和非阻塞）
 *
 * 参  数:
 *   - __sem : 指向自定义信号量结构体的指针，不能为空
 *   - __op  : 操作类型，支持以下两种：
 *             - __wait     ：阻塞等待（对应 sem_wait）
 *             - __trywait  ：非阻塞尝试（对应 sem_trywait）
 *
 * 返回值:
 *   -  0 : 操作成功
 *   - -1 : 操作失败（参数非法或系统调用失败）
 *
 * 注意事项:
 *   - 若 __op 不是支持的类型，则立即返回错误；
 *   - __val 字段为逻辑值，非线程安全，仅用于调试或状态参考；
 *   - 若要获取真实信号量值，可调用 sem_getvalue。
 */
int __tsync_sem_wait(__tsync_sem_t *__sem ,int __op)
{
    if(__sem == NULL)
        return -1;   /* 参数非法，空指针 */

    /* 检查操作类型是否合法 */
    if(__op != __wait && __op != __trywait)
        return -1;

    int __ret = 0;
    if(__op == __wait)
    {
        /* 阻塞等待直到信号量可用 */
        __ret = sem_wait(&__sem->__obj);
        if(__ret != 0)
        {
            PRINT_ERROR(); 
            return -1;
        }
    }
    else if(__op == __trywait)
    {
        /* 非阻塞尝试获取信号量 */
        __ret = sem_trywait(&__sem->__obj);
        if(__ret != 0)
        {
            //PRINT_ERROR(); 
            return -1;
        }
    }
    /* 成功获取信号量后，逻辑上递减 __val */
    __sem->__val--;
    return 0;
}

/*
 * 函数名: __tsync_sem_timedwait
 * 功  能: 封装 sem_timedwait，支持带超时的信号量等待操作
 *
 * 参  数:
 *   - __sem  : 指向自定义信号量结构体的指针，不能为空
 *   - __time : 最大等待时间（单位：秒），为相对超时时间
 *
 * 返回值:
 *   -  0 : 成功获取信号量
 *   - -1 : 操作失败（参数非法、超时或系统调用错误）
 *
 * 注意事项:
 *   - 若 __sem 为空，则立即返回错误；
 *   - 内部使用 CLOCK_REALTIME 获取当前时间，并加上 __time 构造绝对超时时间；
 *   - 若等待超时或发生其他错误，使用 PRINT_ERROR 输出具体错误信息；
 *   - 成功获取信号量后，结构体中的 __val 会减一（仅作参考，非线程安全）；
 *   - 若需要毫秒/纳秒级超时控制，可拓展使用 timespec 结构设置更精细的时间。
 */
int __tsync_sem_timedwait(__tsync_sem_t *__sem ,int __time)
{
    if(__sem == NULL)
        return -1;   /* 参数非法，空指针 */

    struct timespec __ts;
    /* 获取当前绝对时间 */
    clock_gettime(CLOCK_REALTIME, &__ts);
    /* 构造未来 __time 秒后的超时时间 */
    __ts.tv_sec += __time;

    /* 带超时的等待，直到信号量可用或超时发生 */
    int __ret = sem_timedwait(&__sem->__obj ,&__ts);
    if(__ret != 0)
    {
        //PRINT_ERROR();  
        return -1;
    }

    return 0;
}

/*
 * 函数名: __tsync_sem_post
 * 功  能: 释放（增加）信号量，唤醒等待的线程或进程
 *
 * 参  数:
 *   - __sem : 指向自定义信号量结构体的指针，不能为空
 *
 * 返回值:
 *   -  0 : 成功释放信号量
 *   - -1 : 操作失败（参数非法或系统调用失败）
 *
 * 注意事项:
 *   - 若 __sem 为空，立即返回错误；
 *   - 调用底层 POSIX sem_post 增加信号量计数；
 *   - 成功后，结构体中的 __val 逻辑值递增（非线程安全，仅作参考）；
 */
int __tsync_sem_post(__tsync_sem_t *__sem)
{
    if(__sem == NULL)
        return -1;   /* 参数非法，空指针 */

    int __ret = sem_post(&__sem->__obj);
    if(__ret != 0)
    {
        PRINT_ERROR(); 
        return -1;
    }

    /* 逻辑上递增信号量计数 */
    __sem->__val++;
    return 0;
}

/*
 * 函数名: __tsync_sem_getvalue
 * 功  能: 获取信号量当前的计数值，并保存到结构体成员中
 *
 * 参  数:
 *   - __sem : 指向自定义信号量结构体的指针，不能为空
 *
 * 返回值:
 *   -  0 : 成功获取信号量值
 *   - -1 : 操作失败（参数非法或系统调用失败）
 *
 * 注意事项:
 *   - 若 __sem 为空，立即返回错误；
 *   - 调用 POSIX sem_getvalue 获取信号量当前值；
 *   - 获取的值保存至结构体的 __val 成员，供外部查询使用；
 *   - 该值可能随其他线程操作而变化，非原子保证，仅供参考。
 */
int __tsync_sem_getvalue(__tsync_sem_t *__sem)
{
    if(__sem == NULL)
        return -1;   /* 参数非法，空指针 */

    int __ret = sem_getvalue(&__sem->__obj ,&__sem->__val);
    if(__ret != 0)
    {
        PRINT_ERROR(); 
        return -1;
    }
    return 0;
}

/**
 * @function __tsync_sem_init
 * @brief 初始化自定义同步信号量结构体及其属性
 *
 * @param __sem      指向自定义信号量结构体指针，不能为空
 * @param __pshared  信号量共享标志，0 表示线程间共享，非0 表示进程间共享
 * @param __val      信号量初始值
 * @param __num      信号量编号或标识，用于内部管理
 *
 * @retval 0        初始化成功
 * @retval -1       参数非法或系统调用失败
 *
 * @note
 * - 调用前应确保 __sem 不为空；
 * - 通过 sem_init 初始化底层 POSIX 信号量；
 * - 初始化失败时，打印错误信息并返回 -1；
 * - 成功时，保存传入参数到结构体成员中，便于后续管理；
 */
int __tsync_sem_init(__tsync_sem_t *__sem ,int __pshared ,unsigned int __val ,int __num)
{
    if(__sem == NULL)
        return -1;   
    
    /* 调用 POSIX sem_init 初始化信号量对象 */
    int __ret = sem_init(&__sem->__obj ,__pshared ,__val);
    if(__ret == -1)
    {
        PRINT_ERROR();  // 打印系统错误信息
        return -1;      
    }

    /* 保存信号量相关信息 */
    __sem->__num = __num;
    __sem->__val = __val;
    __sem->__pshared = __pshared;
    return 0;
}

/**
 * @function __tsync_sem_destroy
 * @brief 销毁自定义同步信号量结构体及释放相关资源
 *
 * @param __sem  指向自定义信号量结构体指针，不能为空
 *
 * @retval 0    成功销毁信号量
 * @retval -1   参数非法或系统调用失败
 *
 * @note
 * - 调用前应确保没有线程或进程持有该信号量，否则行为未定义；
 * - 调用 POSIX 的 sem_destroy 销毁底层信号量对象；
 * - 销毁成功后，重置结构体内相关成员，避免悬空引用；
 * - 仅销毁信号量本身，不负责释放结构体指针指向的内存；
 */
int __tsync_sem_destroy(__tsync_sem_t *__sem)
{
    if(__sem == NULL)
        return -1;   // 参数非法，空指针返回错误
    
    /* 调用 POSIX sem_destroy 销毁信号量对象 */
    int __ret = sem_destroy(&__sem->__obj);  
    if(__ret == -1)
    {
        PRINT_ERROR();  // 打印系统错误信息
        return -1;      // 销毁失败返回错误
    }

    /* 销毁成功后，重置结构体成员 */
    __sem->__num = 0;
    __sem->__val = 0;
    __sem->__pshared = 0;
    return 0;
}
