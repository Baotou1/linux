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

    int __ret = __proc_fork();
    switch(__ret)
    {
        case -1:
            PROCESS_EXIT_FLUSH(&__proc ,-1);
        case 0:
            PROCESS_REFRESH_INFO("NULL" ,__proc);
            printf("create %d son process.\n" ,__proc->__pid);
            sleep(5);
            PROCESS_REFRESH_INFO("NULL1" ,__proc);
            PROCESS_EXIT_FAST(&__proc ,0);
        default:
            break;
    }
}