#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
#define RUN_PATH ("../execve/newapp/main")
void __proc_exit(void)
{
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

    __proc->__command = "ls";
    int __ret = __proc_system(__proc->__command);
    if(__ret == -1)
    {
        fprintf(stderr, "[ERROR]: failed to execute system command: %s\n", __proc->__command);
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    else
    {
        if(WIFEXITED(__ret) && WEXITSTATUS(__ret) == 127)
        {
            fprintf(stderr, "[ERROR]: command not found or failed to execute: %s\n", __proc->__command);
        }
    }

    if(__proc_setsid(&__proc->__sid) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    fprintf(stdout ,"%s ending...\n" ,__proc->__name);
}
