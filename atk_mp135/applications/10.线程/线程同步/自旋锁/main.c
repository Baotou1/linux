#include "applicate.h"

#define LOCK_FILE ("./main.pid")

double __tim1 ,__tim2;
void __proc_exit(void)
{
    __tim2 = _time_get_timestamp();
#ifdef MUTEX_TEST
    fprintf(stdout ,"count = %d timer = %f\n" ,*(unsigned int*)(__cond.__mutex.__data) ,(__tim2 - __tim1));
    __sync_cond_destroy(&__cond);
#else
    fprintf(stdout ,"count = %d timer = %f\n" ,*(unsigned int*)(__spin.__data) ,(__tim2 - __tim1));
    __sync_spin_destroy(&__spin);
#endif

    LOG_PRINT("INFO", __proc, NULL, "exit %s process ,pid=%lu" ,__proc->__name ,__proc->__pid);
    PROCESS_EXIT_FLUSH(&__proc, 0);
}

int main(int argc, char *argv[])
{
    __tim1 = _time_get_timestamp();
    /*-- 初始化日志 --*/
    if (_log_init() == -1)
        exit(-1);

    /*-- 初始化进程 --*/
    __proc = __proc_init("proc1");
    if (__proc == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __proc_atexit(__proc_exit);
    PROCESS_REFRESH_INFO("NULL", __proc);
    LOG_PRINT("INFO", __proc, NULL, "init %s process ,pid=%lu",
        __proc->__name,
        __proc->__pid);

    /*-- 初始化线程链表 --*/
    __proc->__pthdl = __thd_list_init();
    if (__proc->__pthdl == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __proc->__pthdl->__pthd = __thread_init("main", NULL);
    __proc->__pthdl->__pthd->__id = __thread_getid();

    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd, "init %s thread ,tid=%lu",
        __proc->__pthdl->__pthd->__name,
        __proc->__pthdl->__pthd->__id);

    /* 加入新线程1 */
    __thd_t *__pthd_1= __thread_init("thd1", __thread_1);
    if (__pthd_1== NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __thd_list_add_nd(__proc->__pthdl, __pthd_1);

    /* 加入新线程2 */
    __thd_t *__pthd_2= __thread_init("thd2", __thread_2);
    if (__pthd_2== NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __thd_list_add_nd(__proc->__pthdl, __pthd_2);

    /* 初始化相关锁 */
#ifdef MUTEX_TEST
    int __cond_type = PTHREAD_PROCESS_PRIVATE;
    int __mutex_type = PTHREAD_MUTEX_ERRORCHECK;
    __sync_cond_init(&__cond ,&__cond_type ,&__mutex_type ,&__count ,1);
#else
    int __spin_pshared = PTHREAD_PROCESS_PRIVATE;
    __sync_spin_init(&__spin ,__spin_pshared ,&__count ,1);
#endif

    /* 开始线程调度 */
    __thread_create(__pthd_1);
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd->__name, "create %s thread ,tid=%lu",
        __pthd_1->__name,
        __pthd_1->__id);
    __thread_create(__pthd_2);
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd->__name, "create %s thread ,tid=%lu",
        __pthd_2->__name,
        __pthd_2->__id);

    /*-- 退出主线程 --*/
    __thd_list_find_nd(&__proc->__pthdl, "main");
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd, "exit %s thread ,tid=%lu",
        __proc->__pthdl->__pthd->__name,
        __proc->__pthdl->__pthd->__id);

    __thread_exit(__proc, __proc->__pthdl->__pthd, NULL);
}


