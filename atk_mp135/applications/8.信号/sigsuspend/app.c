#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

void sig_handle(int sig)
{
    printf("11111.\n");
}
/**
 * @name    main
 * @brief   
 */
int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    __sig_t *__psig = _sig_init();
    if(__psig == NULL)
        exit(-1);

    __psig->__act->sa_handler = sig_handle;
    __psig->__num = SIGINT;
    sigset_t _old_mask ,_tmp_mask;

    /* 初始化信号处理方式 */
    if(_sig_sigaction(__psig) == -1)
    {
        SIG_EXIT(__psig);
    }
    
    /* 初始化信号集，集合无元素 */
    if(_sig_sigemptyset(__psig->__sig_set) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigemptyset(&_tmp_mask) == -1)
    {
        SIG_EXIT(__psig);
    }

    /* 添加要屏蔽的信号 */
    if(_sig_sigaddset(__psig->__sig_set ,SIGINT) == -1)
    {
        SIG_EXIT(__psig);
    }

    /* 将设置的信号集加入线程的信号集，增加屏蔽信号 */
    if(_sig_sigprocmask(SIG_BLOCK ,__psig->__sig_set ,&_old_mask) == -1)
    {
        SIG_EXIT(__psig);
    }
    
    printf("sleep\n");
    /* 挂起进程，设置临时信号集，允许SIGINT触发 */
    if(_sig_sigsuspend(&_tmp_mask) == -1)
    {
        SIG_EXIT(__psig);
    }

    /* 恢复之前的屏蔽集（保险起见） */
    if(_sig_sigprocmask(SIG_SETMASK ,&_old_mask ,NULL) == -1)
    {
        SIG_EXIT(__psig);
    }

    _sig_free(&__psig);
    exit(0);
}
