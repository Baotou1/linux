#include "file.h"
#include "process.h"
#include "log.h"

void test1(void)
{
    printf("222\n");
}

int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    __proc_t *__proc = _proc_init("proc1");
    if(__proc == NULL){
        _log_write(__proc->__name ,"error: failed to initialize process.");
        exit(-1);
    }
    _log_write(__proc->__name ,"init.");

    __proc->__function = test1;
    if(_proc_atexit(__proc->__function) == -PROC_ERROR){
        _log_write(__proc->__name ,"error: atexit registration failed.");
        exit(-1);
    }    
    _log_write(__proc->__name ,"init: atexit registration success.");

    __proc_getpid(&__proc->__pid);
    __proc_getppid(&__proc->__ppid);
    PRINT_PROC_INFO("init" ,__proc);
    
    _proc_free(&__proc); 
    _log_free();
    exit(0);
}