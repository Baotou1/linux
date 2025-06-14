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

#if 0
    if(argc > 1){
        
        if(__proc_putenv(argv[1]) == -1)
            _log_write(__proc->__name ,"error: putenv %s" ,argv[1]);
        _log_write(__proc->__name ,"putenv %s" ,argv[1]);

        if(argc > 2){
            char *__env_str = __proc_getenv(argv[2]);
            if(__env_str == NULL)
                _log_write(__proc->__name ,"error: getenv %s" ,argv[2]);
            _log_write(__proc->__name ,"getenv %s." ,argv[2]);

            printf("getenv: %s\n",__env_str);
            PROCESS_DUMP_ENV();

            printf("\nunsetenv:%s\n",argv[2]);
            if(__proc_unsetenv(argv[2]) == -1)
                _log_write(__proc->__name ,"error: unsetenv %s" ,argv[2]);
            _log_write(__proc->__name ,"unsetenv: %s." ,argv[2]);
        }
        PROCESS_DUMP_ENV();

#if 0
        if(__proc_clearenv() == -1)
            _log_write(__proc->__name ,"error: clearenv");
        _log_write(__proc->__name ,"clearenv.");

        printf("\nclearenv\n");
        PROCESS_DUMP_ENV();
#endif
    }
#else
    if(argc > 1){
        if(__proc_setenv(argv[1] ,argv[2] ,1) == -1)
            _log_write(__proc->__name ,"error: settenv %s" ,argv[1]);
        _log_write(__proc->__name ,"setenv %s" ,argv[1]);

        PROCESS_DUMP_ENV();
    }
#endif


    exit(0);
}