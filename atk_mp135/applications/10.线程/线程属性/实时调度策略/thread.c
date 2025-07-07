#include "thread.h"

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

    return pthread_getschedparam(__id ,__policy ,__param);
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
    strcpy(__pthd->__name ,__name);
    return __pthd;
}

/**
 * @func   __thread_create
 * @brief  创建一个新的线程，失败时打印错误信息
 *
 * @param[in,out] __pthd  自定义线程结构体指针，不能为空。
 *                        结构体中应包含线程名称、属性对象、调度策略、优先级、
 *                        入口函数、参数等信息。
 *
 * @return
 *   -  0  ：线程创建成功；
 *   - -1  ：参数非法（如 __pthd 为 NULL）；
 *   - >0  ：线程创建失败，返回 pthread_create 的错误码（如 EAGAIN、EINVAL、EPERM 等）。
 *
 * @details
 *  本函数封装了 pthread_create 的调用流程，基于线程结构体 __thd_t 管理线程信息。
 *  执行流程：
 *    1. 判断传入的线程结构体指针是否为空；
 *    2. 调用 __thread_attr_init 初始化线程属性对象（如栈大小、调度相关默认值等）；
 *    3. 根据 __op 字段判断是否启用实时调度策略相关设置：
 *       - 若 __op 为 THREAD_OP_REALTIME，则依次调用：
 *         - __thread_attr_setinheritsched 设置调度继承方式；
 *         - __thread_attr_setschedpolicy 设置调度策略；
 *         - __thread_attr_setschedparam 设置调度参数（优先级）；
 *       - 若 __op 不为 THREAD_OP_REALTIME，则跳过上述设置，使用默认属性；
 *    4. 调用 pthread_create 创建线程，线程入口函数接收结构体指针作为参数；
 *    5. 若创建失败，调用 PRINT_ERROR 宏打印错误日志；
 *    6. 返回创建结果。
 *
 * @note
 *  - __pthd->__attr 可由外部预先设置，也可在此函数中统一初始化；
 *  - 若使用调度策略/优先级设置，需确保具备足够权限（如 root）；
 *  - __pthd->__start_routine 必须是有效的线程函数指针；
 *  - 可结合 pthread_join / pthread_detach 控制线程生命周期；
 *  - 本函数假设其他属性设置接口（如 __thread_attr_init）是安全的、幂等的。
 *
 * @example
 *  __thd_t my_thread = {
 *      .__name = "worker1",
 *      .__start_routine = my_func,
 *      .__data = NULL,
 *      .__policy = SCHED_RR,
 *      .__inheritsched = PTHREAD_EXPLICIT_SCHED,
 *      .__param = { .sched_priority = 20 },
 *      .__op = THREAD_OP_REALTIME
 *  };
 *  __thread_create(&my_thread);
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
    if(__pthd->__op == THREAD_OP_REALTIME)
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

    /* 创建线程，线程入口函数接收结构体指针作为参数 */
    __ret = pthread_create(&__pthd->__id ,&__pthd->__attr ,__pthd->__start_routine ,__pthd);
    if(__ret != 0)
    {
        PRINT_ERROR();  /* 创建失败时打印错误信息 */
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
    pthread_exit(__ret);                                    
}


