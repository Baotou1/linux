#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;

void __proc_exit(void)
{
    printf("ending...\n");
    PROCESS_EXIT_FLUSH(&__proc ,0);
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

    for(int i = 0 ;i < 1 ;i++)
    {
        __proc->__cproc.__pid[i] = __proc_fork();
        __proc->__cproc.__cnt++;
        switch(__proc->__cproc.__pid[i])
        {
            case -1:
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            case 0:
                PROCESS_REFRESH_INFO("NULL" ,__proc);
                __proc_setpgid(__proc->__pid ,0);
                PROCESS_REFRESH_INFO("NULL" ,__proc);
                printf("create %d son process.\n" ,__proc->__pid);
                sleep(1);/* 控制子进程结束时间,子进程睡眠时间为：0秒、1秒、2秒，然后退出 */
                PROCESS_EXIT_FAST(&__proc ,0);
            default:
                printf("enter father process.\n");
                break;
        }
    }
    
    sleep(1);//防止子进程未结束，太快进入父进程
    printf("~~~~~~~~~~~~~~\n");
    PROCESS_REFRESH_INFO("NULL" ,__proc);

    for(int i = 0 ;i < __proc->__cproc.__cnt ;i++)
    {
        __proc->__cproc.__pid[i] = __proc_wait(&__proc->__cproc.__sta); 
        if(__proc->__cproc.__pid[i] == -1){
            PROCESS_EXIT_FLUSH(&__proc ,-1);
        }
        printf("Reaped child process <%d>, exit status <%d>\n", __proc->__cproc.__pid[i], WEXITSTATUS(__proc->__cproc.__sta));
    }
}