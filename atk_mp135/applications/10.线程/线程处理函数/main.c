#include "applicate.h"

#define LOCK_FILE ("./main.pid")

void __proc_exit(void)
{
    LOG_PRINT("INFO", __proc, NULL, "exit %s process ,pid=%lu",
        __proc->__name,
        __proc->__pid);
    PROCESS_EXIT_FLUSH(&__proc, 0);
}

int main(int argc, char *argv[])
{
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

    /* 加入新线程 */
    __thd_t *__newthd = __thread_init("test1", new_thread_start);
    if (__newthd == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    __thd_list_add_nd(__proc->__pthdl, __newthd);
    __thread_create(__newthd);
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd->__name, "create %s thread ,tid=%lu",
        __newthd->__name,
        __newthd->__id);

#if 0
    /* 取消指定线程 */
    if(__thread_cancel(__newthd->__id) != 0)
    {
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
#endif

    sleep(1);
#if 0
    /* 阻塞等待线程 */
    void *__tret;    
    int __ret = __thread_join(__newthd->__id ,&__tret);
    if(__ret != 0)
    {
        fprintf(stderr, "pthread_join error: %s\n", strerror(__ret));
        PROCESS_EXIT_FLUSH(&__proc, -1);
    }
    printf("新线程终止, code=%ld\n", (long)__tret);
#endif
    /*-- 退出主线程 --*/
    __thd_list_find_nd(&__proc->__pthdl, "main");
    LOG_PRINT("INFO", __proc, __proc->__pthdl->__pthd, "exit %s thread ,tid=%lu",
        __proc->__pthdl->__pthd->__name,
        __proc->__pthdl->__pthd->__id);

    __thread_exit(__proc, __proc->__pthdl->__pthd, NULL);
}


