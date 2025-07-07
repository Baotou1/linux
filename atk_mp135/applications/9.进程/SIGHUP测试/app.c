#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
#define RUN_PATH ("../execve/newapp/main")
void __proc_exit(void)
{
    fprintf(stdout ,"Process %d: executing exit handler\n", getpid());
}
void __sa_sigaction(int __num , siginfo_t *__info, void *__context)
{
    fprintf(stdout ,"signal.\n");
}

int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    /*-- 初始化相关参数 --*/ 
    __proc = __proc_init("proc1");
    if(__proc == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    
    if(__proc_atexit(__proc_exit) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    __proc->__sig = _sig_init();
    if(__proc->__sig == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    /*-- 屏蔽 SIGHUP --*/ 
    if(_sig_sigemptyset(__proc->__sig->__sig_set) == -1 ||
                _sig_sigaddset(__proc->__sig->__sig_set ,SIGHUP) == -1 ||
                        _sig_sigprocmask(SIG_BLOCK ,__proc->__sig->__sig_set ,NULL) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    /*-- 注册 SIGINT 的处理函数（可选）--*/ 
    __proc->__sig->__num = SIGINT;
    __proc->__sig->__act->sa_sigaction = __sa_sigaction;
    __proc->__sig->__act->sa_flags = SA_SIGINFO;  
    if(_sig_sigaction(__proc->__sig) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    for( ; ; ){
        sleep(1);
        puts("进程运行中......");
    }
    
    PROCESS_EXIT_FLUSH(&__proc ,0);
}