#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
#define RUN_PATH ("../execve/newapp/main")
void __proc_exit(void)
{
    PRINT_ERROR();
    printf("%s error...\n" ,__proc->__name);
    PROCESS_EXIT_FLUSH(&__proc ,-1);
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

    __proc->__exec.__path = RUN_PATH;
    char *arr_argv[5] = {NULL};
    char *envp[5] = {"NAME=app", "AGE=25","SEX=man", NULL};

    arr_argv[0] = RUN_PATH;
    arr_argv[1] = "hello";
    arr_argv[2] = "world";
    arr_argv[3] = NULL;

    __proc->__exec.__envp = envp;
    __proc->__exec.__argv = arr_argv;
    __proc_execv(&__proc->__exec);
}