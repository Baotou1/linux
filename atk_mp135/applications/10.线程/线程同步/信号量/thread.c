#include "thread.h"
#include "log.h"

/**
 * @function __sync_get_mutexattr
 * @brief 获取同步结构体中互斥锁的锁类型属性
 *
 * @param __sync  指向同步锁结构体的指针，不能为空
 *
 * @return
 *  - 成功返回锁类型值，如 PTHREAD_MUTEX_NORMAL、PTHREAD_MUTEX_RECURSIVE 等
 *  - 失败返回负值（如 -1），表示获取失败或参数非法
 *
 * @note
 *  - 返回值既表示锁类型，也可能是错误码，调用时应判断返回值是否小于 0
 *  - 该函数直接调用 pthread_mutexattr_getattr 来获取属性
 */
int __sync_get_mutexattr(__sync_mutex_t *__mutex)
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
 * @function __sync_set_mutexattr
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
int __sync_set_mutexattr(__sync_mutex_t *__mutex ,int __type)
{
    /* 参数检查，防止空指针访问 */
    if(__mutex == NULL)
        return -1;

    /* 调用 pthread_mutexattr_gettype 获取锁类型 */
    return pthread_mutexattr_settype(&__mutex->__attr, __type);
}

/**
 * @function __sync_mutex_lock_op
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
 * - 调用成功后，必须调用 __sync_mutex_unlock 解锁；
 * - 阻塞模式下，若互斥锁被其他线程持有，当前线程将阻塞等待；
 * - 非阻塞模式下，若互斥锁已被占用，立即返回失败；
 */
int __sync_mutex_lock_op(__sync_mutex_t *__mutex ,int __op)
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
 * @function __sync_mutex_unlock
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
int __sync_mutex_unlock(__sync_mutex_t *__mutex)
{
    if(!__mutex)
        return -1;
    
    return pthread_mutex_unlock(&__mutex->__lock);
}

/**
 * @function __sync_mutex_init
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
 * - 成功后建议使用 __sync_mutex_destroy 释放资源；
 * - 本函数不申请结构体内存，仅初始化其成员。
 */
int __sync_mutex_init(__sync_mutex_t *__mutex, const int *__type 
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
 * @function __sync_mutex_destroy
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
int __sync_mutex_destroy(__sync_mutex_t *__mutex)
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
 * @function __sync_get_condattr
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
int __sync_get_condattr(__sync_cond_t *__cond)
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
 * @function __sync_set_condattr
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
int __sync_set_condattr(__sync_cond_t *__cond ,int __pshared)
{
    if(__cond == NULL)
        return -1;

    return pthread_condattr_setpshared(&__cond->__attr ,__pshared);
}

/**
 * @function __sync_cond_wait
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
 * - 使用时应配合 __sync_cond_signal 或 __sync_cond_broadcast。
 */
int __sync_cond_wait(__sync_cond_t *__cond)
{
    /* 参数检查 */
    if(__cond==NULL)
        return -1;

    /* 调用系统条件等待函数，阻塞等待条件变量被唤醒 */
    return pthread_cond_wait(&__cond->__obj, &__cond->__mutex.__lock);
}

/**
 * @function __sync_cond_signal
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
 * - 通常配合 __sync_cond_wait 使用；
 * - 必须在线程持有与条件变量关联的互斥锁时调用；
 * - 不保证立即唤醒线程，仅通知条件已满足。
 */
int __sync_cond_signal(__sync_cond_t *__cond)
{
    /** 参数检查 */
    if(__cond==NULL)
        return -1;

    /* 唤醒等待该条件变量的一个线程 */
    return pthread_cond_signal(&__cond->__obj);
}

/**
 * @function __sync_cond_broadcast
 * @brief 广播唤醒所有等待该条件变量的线程
 *
 * @param __cond 条件变量结构体指针，不能为空
 *
 * @retval 0      成功唤醒所有等待线程
 * @retval -1     参数非法（__cond 为 NULL）
 * @retval >0     pthread_cond_broadcast 返回的错误码
 *
 * @note
 * - 唤醒所有因 __sync_cond_wait 阻塞的线程；
 * - 一般用于资源状态大范围变化场景；
 * - 调用时需持有对应的互斥锁；
 * - 广播不保证线程立即抢占执行。
 */
int __sync_cond_broadcast(__sync_cond_t *__cond)
{
    /** 参数检查 */
    if(__cond==NULL)
        return -1;

    /* 广播唤醒所有等待该条件变量的线程 */
    return pthread_cond_broadcast(&__cond->__obj);
}

/**
 * @function __sync_cond_init
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
 * - 使用完毕请调用 __sync_cond_destroy 释放资源，防止内存/锁泄露。
 */
int __sync_cond_init(__sync_cond_t *__cond ,const int *__cond_pshared ,
    const int *__mutex_type ,void *__data ,int __num)
{
    if(__cond == NULL)
        return -1;

    /* 初始化内部互斥锁 */
    int __ret = __sync_mutex_init(&__cond->__mutex ,__mutex_type ,__data ,__num);
    if(__ret != 0)
        return __ret;

    /* 初始化条件变量属性 */
    __ret = pthread_condattr_init(&__cond->__attr);
    if(__ret != 0)
    {
        __sync_mutex_destroy(&__cond->__mutex);
        return __ret;
    }

    /* 设置条件变量属性 */
    if(__cond_pshared != NULL)
    {
        __ret = pthread_condattr_setpshared(&__cond->__attr ,*__cond_pshared);
        if(__ret != 0)
        {
            pthread_condattr_destroy(&__cond->__attr);
            __sync_mutex_destroy(&__cond->__mutex);
            return __ret;
        }
    }

    /* 初始化条件变量，注意传入属性指针（可能为 NULL） */
    __ret = pthread_cond_init(&__cond->__obj , &__cond->__attr);
    if(__ret != 0)
    {
        pthread_condattr_destroy(&__cond->__attr);
        /* 条件变量初始化失败，销毁互斥锁防止资源泄漏 */
        __sync_mutex_destroy(&__cond->__mutex);
        return __ret;
    }
    return 0;
}

/**
 * @function __sync_cond_destroy
 * @brief 销毁条件变量结构体中的资源
 *
 * @param __cond 条件变量结构体指针，不能为空
 *
 * @retval 0      成功销毁
 * @retval -1     参数非法
 * @retval >0     pthread_* 系列函数的错误码
 */
int __sync_cond_destroy(__sync_cond_t *__cond)
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
    __ret = __sync_mutex_destroy(&__cond->__mutex);
    return __ret;
}

/**
 * @function __sync_spin_lock_op
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
 * - 成功加锁后，必须调用 __sync_spin_unlock 解锁。
 */
int __sync_spin_lock_op(__sync_spin_t *__spin ,int __op)
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
 * @function __sync_spin_unlock
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
 * - 与 __sync_spin_lock / __sync_spin_trylock 成对使用；
 * - 解锁后，其他等待线程可获取该锁。
 */
int __sync_spin_unlock(__sync_spin_t *__spin)
{
    if(__spin == NULL)
        return -1;

    return pthread_spin_unlock(&__spin->__lock);
}

/**
 * @function __sync_spin_init
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
 * - 使用后应调用 __sync_spin_destroy 释放资源；
 * - 本函数不申请结构体内存，仅初始化其成员；
 * - __data 不能为 NULL，使用前需由用户分配。
 */
int __sync_spin_init(__sync_spin_t *__spin ,int __pshared ,void *__data ,int __num)
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
 * @function __sync_spin_destroy
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
int __sync_spin_destroy(__sync_spin_t *__spin)
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
 * @function __sync_get_rwlockattr
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
int __sync_get_rwlockattr(__sync_rwlock_t *__rwlock ,int *__pshared)
{
    if(__rwlock == NULL)
        return -1;

    return pthread_rwlockattr_getpshared(&__rwlock->__attr ,__pshared);
}

/**
 * @function __sync_set_rwlockattr
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
int __sync_set_rwlockattr(__sync_rwlock_t *__rwlock ,int __pshared)
{
    if(__rwlock == NULL)
        return -1;

    return pthread_rwlockattr_setpshared(&__rwlock->__attr ,__pshared);
}

/**
 * @function __sync_rwlock_lock
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
 * - 调用前应先使用 `__sync_rwlock_init()` 完成锁的初始化；
 * - 解锁请调用 `pthread_rwlock_unlock(&__rwlock->__lock)`；
 * - 建议使用 `enum __rwlock_op` 类型（wrlock/rdlock）代替魔法数字传参。
 */
int __sync_rwlock_lock(__sync_rwlock_t *__rwlock ,int __op)
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
 * @function __sync_rwlock_trylock
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
 * - 初始化读写锁请使用 `__sync_rwlock_init()`；
 * - 解锁请使用 `pthread_rwlock_unlock(&__rwlock->__lock)`。
 */
int __sync_rwlock_trylock(__sync_rwlock_t *__rwlock ,int __op)
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
 * @function __sync_rwlock_unlock
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
int __sync_rwlock_unlock(__sync_rwlock_t *__rwlock)
{
    if(__rwlock == NULL)
        return -1;

    return pthread_rwlock_unlock(&__rwlock->__lock);
}

/**
 * @function __sync_rwlock_init
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
int __sync_rwlock_init(__sync_rwlock_t *__rwlock ,int *__pshared ,void *__data ,int __num)
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
 * @function __sync_rwlock_destroy
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
int __sync_rwlock_destroy(__sync_rwlock_t *__rwlock)
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
 * 函数名: __sync_sem_wait
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
int __sync_sem_wait(__sync_sem_t *__sem ,int __op)
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
 * 函数名: __sync_sem_timedwait
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
int __sync_sem_timedwait(__sync_sem_t *__sem ,int __time)
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
 * 函数名: __sync_sem_post
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
int __sync_sem_post(__sync_sem_t *__sem)
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
 * 函数名: __sync_sem_getvalue
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
int __sync_sem_getvalue(__sync_sem_t *__sem)
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
 * @function __sync_sem_init
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
int __sync_sem_init(__sync_sem_t *__sem ,int __pshared ,unsigned int __val ,int __num)
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
 * @function __sync_sem_destroy
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
int __sync_sem_destroy(__sync_sem_t *__sem)
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

/*
 * 函数名: __thread_attr_init
 * 功  能: 初始化线程属性对象
 *
 * 参  数:
 *   - __attr : 指向 pthread_attr_t 类型的线程属性对象指针，不能为空
 *
 * 返回值:
 *   -  0    : 初始化成功
 *   - >0    : 初始化失败，返回 pthread_attr_init 的错误码
 *
 * 注意事项:
 *   - 调用该函数后可设置线程属性（如栈大小、分离状态等）；
 *   - 使用完属性对象后，应调用 pthread_attr_destroy 释放资源；
 *   - 若 __attr 非法或系统资源不足，可能返回错误；
 *   - 错误码未在此函数中处理，请在调用处根据需要记录或输出日志。
 */
int __thread_attr_init(pthread_attr_t *__attr)
{
    int __ret = pthread_attr_init(__attr);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
    }
    return __ret; 
}

/*
 * 函数名: __thread_attr_destroy
 * 功  能: 销毁线程属性对象，释放其占用的系统资源
 *
 * 参  数:
 *   - __attr : 指向 pthread_attr_t 类型的线程属性对象指针，不能为空
 *
 * 返回值:
 *   -  0    : 成功销毁线程属性对象
 *   - -1    : 参数非法（__attr 为 NULL）
 *   - >0    : 销毁失败，返回 pthread_attr_destroy 的错误码
 *
 * 注意事项:
 *   - 线程属性对象用于 pthread_create 之前配置线程行为（如栈大小、调度策略等）；
 *   - 创建线程后，若不再使用该属性对象，应及时销毁以释放资源；
 *   - 调用本函数前，必须确保属性对象已由 pthread_attr_init 成功初始化；
 *   - 本函数不影响已创建线程，仅释放属性结构体的系统资源。
 */
int __thread_attr_destroy(pthread_attr_t *__attr)
{
    if(__attr == NULL)
        return -1;

    int __ret = pthread_attr_destroy(__attr);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
    }
    return __ret; 
}

/**
 * @func   __thread_getid
 * @brief  获取当前线程的线程 ID
 *
 * @return 当前线程的线程 ID（pthread_t 类型）
 *
 * @details
 *  该函数调用 pthread_self() 获取当前线程的唯一标识符，并返回该值。
 *  可用于调试、多线程日志标识、线程管理等场景。
 *
 * @note
 *  - pthread_t 类型的值不能直接用 printf 打印为整数，通常需通过 pthread_equal 比较；
 */
pthread_t __thread_getid(void)
{
    return pthread_self();
}

/**
 * @func    __thread_join
 * @brief   等待指定线程结束并获取其返回值
 *
 * @param   __id     线程 ID（pthread_t 类型），表示要等待的目标线程。
 * @param   __tret   用于接收线程返回值的地址（void** 类型），不能为 NULL。
 *
 * @return
 *  - 成功返回 0；
 *  - 参数 __tret 为 NULL 返回 -1；
 *  - 调用 pthread_join 失败时，返回其错误码。
 *
 * @details
 *  封装 pthread_join() 实现线程同步，调用方可通过 *__tret 获取目标线程的退出值。
 *  若 pthread_join 调用失败，将使用 PRINT_ERROR 宏输出详细错误信息。
 *
 * @note
 *  - 不应对同一个线程 ID 多次调用 pthread_join；
 *  - 若不关心返回值，也应传入一个有效的 void* 指针（如临时变量地址）；
 *  - 调用前应确保目标线程已经启动。
 */
int __thread_join(pthread_t __id ,void **__tret)
{
    int __ret = pthread_join(__id ,__tret);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
    }
    return __ret;
}

/**
 * @func    __thread_cancel
 * @brief   取消指定线程的执行
 *
 * @param   __id     线程 ID（pthread_t 类型），表示要取消的目标线程。
 *
 * @return
 *  - 成功返回 0；
 *  - 调用 pthread_cancel 失败时，返回其错误码。
 *
 * @note
 *  - 线程取消可能导致资源未释放，使用时需谨慎；
 *  - 调用前应确保目标线程已经启动。
 */
int __thread_cancel(pthread_t __id)
{
    int __ret = pthread_cancel(__id);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
    }
    return __ret;
}

/**
 * @func    __thread_detach
 * @brief   设置指定线程为分离状态，避免资源泄露
 *
 * @param   __id     线程 ID（pthread_t 类型），表示要分离的目标线程。
 *
 * @return
 *  - 成功返回 0；
 *  - 调用 pthread_detach 失败时，返回其错误码（如 EINVAL、ESRCH）。
 *
 * @details
 *  该函数封装 pthread_detach()，将线程设置为“分离状态”，
 *  系统在其退出后自动回收资源，无需通过 pthread_join 等待。
 *
 * @note
 *  - 分离线程不能再调用 pthread_join，否则行为未定义；
 *  - 应在新线程创建后尽早调用 __thread_detach；
 *  - 若线程未分离且未被 join，将会成为“僵尸线程”造成资源泄露。
 */
int __thread_detach(pthread_t __id)
{
    int __ret = pthread_detach(__id);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
    }
    return __ret;
}

/**
 * @func   __thread_init
 * @brief  初始化线程结构体对象
 *
 * @param[in]  __name           指定线程名称字符串，不能为空
 * @param[in]  __start_routine  线程入口函数指针，可为 NULL
 * @param[in]  __data           传递给线程入口函数的参数指针，可为 NULL
 *
 * @retval __thd_t* 成功返回动态分配的线程结构体指针
 * @retval NULL     失败返回 NULL（如内存分配失败或参数非法）
 *
 * @details
 *  本函数用于为一个线程结构体分配内存，并初始化其名称字段、入口函数指针和线程参数。
 *  - 若传入的名称字符串为 NULL，则直接返回 NULL。
 *  - 若内存分配失败，也将返回 NULL。
 *  - 否则，将把 __name 拷贝到结构体的 `__name` 字段中，保存入口函数和参数指针，并返回该结构体指针。
 *
 * @note
 *  返回的结构体由调用者负责释放（例如使用 `free()`），
 *  建议搭配 `__thread_free()` 等销毁函数使用。
 */
__thd_t *__thread_init(char *__name ,void *(*__start_routine)(void *) ,void *__data)
{
    /* 参数校验：名称和函数指针都不能为空 */
    if(__name == NULL)
        return NULL;

    /* 分配线程结构体内存，calloc 会将内容置零 */
    __thd_t *__pthd = (__thd_t *)calloc(1 ,sizeof(__thd_t));
    if(__pthd == NULL)
        return NULL;

    /* 传送的数据 */
    __pthd->__data = __data;

    /* 拷贝线程名称 */
    strcpy(__pthd->__name ,__name);

    /* 保存线程启动函数指针 */
    __pthd->__start_routine = __start_routine;

    return __pthd;
}

/**
 * @func   __thread_create
 * @brief  创建一个新的线程，失败时打印错误信息
 *
 * @param[in,out] __pthd         自定义线程结构体指针，不能为空。
 *                               结构体中应包含线程属性、启动函数和参数等信息。
 *
 * @return
 *   -  0  ：线程创建成功
 *   - -1  ：参数非法（__pthd 为 NULL）
 *   - >0  ：pthread_create 返回的错误码，表示线程创建失败
 *
 * @details
 *  本函数封装了 pthread_create 的调用流程，使用线程结构体 __thd_t 来组织线程相关信息。
 *  执行逻辑如下：
 *    - 首先检查参数是否为空；
 *    - 调用 __thread_attr_init 初始化线程属性对象；
 *    - 然后调用 pthread_create 创建线程；
 *    - 创建失败时，会调用 PRINT_ERROR 宏输出错误日志。
 *
 * @note
 *  - __pthd->__attr 可以为 NULL，表示使用默认线程属性；
 *  - __pthd->__start_routine 应为有效的线程入口函数；
 *  - 线程启动后将自动接收 __pthd 结构体作为参数；
 *  - 若创建成功，__pthd->__id 将保存线程 ID；
 *  - 调用成功后可配合 pthread_join 或 pthread_detach 等控制线程生命周期。
 */
int __thread_create(__thd_t *__pthd)
{
    if(__pthd == NULL)
        return -1;

    int __ret = 0;

    /* 初始化属性对象（如果有） */
    __ret = __thread_attr_init(&__pthd->__attr);
    if(__ret != 0)
    {
        return __ret;
    }

    /* 创建线程，参数传递结构体自身 */
    __ret = pthread_create(&__pthd->__id ,&__pthd->__attr ,__pthd->__start_routine ,__pthd);
    if(__ret != 0)
    {
        PRINT_ERROR();  /* 创建失败，打印错误信息 */
    }

    return __ret;
}

/**
 * @func   __thread_free
 * @brief  安全释放线程结构体指针并置空
 * 
 * @param[in,out] __pthd  指向线程结构体指针的地址（即 __thd_t **），用于释放内存并将其置为 NULL
 * 
 * @details
 *  该函数检查传入指针及其指向内容是否为 NULL，
 *  若有效则释放其占用的堆内存，并将其指向的指针置为 NULL，以防止悬空引用。
 */
void __thread_free(__thd_t **__pthd)
{
    /* 参数检查：指针本身或其内容为空则直接返回 */
    if(__pthd == NULL || (*__pthd) == NULL)
        return;

    /* 释放结构体内存 */
    free((*__pthd));

    /* 防止悬空指针 */
    (*__pthd) = NULL;
}

/**
 * @function __thread_exit
 * @brief    封装线程退出操作，带线程资源回收和日志功能
 *
 * @param[in] __proc  当前线程所属的进程结构体指针，用于访问线程链表
 * @param[in] __pthd  当前线程对应的线程结构体指针，包含线程元信息
 * @param[in] __ret   线程退出时的返回值，传递给 pthread_join 接收方
 *
 * @details
 * 本函数在线程结束前执行必要的清理工作，包括：
 *   1. 记录线程退出日志；
 *   2. 从进程结构中删除该线程对应的节点；
 *   3. 调用 pthread_exit(__ret) 正式退出线程。
 *
 * 使用说明：
 * - 推荐在线程执行逻辑结束前调用此函数；
 * - 若线程结构或进程结构为空，则不执行任何操作；
 * - __ret 用于线程返回值，可通过 pthread_join 获取；
 * - 要确保 __thd_list_delete_nd 是线程安全的，必要时需加锁。
 */
#include "process.h"
void __thread_exit(__proc_t *__proc ,__thd_t *__pthd ,void *__ret)
{
    if(__proc == NULL || __pthd == NULL)
        return;

    int __ret = 0;

    /* 销毁线程属性 */
    __ret = __thread_attr_destroy(&__pthd->__attr);
    if(__ret != 0)
        return;

    /* 删除线程节点 */
    __ret = __thd_list_delete_nd(&__proc->__pthdl ,__pthd->__name); 
    if(__ret != 0)
        return;

    /* 线程退出，返回值传给 pthread_join */
    pthread_exit(__ret);                                    
}


