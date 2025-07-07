#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
/**
 * @name    test1
 * @brief   程序退出时调用的清理函数（通过 atexit 注册）
 */
void test1(void)
{
    __proc_dump_env();
    _log_write(__proc->__name ,"print dump env.");
    _proc_free(&__proc); 
    _log_free();
}

/**
 * @name    main
 * @brief   
 */
int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    __proc = _proc_init("proc1");
    if(__proc == NULL){
        _log_write("unknow" ,"error: failed to initialize process.");
        exit(-1);
    }
    _log_write(__proc->__name ,"init.");

    __proc->__function = test1;
    if(_proc_atexit(__proc->__function) == -PROC_ERROR){
        _log_write(__proc->__name ,"error: atexit registration failed.");
        exit(-1);
    }    
    _log_write(__proc->__name ,"atexit registration success.");

    __proc_getpid(&__proc->__pid);
    __proc_getppid(&__proc->__ppid);
    PRINT_PROC_INFO("init" ,__proc);
   
    exit(0);
}
