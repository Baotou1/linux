#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
#define USER_SIGNAL (SIGRTMIN + 5)

void __proc_exit(void)
{
    PROCESS_EXIT_FLUSH(&__proc ,0);
}

void _fproc_sigusr_handler(int __num, siginfo_t *__info, void *__context)
{
    printf("enter father process.\n");
    printf("Signal %d received\n", __num);
    // 这里可以处理信号，或者仅仅是为了唤醒sigsuspend
}

int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    __proc = __proc_init("proc1");
    if(__proc == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    if(__proc_atexit(__proc_exit) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    switch(__proc_fork())
    {
        case -1:
            PROCESS_EXIT_FLUSH(&__proc ,-1);

        case 0:
            printf("enter son process.\n");
            PROCESS_REFRESH_INFO("NULL" ,__proc);

            __proc->__sig = _sig_init();
            if(__proc->__sig == NULL)
            {
                PROCESS_EXIT_FAST(&__proc ,-1);
            }
            __proc->__sig->__val.sival_int = 0;

            if(_sig_sigqueue(__proc->__ppid ,USER_SIGNAL ,__proc->__sig->__val) == -1)
            {
                PROCESS_EXIT_FAST(&__proc ,-1);
            }
            PROCESS_EXIT_FAST(&__proc ,0);

        default:
            printf("enter father process.\n");
            __proc->__sig = _sig_init();
            if(__proc->__sig == NULL)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }

            sigset_t _old_mask;
#if 1
            __proc->__sig->__num = USER_SIGNAL;
            __proc->__sig->__act->sa_sigaction = _fproc_sigusr_handler;
            __proc->__sig->__act->sa_flags = SA_SIGINFO;
            if(_sig_sigaction(__proc->__sig) == -1)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }
#endif
            if(_sig_sigfillset(__proc->__sig->__sig_set) == -1)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }

            if(_sig_sigdelset(__proc->__sig->__sig_set ,USER_SIGNAL) == -1)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }

            if(_sig_sigprocmask(SIG_SETMASK ,__proc->__sig->__sig_set ,&_old_mask) == -1)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }
            
            if(_sig_sigsuspend(__proc->__sig->__sig_set) == -1)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }

            if(_sig_sigprocmask(SIG_SETMASK ,&_old_mask ,NULL) == -1)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }

            PROCESS_REFRESH_INFO("NULL" ,__proc);
            break;
    }
}