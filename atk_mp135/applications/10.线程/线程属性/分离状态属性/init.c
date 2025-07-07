/**
 * @file    init.c
 * @brief   程序初始化模块实现文件
 * 
 * 本文件实现了程序启动阶段的核心初始化流程，包括日志系统初始化、
 * 进程结构体初始化及线程管理初始化等。通过封装接口，确保程序在启动
 * 过程中正确配置运行环境，管理基础资源，便于后续模块调用和维护。
 * 
 * @note
 * - 依赖 applicate.h、init.h 以及日志、进程、线程相关模块；
 * - 各初始化函数均对错误进行检测，异常时安全退出或清理；
 * - 线程初始化包含主线程信息填充和两个子工作线程的创建与调度；
 * - 进程退出函数由注册回调自动调用，完成资源清理和日志输出。
 * 
 * @author  baotou
 * @date    2025-06-28
 */

#include "init.h"
#include "applicate.h"

unsigned int __count = 0;
__tsync_rwlock_t __rwlock;
__tsync_sem_t __sem;
static void process_exit_handler(void);

/**
 * @function log_init
 * @brief 初始化日志系统封装函数
 *
 * @note
 * - 本函数是对 `_log_init()` 的简化封装，便于直接调用；
 * - 若日志系统初始化失败，将直接退出程序；
 * - 通常在主函数开始处调用一次；
 */
void log_init(void)
{
    if(_log_init() == -1)
        exit(-1);
}

/**
 * @function process_init
 * @brief 初始化当前进程结构体及其基础信息
 *
 * @note
 * - 本函数依赖 `__proc_init()` 创建进程结构体；
 * - 初始化失败时会调用 `PROCESS_EXIT_FLUSH` 立即退出；
 * - 成功后会注册进程退出函数，用于资源回收；
 * - 初始化完成后会刷新进程信息并打印初始化日志；
 * - 通常在程序主函数中尽早调用；
 */
void process_init(void)
{
    /* 为进程结构体分配 */
    __proc = __proc_init("proc1");
    if (__proc == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }

    /* 注册退出清理函数 */
    __proc_atexit(process_exit_handler);

    /* 刷新进程状态信息 */
    PROCESS_REFRESH_INFO("NULL", __proc);

    /* 打印初始化日志 */
    LOG_PRINT("INFO", __proc, NULL, "init %s process ,pid=%lu",
        __proc->__name,
        __proc->__pid);
}

/*
 * 函数名: thread_sync_init
 * 功  能: 初始化线程同步相关的同步资源
 *
 * 说明:
 *   - 用于初始化线程间同步所需的资源，可能包括信号量、互斥锁、读写锁、条件变量等；
 *   - 这里示例初始化一个线程间共享的同步对象（如信号量），初始值或状态为1；
 *   - 如果初始化失败，调用 PROCESS_EXIT_FLUSH 进行错误处理和程序退出；
 *   - 函数无参数，无返回值，调用者应确保该函数在使用同步资源前调用。
 */
void thread_sync_init(void)
{
    int __pashared = 0;  /* 线程间共享标志 */

    /* 初始化同步资源，示例为信号量 */
    if(__tsync_sem_init(&__sem ,__pashared ,0 ,1))
    {
        /* 初始化失败，执行退出处理 */
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
}

/**
 * @function thread_init
 * @brief 初始化线程链表、主线程信息并创建两个工作线程
 *
 * @note
 * - 本函数在进程初始化后调用，用于设置线程结构和调度；
 * - 初始化失败时会立即退出进程，防止资源泄漏；
 * - 所有线程都会注册到线程链表中进行统一管理；
 * - 主线程信息由当前调用线程填充；
 * - 新建线程创建成功后立即调度执行。
 */
void thread_init(void)
{
    /* 初始化线程链表容器 */
    __proc->__pthdl = __thd_list_init();
    if (__proc->__pthdl == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    /* 填充主线程信息 */
    __proc->__pthdl->__pthd = __thread_init("main");
    __proc->__pthdl->__pthd->__id = __thread_getid();
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd, "init %s thread ,tid=%lu",
        __proc->__pthdl->__pthd->__name,
        __proc->__pthdl->__pthd->__id);

#if 0
    /* 创建工作线程 1 */
    __thd_t *__pthd_1= __thread_init("thd1", __thread_1 ,NULL);
    if(__pthd_1== NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __thd_list_add_nd(__proc->__pthdl, __pthd_1);
    /* 创建工作线程 2 */
    __thd_t *__pthd_2= __thread_init("thd2", __thread_2 ,NULL);
    if(__pthd_2== NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __thd_list_add_nd(__proc->__pthdl, __pthd_2);

     /* 启动线程 1 */
    __thread_create(__pthd_1);
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd->__name, "create %s thread ,tid=%lu",
        __pthd_1->__name,
        __pthd_1->__id);
    /* 启动线程 2 */
    __thread_create(__pthd_2);
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd->__name, "create %s thread ,tid=%lu",
        __pthd_2->__name,
        __pthd_2->__id);
#else
    __thd_t *__pthd_1= __thread_init("thd1");
    if(__pthd_1 == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __thd_t *__pthd_2= __thread_init("thd2");
    if(__pthd_2 == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }

    __thd_list_add_nd(__proc->__pthdl, __pthd_1);
    __thd_list_add_nd(__proc->__pthdl, __pthd_2);

    __pthd_1->__start_routine = __thread_1;
    __pthd_1->__inheritsched = PTHREAD_EXPLICIT_SCHED;
    __pthd_1->__policy = SCHED_RR;
    __pthd_1->__param.sched_priority = 50;
    __pthd_1->__op = THREAD_OP_REALTIME | THREAD_OP_DETACHED;
    
    __pthd_2->__start_routine = __thread_2;
    __pthd_2->__inheritsched = PTHREAD_EXPLICIT_SCHED;
    __pthd_2->__policy = SCHED_RR;
    __pthd_2->__param.sched_priority = 50;
    __pthd_2->__op = THREAD_OP_REALTIME | THREAD_OP_DETACHED;

    int ret1 = __thread_create(__pthd_1);
    int ret2 = __thread_create(__pthd_2);

    if(ret1 != 0)
        fprintf(stderr ,"thread 1 create failed, error=%d\n", ret1);
    if(ret2 != 0) 
        fprintf(stderr ,"thread 2 create failed, error=%d\n", ret2);
#if 0 
    ret1 = __thread_join(__pthd_1);
    if(ret1 == EINVAL) 
       fprintf(stderr ,"Thread is detached, cannot join.\n");

    ret2 = __thread_join(__pthd_2);
    if(ret2 == EINVAL) 
        fprintf(stderr ,"Thread is detached, cannot join.\n");
#endif
#endif
}

/*<< ***********************************************************************************/
/**
 * @function thread_exit_handler
 * @brief    通用线程退出处理函数
 * 
 * @param[in] __arg  线程结构体指针（__thd_t *），表示当前线程信息
 *
 * @details
 * - 该函数作为线程清理函数使用，通常通过 `pthread_cleanup_push` 注册；
 * - 调用 `__thread_exit()` 完成线程安全退出与链表移除；
 * - 建议所有线程退出前都注册此函数，确保统一退出流程和资源释放；
 * - 退出返回值固定为 (void *)50，可根据需要修改。
 */
void thread_exit_handler(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;

    /* 打印日志，记录线程退出信息 */
    LOG_PRINT("INFO", __proc, __pthd, "exit %s thread ,tid=%lu",
        __pthd->__name,
        __pthd->__id);
        
    /* 控制台打印线程清理提示 */
    fprintf(stdout, "线程清理: %s\n", __pthd->__name);

    /* 退出线程，释放链表节点，返回退出值 */
    __thread_exit(__proc, __pthd ,0);
}

/**
 * @function process_exit_handler
 * @brief 进程退出清理函数，释放资源并打印退出日志
 *
 * @note
 * - 本函数由 `__proc_atexit()` 注册，在进程退出时自动调用；
 * - 用于计算运行时间、记录退出日志并安全释放资源；
 * - 内部调用 `PROCESS_EXIT_FLUSH` 执行结构体销毁与日志同步；
 */
static void process_exit_handler(void)
{
    /*<< ***********************************************************************************
     * 打印进程退出日志，包含进程名称和 PID
     */
    LOG_PRINT("INFO", __proc, NULL, "exit %s process ,pid=%lu",
        __proc->__name,
        __proc->__pid);

    /*<< ***********************************************************************************
     * 执行进程清理，包括资源释放、日志同步等操作
     */
    /* 摧毁线程同步相关资源 */

    /* 关闭日志 */
    _log_free();

    /* 运行时间 */
    __tim2 = _time_get_timestamp();
    fprintf(stdout ,"timer = %f\n" ,__tim2 - __tim1);

    /* 释放进程所有资源 */
    PROCESS_EXIT_FLUSH(&__proc, 0);
}