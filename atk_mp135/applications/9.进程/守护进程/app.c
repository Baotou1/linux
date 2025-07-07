#include "file.h"
#include "process.h"
#include "log.h"

__proc_t *__proc = NULL;
#define RUN_PATH ("../execve/newapp/main")
void __proc_exit(void)
{
    //fprintf(stdout ,"Process %d: executing exit handler\n", getpid());
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
    
    switch(__proc_fork(&__proc->__cproc))
    {
        case -1:
            PROCESS_EXIT_FLUSH(&__proc ,-1);
        case 0:
            /* 创建新会话 */
            if(__proc_setsid(&__proc->__sid) == -1)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }

            /* 设置工作目录 */
            if(_dfile_chdir("/") == -FILE_ERROR)
            {
                PROCESS_EXIT_FLUSH(&__proc ,-1);
            }

            /* 设置文件为最大权限 */
            __UMASK(0);

            /* 关闭所以文件描述符 */
            CLOSE_ALL_FDS;

            /* 将文件描述符号为 0、 1、 2 定位到/dev/null */
            int __fd = open("/dev/null" ,O_RDWR);
            dup2(__fd ,0);
            dup2(__fd ,1);
            dup2(__fd ,2);

            /* 屏蔽SIGCHLD信号 */
            signal(SIGCHLD ,SIG_IGN);

            while (1)
            {
                /* code */
                fprintf(stdout ,"111\n");
                sleep(1);
            }
            
            PROCESS_EXIT_FLUSH(&__proc ,0);
        default:
            PRINT_CPROC_INFO(__proc);
            break;
    }
}