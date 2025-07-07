#include "applicate.h"

#define LOCK_FILE ("./main.pid")

void __proc_exit(void)
{
    PROC_INFO_LOG(__proc ,exit);
    PROCESS_EXIT_FLUSH(&__proc ,0);
}

int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    /*-- 初始化进程 --*/ 
    __proc = __proc_init("proc1");
    if(__proc == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    
    __proc_atexit(__proc_exit);             //进程退出处理函数
    PROCESS_REFRESH_INFO("NULL" ,__proc);   //刷新进程信息
    PROC_INFO_LOG(__proc ,init);            //写入日志

    /*-- 初始化线程链表 --*/
    __proc->__pthdl = __thd_list_init();
    if(__proc->__pthdl == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    } 
    __proc->__pthdl->__pthd = __thread_init("main" ,NULL);
    __proc->__pthdl->__pthd->__id = __thread_getid();
    THREAD_INFO_LOG(__proc->__pthdl->__pthd ,init);

    /* 加入新线程 */
    __thd_t *__newthd = __thread_init("test1" ,new_thread_start);
    if(__newthd == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    __thd_list_add_nd(__proc->__pthdl ,__newthd);
    __thread_create(__newthd);           // 启动新线程
    THREAD_INFO_LOG(__newthd ,add);

    /*-- 退出主线程 --*/               
    __thd_list_find_nd(&__proc->__pthdl ,"main");   // 查找主线程节点（可用于更新链表头）
    __thread_exit(__proc ,__proc->__pthdl->__pthd ,NULL); // 主线程退出
}
