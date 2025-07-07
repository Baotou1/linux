#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
void __proc_exit(void)
{
    printf("%s ending...\n" ,__proc->__name);
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


    for(int j = 0; j < argc; j++)
        printf("argv[%d]: %s\n", j, argv[j]);

    PROCESS_DUMP_ENV(__proc);
}
