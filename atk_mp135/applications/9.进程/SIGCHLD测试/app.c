#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
#define USER_SIGNAL (SIGRTMIN + 5)
void __proc_exit(void)
{
    printf("ending...\n");
    PROCESS_EXIT_FLUSH(&__proc ,0);
}
void __proc_sa_sigaction(int __num, siginfo_t *__info, void *__context)
{
    while(1)
    {
        pid_t __ret = __proc_waitpid(-1 ,&__proc->__cproc.__sta ,WNOHANG);
        if(__ret == -1)
        {
            if(errno == ECHILD)
            {
                break;
            }
            else
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }
        }
        else if(__ret > 0)
        {
            printf("Parent process is reaping child process , cpid: %d.\n",__ret);
            
            __cproc_reset(&__proc->__cproc ,__ret);
            PRINT_CPROC_INFO(__proc);
        }
    }    
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

    __proc->__sig = _sig_init();
    if(__proc->__sig == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    if(_sig_sigemptyset(__proc->__sig->__sig_set) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    __proc->__sig->__act->sa_sigaction = __proc_sa_sigaction;
    __proc->__sig->__num = SIGCHLD;
    if(_sig_sigaction(__proc->__sig) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    for(int i = 0 ;i < 3 ;i++)
    {
        int __ret = __proc_fork(&__proc->__cproc);
        switch(__ret)
        {
            case -1:
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            case 0:
                PROCESS_REFRESH_INFO("create son process" ,__proc);
                sleep(i+1);
                PROCESS_EXIT_FAST(&__proc ,0);
            default:
                break;
        }
    }
    sleep(3);
}