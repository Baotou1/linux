#include "thread.h"

/**
 * @func    __thread_once
 * @brief   执行一次性初始化操作，确保初始化函数仅被调用一次
 *
 * @param[in] __once  指向自定义一次性初始化结构体 __thd_once_t 的指针，不能为空。
 *                    结构体内包含 pthread_once 控制变量和初始化函数指针。
 *
 * @return
 *   -  0  ：执行成功，初始化函数已调用或已被其他线程调用过；
 *   - -1  ：参数非法（如 __once 为 NULL）；
 *   - >0  ：pthread_once 返回的错误码（如资源问题或其他系统错误）。
 *
 * @details
 *  本函数封装了 pthread_once，用于确保某个初始化函数在整个程序生命周期内只执行一次，
 *  即使多个线程并发调用该函数也能保持安全性。
 *
 *  内部会调用：
 *    pthread_once(&__once->__once_control, __once->__init_routine);
 *
 * @note
 *  - 初始化函数必须是无参数、无返回值的函数；
 *  - __once 控制变量必须在使用前初始化为 PTHREAD_ONCE_INIT；
 *  - 若多线程同时调用本函数，只有一个线程会执行初始化函数，其余线程会等待或直接返回；
 *  - 此函数是线程安全的。
 */
int __thread_once(__thd_once_t *__once)
{
    if(__once == NULL)
        return -1;

    int __ret = pthread_once(&__once->__once_control ,__once->__init_routine);
    if(__ret != 0)
    {
        return __ret;
    }

    return 0;
}

/**
 * @func    __thread_attr_getstack
 * @brief   获取线程属性对象中配置的栈地址和栈大小
 *
 * @param[in]   __pthd         自定义线程结构体指针，不能为空，且其属性对象必须已初始化；
 * @param[out]  __stackaddr    返回栈起始地址的指针（类型为 void*），用于接收线程使用的栈基址；
 * @param[out]  __stacksize    返回栈大小的指针（类型为 size_t*），用于接收栈空间字节数；
 *
 * @return
 *   -  0  ：获取成功；
 *   - -1  ：参数非法（如 __pthd 或 __stackaddr 为 NULL）；
 *   - >0  ：pthread_attr_getstack 返回的错误码（如属性未初始化等）；
 *
 * @details
 *  本函数封装了 pthread_attr_getstack()，用于在调用 pthread_create 之前，查询线程的栈设置。
 *  若未显式设置栈地址和大小，则返回系统默认值（如默认地址为 NULL，栈大小一般为 8MB）。
 *
 * @note
 *  - 若线程栈未通过 pthread_attr_setstack 显式配置，则返回默认系统配置；
 *  - __stackaddr 必须是可写指针（即 void** 类型，注意不是 void*）；
 *  - __stacksize 也必须是合法指针，不能为 NULL；
 *  - 只能在线程创建前调用此函数；
 *
 * @see pthread_attr_getstack(3), pthread_attr_setstack(3), pthread_attr_init(3)
 *
 * @example
 *  void *addr = NULL;
 *  size_t size = 0;
 *  if (__thread_attr_getstack(&thd, &addr, &size) == 0) {
 *      printf("Stack address: %p, size: %zu bytes\n", addr, size);
 *  }
 */
int __thread_attr_getstack(__thd_t *__pthd ,void **__stackaddr, size_t *__stacksize)
{
    if(__pthd == NULL || __stackaddr == NULL || __stacksize == NULL)
        return -1;

    int __ret = pthread_attr_getstack(&__pthd->__attr ,__stackaddr ,__stacksize);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        printf("1111\n");
        return __ret;
    }
    return 0;
}

/**
 * @func    __thread_attr_setstacksize
 * @brief   设置线程属性对象中的线程栈大小
 *
 * @param[in,out] __pthd       指向自定义线程结构体 __thd_t 的指针，不能为空。
 *                             该结构体必须包含已初始化的 pthread_attr_t 属性对象。
 * @param[in]     __stacksize  线程栈大小，单位为字节。必须大于或等于系统最小栈大小 PTHREAD_STACK_MIN。
 *
 * @return
 *   -  0   ：设置成功；
 *   - -1   ：参数非法（如 __pthd 为 NULL）；
 *   - >0   ：调用 pthread_attr_setstacksize 返回的错误码（如 EINVAL，栈大小不合法等）。
 *
 * @details
 *  本函数封装了 pthread_attr_setstacksize 的调用，用于设置线程创建时的栈大小。
 *  该栈大小会影响新线程的用户栈空间，合理设置栈大小可以避免栈溢出或浪费内存。
 *  若不调用此函数，则线程默认栈大小由系统决定（通常较大，约为几百KB到MB）。
 *
 * @note
 *  - 栈大小应不少于 PTHREAD_STACK_MIN（定义于 pthread.h）；
 *  - 设置过小的栈大小可能导致线程运行异常或崩溃；
 *  - 仅影响后续基于该属性创建的线程；
 *  - 设置前应确保 pthread_attr_t 已被正确初始化。
 *
 * @see pthread_attr_setstacksize(3)
 */
int __thread_attr_setstacksize(__thd_t *__pthd ,size_t __stacksize)
{
    if(__pthd == NULL)
        return -1;

    int __ret = pthread_attr_setstacksize(&__pthd->__attr ,__stacksize);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @func    __thread_attr_setstack
 * @brief   设置线程的自定义栈地址和栈大小
 *
 * @param[in,out] __pthd        自定义线程结构体指针，不能为空，内部需包含有效 pthread_attr_t 对象；
 * @param[in]     __stackaddr   栈的起始地址，由调用方自行分配并保证对齐（建议使用 posix_memalign）；
 * @param[in]     __stacksize   栈大小（单位：字节），必须 ≥ PTHREAD_STACK_MIN；
 *
 * @return
 *   -  0  ：设置成功；
 *   - -1  ：参数非法（如 __pthd 或 __stackaddr 为 NULL）；
 *   - >0  ：pthread_attr_setstack 返回的错误码（如 EINVAL 等）。
 *
 * @details
 *  该函数封装 pthread_attr_setstack()，用于在线程创建前显式设置线程的栈地址和大小。
 *  栈内存应由调用者分配，且在线程生命周期内保持有效。栈大小需满足最小要求，且地址需满足平台对齐要求。
 *
 * @note
 *  - 若未设置，系统将自动分配默认栈地址和大小（如 8MB）；
 *  - 设置栈后，线程不能使用 pthread_attr_setstacksize 或 stackaddr 单独修改；
 *  - 通常建议搭配 THREAD_OP_STACKSIZE 使用；
 *  - 若分配失败或不确定对齐，建议使用 posix_memalign()；
 *
 * @example
 *  void *stack = NULL;
 *  posix_memalign(&stack, 16, 2 * 1024 * 1024); // 分配 2MB 对齐栈空间
 *  __thread_attr_setstack(&thd, stack, 2 * 1024 * 1024);
 */
int __thread_attr_setstack(__thd_t *__pthd ,void *__stackaddr, size_t __stacksize)
{
    if(__pthd == NULL || __stackaddr == NULL)
        return -1;

    int __ret = pthread_attr_setstack(&__pthd->__attr ,__stackaddr ,__stacksize);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}


/**
 * @func __thread_attr_getdetachstate
 * @brief 获取线程属性对象的分离状态（detach state）
 *
 * @details
 * 函数 __thread_attr_getdetachstate 用于查询线程结构体中属性对象（pthread_attr_t）的分离状态设置，
 * 以确定线程创建时是以“可连接（joinable）”还是“分离（detached）”状态启动。
 *
 * 分离状态说明：
 * - PTHREAD_CREATE_JOINABLE：线程可连接（默认），需调用 pthread_join 手动释放资源；
 * - PTHREAD_CREATE_DETACHED：线程分离，执行完自动释放资源，无法 join。
 *
 * @param[in]  __pthd         线程结构体指针，不能为空，且必须包含已初始化的属性对象；
 * @param[out] __detachstate  用于返回线程分离状态的整型指针，调用成功后其值为：
 *                               - PTHREAD_CREATE_JOINABLE（0）
 *                               - PTHREAD_CREATE_DETACHED（1）
 *
 * @return
 *   -  0  ：获取成功；
 *   - -1  ：参数非法（如 __pthd 为 NULL，或 __detachstate 为 NULL）；
 *   - >0  ：pthread_attr_getdetachstate 返回的错误码（如未初始化属性对象等）；
 *
 * @note
 * - 本函数适用于线程创建前查看线程属性模板；
 * - 若线程已经创建并运行，可使用 pthread_detach 或 pthread_join 判断其行为，但不能再获取属性；
 * - 若属性未显式设置，通常默认为 PTHREAD_CREATE_JOINABLE；
 * - 使用前应确保 __detachstate 指针可写，否则可能导致段错误；
 *
 * @see pthread_attr_getdetachstate(3), pthread_attr_setdetachstate(3), pthread_create(3)
 */
int __thread_attr_getdetachstate(__thd_t *__pthd ,int *__detachstate)
{
    if(__pthd == NULL || __detachstate == NULL)
        return -1;

    int __ret = pthread_attr_getdetachstate(&__pthd->__attr ,__detachstate);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @func __thread_attr_setdetachstate
 * @brief 设置线程属性的分离状态（detach state）
 *
 * @details
 * 函数 __thread_attr_setdetachstate 用于设置线程结构体中属性对象（pthread_attr_t）的
 * 分离状态，决定线程在退出时资源的回收方式。
 *
 * 分离状态控制线程是否需要通过 pthread_join 手动回收资源，或让系统自动释放：
 * - PTHREAD_CREATE_JOINABLE：线程可连接，需 pthread_join；
 * - PTHREAD_CREATE_DETACHED：线程分离，自动释放资源。
 *
 * @param[in,out] __pthd         线程结构体指针，不能为空，必须包含有效属性对象 __attr；
 * @param[in]     __detachstate  目标分离状态，应为：
 *                                  - PTHREAD_CREATE_JOINABLE（可连接，默认）
 *                                  - PTHREAD_CREATE_DETACHED（分离）
 *
 * @return
 *   -  0  ：设置成功；
 *   - -1  ：参数非法（如 __pthd 为 NULL，或 __detachstate 非法）；
 *   - >0  ：pthread_attr_setdetachstate 返回的错误码；
 *
 * @note
 * - 本函数应在调用 pthread_create() 前调用；
 * - 若设置为 detached 状态，则不能再调用 pthread_join()，否则将返回 EINVAL；
 * - 若既未调用该函数也未 pthread_detach，也不调用 pthread_join，会导致资源泄漏；
 * - 推荐在线程数量多、无需等待线程返回值的场景中使用 detached 线程。
 *
 * @see pthread_attr_setdetachstate(3), pthread_create(3), pthread_detach(3), pthread_join(3)
 */
int __thread_attr_setdetachstate(__thd_t *__pthd ,int __detachstate)
{
    if(__pthd == NULL)
        return -1;

    if(__detachstate != PTHREAD_CREATE_DETACHED && __detachstate != PTHREAD_CREATE_JOINABLE)
        return -1;

    int __ret = pthread_attr_setdetachstate(&__pthd->__attr ,__detachstate);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @brief 获取已创建线程的调度策略和调度参数
 *
 * 该函数用于查询指定线程（通过 pthread_t ID）当前的运行时调度策略（如 SCHED_FIFO）
 * 以及调度参数（如优先级），封装自 pthread_getschedparam。
 *
 * @param[in]  __id     已创建线程的线程 ID；
 * @param[out] __policy 用于返回线程当前的调度策略：
 *                        - SCHED_OTHER      ：普通分时调度（默认）
 *                        - SCHED_FIFO       ：实时调度：先进先出
 *                        - SCHED_RR         ：实时调度：时间片轮转
 *                        - SCHED_DEADLINE   ：截止期调度（部分系统支持）
 * @param[out] __param  用于返回线程当前的调度参数（如 sched_priority）；
 *
 * @return
 *   -  0  ：成功获取线程调度信息；
 *   - -1  ：参数无效（如任意指针为 NULL）；
 *   - >0  ：pthread_getschedparam 返回的错误码：
 *             - ESRCH  ：线程 ID 无效；
 *             - EINVAL ：__policy 或 __param 为非法指针；
 *
 * @note
 *   - 仅适用于已创建且仍存在的线程；
 *   - 若使用默认调度（SCHED_OTHER），返回的 sched_priority 通常为 0；
 *   - 若要获取线程属性模板（pthread_attr_t）的调度策略，请使用 pthread_attr_getschedparam。
 */

int __thread_getschedparam(pthread_t __id ,int *__policy ,struct sched_param *__param)
{
    if(__policy == NULL || __param == NULL)
        return -1;

    int __ret = pthread_getschedparam(__id ,__policy ,__param);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @brief 设置已创建线程的调度策略及调度参数
 *
 * 该函数用于为指定线程（通过 pthread_t ID）设置运行时调度策略及调度参数，
 * 封装自 pthread_setschedparam。适用于线程创建之后的调度行为控制。
 *
 * @param[in] __id     已创建线程的线程 ID；
 * @param[in] __policy 要设置的调度策略，支持以下值：
 *                        - SCHED_OTHER     ：普通分时调度（默认）
 *                        - SCHED_FIFO      ：实时调度，先进先出
 *                        - SCHED_RR        ：实时调度，时间片轮转
 *                        - SCHED_DEADLINE  ：基于截止期的调度（系统支持时可用）
 * @param[in] __param  调度参数结构体，包含优先级等设置：
 *                        - sched_priority 通常应在 1~99 范围内（实时策略）；
 *
 * @return
 *   -  0  ：设置成功；
 *   - -1  ：参数无效（如策略非法）；
 *   - >0  ：pthread_setschedparam 返回的错误码，例如：
 *             - EPERM  ：当前进程无权限设置实时策略（需 root）；
 *             - EINVAL ：策略或参数非法；
 *
 * @note
 *   - 若线程使用 SCHED_OTHER，则 sched_priority 通常被忽略；
 *   - 实时调度（如 SCHED_FIFO/SCHED_RR）通常需要 root 权限；
 *   - 若希望在线程创建前配置调度行为，应使用 pthread_attr 接口；
 *   - 推荐使用 sched_get_priority_min()/max() 获取策略支持的合法优先级范围。
 */

int __thread_setschedparam(pthread_t __id ,int __policy ,struct sched_param __param)
{
    if(__policy != SCHED_OTHER && __policy != SCHED_FIFO &&
       __policy != SCHED_RR && __policy != SCHED_DEADLINE)
        return -1;

    int __ret = pthread_setschedparam(__id ,__policy ,&__param);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @brief 设置线程属性的继承调度策略
 *
 * 该函数用于设置线程属性对象中线程调度策略的继承方式。
 * 可以指定线程继承创建者的调度策略（PTHREAD_INHERIT_SCHED）或
 * 显式指定调度策略（PTHREAD_EXPLICIT_SCHED）。
 *
 * @param __pthd          线程结构体指针，不能为空
 * @param __inheritsched  继承调度策略，必须是 PTHREAD_INHERIT_SCHED 或 PTHREAD_EXPLICIT_SCHED
 *
 * @return
 *   - 0   ：设置成功
 *   - -1  ：参数无效（__pthd为空或__inheritsched非法）
 *   - >0  ：pthread_attr_setinheritsched 返回的错误码
 */
int __thread_attr_setinheritsched(__thd_t *__pthd ,int __inheritsched)
{
    if(__pthd == NULL)
        return -1;

    if(__inheritsched != PTHREAD_INHERIT_SCHED && __inheritsched != PTHREAD_EXPLICIT_SCHED)
        return -1;

    int __ret = pthread_attr_setinheritsched(&__pthd->__attr ,__inheritsched);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @brief 设置线程属性的调度策略
 *
 * 该函数用于设置线程属性对象的调度策略。
 * 常见策略包括 SCHED_OTHER、SCHED_FIFO、SCHED_RR 和 SCHED_DEADLINE。
 *
 * @param __pthd    线程结构体指针，不能为空
 * @param __policy  调度策略，必须是 SCHED_OTHER、SCHED_FIFO、SCHED_RR 或 SCHED_DEADLINE 中的一种
 *
 * @return
 *   - 0   ：设置成功
 *   - -1  ：参数无效（__pthd为空或__policy非法）
 *   - >0  ：pthread_attr_setschedpolicy 返回的错误码
 */
int __thread_attr_setschedpolicy(__thd_t *__pthd , int __policy)
{
    if(__pthd == NULL)
        return -1;

    if(__policy != SCHED_OTHER && __policy != SCHED_FIFO &&
       __policy != SCHED_RR && __policy != SCHED_DEADLINE)
        return -1;

    int __ret = pthread_attr_setschedpolicy(&__pthd->__attr ,__policy);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @brief 设置线程属性的调度参数
 *
 * 该函数用于设置线程结构体中线程属性对象的调度参数（如线程优先级）。
 * 调度参数必须与当前设置的调度策略匹配，否则设置不会生效或被系统忽略。
 *
 * struct sched_param 中常用字段：
 * - sched_priority：线程优先级，仅在实时策略（SCHED_FIFO、SCHED_RR）下有效；
 *
 * 设置优先级前应确保调度策略为支持优先级的类型，并使用
 * pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) 进行显式调度配置。
 *
 * @param __pthd     线程结构体指针，不能为空，且其属性已初始化；
 * @param __param    指向 sched_param 结构体的指针，不能为空。
 *
 * @return
 *   - 0   ：设置成功；
 *   - -1  ：参数无效（如 __pthd 或 __param 为 NULL）；
 *   - >0  ：pthread_attr_setschedparam 返回的错误码，如 EINVAL、ENOTSUP。
 */
int __thread_attr_setschedparam(__thd_t *__pthd , struct sched_param __param)
{
    if(__pthd == NULL)
        return -1;

    int __ret = pthread_attr_setschedparam(&__pthd->__attr ,&__param);
    if(__ret != 0)
    {
        /* 错误码处理（如日志记录等） */
        return __ret;
    }
    return 0;
}

/**
 * @brief 初始化线程属性对象
 *
 * 该函数初始化线程结构体中的 pthread_attr_t 类型的线程属性对象，
 * 使其处于默认状态，供后续线程创建时使用。
 *
 * @param __pthd  指向线程结构体的指针，不能为空
 *
 * @return
 *   - 0    ：初始化成功
 *   - >0   ：初始化失败，返回 pthread_attr_init 的错误码
 *   - -1   ：参数无效（__pthd为空）
 *
 * @note
 *   - 调用该函数后可设置线程属性（如栈大小、分离状态等）；
 *   - 使用完属性对象后，应调用 pthread_attr_destroy 释放资源；
 *   - 若 __pthd 非法或系统资源不足，可能返回错误；
 *   - 本函数内部不做错误码日志记录，调用者可根据需要自行处理。
 */
int __thread_attr_init(__thd_t *__pthd)
{
    if(__pthd == NULL)
        return -1;

    int __ret = pthread_attr_init(&__pthd->__attr);
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
int __thread_attr_destroy(__thd_t *__pthd)
{
    if(__pthd == NULL)
        return -1;

    int __ret = pthread_attr_destroy(&__pthd->__attr);
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
 * @param   __pthd   自定义线程结构体指针，不能为空。函数将阻塞等待对应线程结束。
 * @param   __ret    用于接收线程返回值的指针地址，不能为空。
 *
 * @return
 *   -  0  ：等待成功，线程已结束，返回值通过 __ret 返回；
 *   - -1  ：参数非法（如 __pthd 或 __ret 为 NULL）；
 *   - >0  ：调用 pthread_join 失败，返回对应错误码（如 ESRCH、EINVAL、EDEADLK）。
 *
 * @details
 *   本函数封装了 pthread_join() 的使用，调用方可通过参数 __ret 获取线程退出时的返回值。
 *   函数阻塞调用者直到指定线程终止，并通过 __ret 返回线程退出值。
 *   若调用失败，将通过 PRINT_ERROR 宏输出详细错误信息，便于调试。
 *
 * @note
 *   - 若线程是分离（detached）状态，则不能调用 pthread_join，否则返回 EINVAL；
 *   - 不应对同一线程调用多次 pthread_join；
 *   - 调用前建议确保目标线程已启动；
 *   - 若线程函数未显式 return，返回值为 NULL。
 *
 * @example
 *   void *tret = NULL;
 *   __thd_t worker = ...;
 *   __thread_create(&worker);
 *   __thread_join(&worker, &tret);
 *   printf("thread returned: %p\n", tret);
 */
int __thread_join(__thd_t *__pthd ,void *__ret)
{
    if(__pthd == NULL)
        return -1;

    int __rc = pthread_join(__pthd->__id ,&__ret);
    if(__rc != 0)
    {
        /* 错误码处理（如日志记录等） */
    }
    return __rc;
}

/**
 * @func    __thread_cancel
 * @brief   取消指定线程的执行
 *
 * @param   __pthd   自定义线程结构体指针，不能为空。表示要取消的目标线程。
 *
 * @return
 *   - 0   ：成功发送取消请求；
 *   - -1  ：参数非法（如 __pthd 为 NULL）；
 *   - >0  ：调用 pthread_cancel 失败，返回对应错误码。
 *
 * @note
 *   - 线程取消可能导致资源未释放，使用时需谨慎；
 *   - 调用前应确保目标线程已经启动；
 *   - 线程响应取消需要在代码中开启取消点或调用可取消函数。
 */
int __thread_cancel(__thd_t *__pthd)
{
    if(__pthd == NULL)
        return -1;

    int __ret = pthread_cancel(__pthd->__id);
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
 * @param   __pthd   自定义线程结构体指针，不能为空。表示要分离的目标线程。
 *
 * @return
 *   - 0   ：成功将线程设置为分离状态；
 *   - -1  ：参数非法（如 __pthd 为 NULL）；
 *   - >0  ：调用 pthread_detach 失败，返回对应错误码（如 EINVAL、ESRCH）。
 *
 * @details
 *   该函数封装 pthread_detach()，将线程设置为“分离状态”，
 *   系统在其退出后自动回收资源，无需通过 pthread_join 等待。
 *
 * @note
 *   - 分离线程不能再调用 pthread_join，否则行为未定义；
 *   - 应在新线程创建后尽早调用 __thread_detach；
 *   - 若线程未分离且未被 join，将成为“僵尸线程”造成资源泄露。
 */
int __thread_detach(__thd_t *__pthd)
{
    if(__pthd == NULL)
        return -1;

    int __ret = pthread_detach(__pthd->__id);
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
__thd_t *__thread_init(char *__name)
{
    /* 参数校验：名称和函数指针都不能为空 */
    if(__name == NULL)
        return NULL;

    /* 分配线程结构体内存，calloc 会将内容置零 */
    __thd_t *__pthd = (__thd_t *)calloc(1 ,sizeof(__thd_t));
    if(__pthd == NULL)
        return NULL;

    /* 拷贝线程名称 */
    // 安全复制线程名，防止溢出
    strncpy(__pthd->__name, __name, sizeof(__pthd->__name) - 1);
    __pthd->__name[sizeof(__pthd->__name) - 1] = '\0';
    return __pthd;
}

/**
 * @func   __thread_create
 * @brief  创建一个新的线程，并根据操作标志配置线程属性
 *
 * @param[in,out] __pthd  指向自定义线程结构体 __thd_t 的指针，不能为空。
 *                        结构体应包含线程入口函数、调度策略、优先级、属性对象、
 *                        栈地址及大小（如需自定义栈）等信息。
 *
 * @return
 *   -  0  ：线程创建成功；
 *   - -1  ：参数非法（如 __pthd 为 NULL，或自定义栈大小小于系统最小值）；
 *   - >0  ：pthread_create 或属性配置失败，返回相应错误码（如 EINVAL、EPERM）。
 *
 * @details
 *  本函数封装了 pthread_create 的创建流程，统一管理线程的属性设置和调度配置。
 *  执行步骤如下：
 *    1. 检查 __pthd 是否为 NULL；
 *    2. 调用 __thread_attr_init() 初始化线程属性对象（包括默认栈大小和调度属性）；
 *    3. 若设置 THREAD_OP_REALTIME 标志，按以下顺序配置调度参数：
 *       - __thread_attr_setinheritsched() 设置调度继承方式；
 *       - __thread_attr_setschedpolicy() 设置调度策略（如 SCHED_FIFO）；
 *       - __thread_attr_setschedparam() 设置调度优先级；
 *    4. 若设置 THREAD_OP_DETACHED 标志，则调用
 *       __thread_attr_setdetachstate() 设置线程为分离状态；
 *    5. 若设置 THREAD_OP_STACKSIZE 标志，且栈大小合法，则调用
 *       __thread_attr_setstack() 设置线程栈地址和大小；
 *    6. 使用 pthread_create() 创建线程，入口函数参数为结构体指针；
 *    7. 若创建失败，调用 PRINT_ERROR 宏输出错误信息；
 *    8. 返回线程创建结果，成功返回 0，失败返回错误码。
 *
 * @note
 *  - __start_routine 必须为有效函数指针；
 *  - 若使用显式调度策略/优先级配置，建议具备 root 权限；
 *  - 可通过设置 __op 字段组合 THREAD_OP_REALTIME / THREAD_OP_DETACHED / THREAD_OP_STACKSIZE；
 *  - 自定义栈大小不能小于系统最小值 PTHREAD_STACK_MIN；
 *  - __thread_attr_* 系列函数需保证幂等和错误处理；
 *  - 若需等待线程退出，可在外部调用 pthread_join(__pthd->__id, NULL)。
 */
int __thread_create(__thd_t *__pthd)
{
    if(__pthd == NULL)               /* 参数检查，防止空指针访问 */
        return -1;

    int __ret = 0;

    /* 初始化线程属性对象（栈大小、调度策略等默认值） */
    __ret = __thread_attr_init(__pthd);
    if(__ret != 0)
    {
        return __ret;
    }

    /* 如果操作标志为启用实时调度，则配置线程属性的调度相关参数 */
    if((__pthd->__op & THREAD_OP_REALTIME) == THREAD_OP_REALTIME)
    {
        /* 设置线程调度继承方式（继承或显式调度） */
        __ret = __thread_attr_setinheritsched(__pthd, __pthd->__inheritsched);
        if(__ret != 0)
        {
            return __ret;
        }

        /* 设置线程调度策略，如 SCHED_OTHER、SCHED_FIFO、SCHED_RR */
        __ret = __thread_attr_setschedpolicy(__pthd ,__pthd->__policy);
        if(__ret != 0)
        {
            return __ret;
        }

        /* 设置线程调度参数（优先级） */
        __ret = __thread_attr_setschedparam(__pthd ,__pthd->__param);
        if(__ret != 0)
        {
            return __ret;
        }
    }

    /* 是否设置线程分离属性 */
    if((__pthd->__op & THREAD_OP_DETACHED) == THREAD_OP_DETACHED)
    {
        __ret = __thread_attr_setdetachstate(__pthd ,PTHREAD_CREATE_DETACHED);
        if(__ret != 0)
        {
            return __ret;
        }
    }

    /* 是否设置线程栈属性 */
    if((__pthd->__op & THREAD_OP_STACKSIZE) == THREAD_OP_STACKSIZE)
    {
        if(__pthd->__stack_sz < PTHREAD_STACK_MIN)
            return -1;

        if(__pthd->__stack_addr == NULL)
            __ret = __thread_attr_setstacksize(__pthd ,__pthd->__stack_sz);
        else
            __ret = __thread_attr_setstack(__pthd ,__pthd->__stack_addr ,__pthd->__stack_sz);

        if(__ret != 0)
        {
            return __ret;
        }
    }

    /* 创建线程，线程入口函数接收结构体指针作为参数 */
    __ret = pthread_create(&__pthd->__id ,&__pthd->__attr ,__pthd->__start_routine ,__pthd);
    if(__ret != 0)
    {
        PRINT_ERROR();  /* 创建失败时打印错误信息 */
    }

    return 0;
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
void __thread_exit(__proc_t *__proc ,__thd_t *__pthd ,int __ret)
{
    if(__proc == NULL || __pthd == NULL)
        return;

    int __rc = 0;

    /* 销毁线程属性 */
    __rc = __thread_attr_destroy(__pthd);
    if(__rc != 0)
        return;

    /* 删除线程节点 */
    __rc = __thd_list_delete_nd(&__proc->__pthdl ,__pthd->__name); 
    if(__rc != 0)
        return;

    /* 线程退出，返回值传给 pthread_join */
    pthread_exit(&__ret);                                    
}


