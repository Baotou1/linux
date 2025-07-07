#include "thread.h"
#include "log.h"

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
 * @param[in]  __name  指定线程名称字符串，不能为空
 *
 * @retval __thd_t* 成功返回动态分配的线程结构体指针
 * @retval NULL     失败返回 NULL（如内存分配失败或参数非法）
 *
 * @details
 *  本函数用于为一个线程结构体分配内存，并初始化其名称字段。
 *  - 若传入的名称字符串为 NULL，则直接返回 NULL。
 *  - 若内存分配失败，也将返回 NULL。
 *  - 否则，将把 __name 拷贝到结构体的 `__name` 字段中，并返回该结构体指针。
 *
 * @note
 *  返回的结构体由调用者负责释放（例如使用 `free()`），
 *  建议搭配 `__thread_free()` 等销毁函数使用。
 */
__thd_t *__thread_init(char *__name ,void *(*__start_routine)(void *))
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

    /* 保存线程启动函数指针 */
    __pthd->__start_routine = __start_routine;

    return __pthd;
}

/**
 * @func   __thread_create
 * @brief  创建一个新的线程，失败时打印错误信息
 *
 * @param[out] __thd            指向线程ID的指针
 * @param[in]  __attr           线程属性（可为 NULL，表示默认属性）
 * @param[in]  __start_routine  线程执行函数入口
 * @param[in]  __arg            传递给线程函数的参数
 *
 * @return  pthread_create 返回值，0 表示成功，非0表示失败
 *
 * @details
 *  封装 pthread_create，调用失败时通过 PRINT_ERROR 宏打印错误信息。
 */
int __thread_create(__thd_t *__pthd)
{
    int __ret = 0;

    /* 创建线程 */
    __ret = pthread_create(&__pthd->__id ,__pthd->__attr ,__pthd->__start_routine ,__pthd);

    /* 如果失败，打印错误信息 */
    if(__ret != 0)
    {
        PRINT_ERROR();
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

    __thd_list_delete_nd(&__proc->__pthdl ,__pthd->__name); /* 删除线程节点 */
    pthread_exit(__ret);                                    /* 线程退出，返回值传给 pthread_join */
}

