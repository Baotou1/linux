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
    sigset_t _old_mask ,_tmp_mask ,_wait_mask;

    if(_sig_sigaction(__psig) == -1)
    {
        SIG_EXIT(__psig);
    }
    
    if(_sig_sigemptyset(__psig->__sig_set) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigemptyset(&_tmp_mask) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigaddset(__psig->__sig_set ,SIGINT) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigprocmask(SIG_BLOCK ,__psig->__sig_set ,&_old_mask) == -1)
    {
        SIG_EXIT(__psig);
    }
    
    printf("sleep\n");
    sleep(2);
    if(_sig_raise(SIGINT) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigpending(&_wait_mask) == -1)
    {   
        SIG_EXIT(__psig);
    } 

    switch(_sig_sigismember(&_wait_mask, SIGINT))
    {
        case -1:
            SIG_EXIT(__psig);
        case 0:
            printf("SIGINT 信号未处于等待状态\n");
            break;
        case 1:
            printf("SIGINT 信号处于等待状态\n");
            break;
    }

    if(_sig_sigsuspend(&_tmp_mask) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigprocmask(SIG_SETMASK ,&_old_mask ,NULL) == -1)
    {
        SIG_EXIT(__psig);
    }

    _sig_free(&__psig);
    exit(0);
}
