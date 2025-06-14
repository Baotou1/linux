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
    //_proc_free(&__proc); 
    //_log_free();
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

    __proc->__cpid = __proc_vfork();
    if(__proc->__cpid == -1){
        _log_write(__proc->__name ,"error: creat child process failed.");
        exit(-1);
    }
    else if(__proc->__cpid == 0){
        PROCESS_SET_NAME("proc1_child" ,__proc->__name);
        PROCESS_REFRESH_INFO("init" ,__proc);
        _log_write(__proc->__name ,"0");
        _exit(0);
    }
    else{
        PROCESS_SET_NAME("proc1" ,__proc->__name);
        PROCESS_REFRESH_INFO("init" ,__proc);
        _log_write(__proc->__name ,"runningaaaaaaa");
        //wait(NULL);
        exit(0);
    }
}