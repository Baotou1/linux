#include "applicate.h"

#define LOCK_FILE ("./main.pid")


int main(int argc, char *argv[])
{
    __tim1 = _time_get_timestamp();
    /*-- 初始化日志 --*/
    log_init();
    /*-- 初始化进程 --*/
    process_init();
    /* 初始化相关线程同步 */
    thread_sync_init();
    /*-- 初始化线程链表 --*/
    thread_init();
#if 0   
    while(1)
    {
        sleep(3);
        __tsync_sem_post(&__sem);
    }
#endif
    /*-- 退出主线程 --*/
    __thd_list_find_nd(&__proc->__pthdl, "main");
    pthread_cleanup_push(thread_exit_handler, __proc->__pthdl->__pthd->__name);
    pthread_cleanup_pop(1);
}


