#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
#define LOCK_FILE ("./main.pid")

void __proc_exit(void)
{
    fprintf(stdout ,"Process %d: 进程结束.\n", getpid());
}

int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    /*-- 初始化进程 --*/ 
    __proc = __proc_init("proc1");
    if(__proc == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    
    if(__proc_atexit(__proc_exit) == -1)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    /* 初始化文件链表 */
    __proc->__pfl = __file_list_init();
    if(__proc->__pfl == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    __proc->__pfl->__pf = _file_init(LOCK_FILE);
    if(__proc->__pfl->__pf == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    /* 打开文件，并以非阻塞方式获取文件锁 */
    if(_file_open(__proc->__pfl->__pf ,O_WRONLY | O_CREAT ,0666) == -FILE_ERROR
                    || _file_flock(__proc->__pfl->__pf ,LOCK_EX | LOCK_NB) == -FILE_ERROR)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    fprintf(stdout ,"程序正在运行.\n");
    PROCESS_REFRESH_INFO("NULL" ,__proc);
    char __str[20] = {0};
    sprintf(__str ,"%d" ,__proc->__pid);
    if(_file_write(__proc->__pfl->__pf ,__str ,0 ,SEEK_END ,strlen(__str)) == -FILE_ERROR)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    while(1)
    {
        sleep(1);
    }
    PROCESS_EXIT_FLUSH(&__proc ,0);
}