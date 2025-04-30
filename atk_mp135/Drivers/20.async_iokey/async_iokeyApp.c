#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <sys/ioctl.h>
#include "poll.h"
#include <signal.h>

/* 按键状态值 */
#define KEY_PRESS         (0x00)
#define KEY_RELEASE       (0x01)
#define KEY_KEEP          (0x02) /* 按键状态中间态 */

static int fd;
void _key_sighandler_t(int num){
    int data;
    read(fd ,&data ,sizeof(data));
    if(data == KEY_PRESS){
        printf("KEY_PRESS\n");
    }
    else if(data == KEY_RELEASE){
        printf("KEY_RELEASE\n");
    }
}
int main(int argc ,char *argv[])
{
    int ret;
    int flag;
    int key_sta;
    struct pollfd fds;
    
    if(argc != 2){
        printf("agrc != 2\n");
        return -1;
    }
    /* 以阻塞访问读 */
    fd = open(argv[1] ,O_RDONLY | O_NONBLOCK);
    if(fd < 0){
        printf("open %s file error\n",argv[1]);
        return -1;
    }
    printf("open %s file ok\n",argv[1]);
    
    /* 注册信号处理函数 */
    signal(SIGIO ,_key_sighandler_t);
    /* 将应用程序“进程号”告诉给“内核” */
    if(fcntl(fd , F_SETOWN ,getpid()) < 0){
        printf("failed to inform the 'kernel' of the 'process number' of the application\n");
        close(fd);
        return 0;
    }
    /* 读取当前进程状态 */
    flag = fcntl(fd ,F_GETFL);
    /* 开启当前进程异步通知功能 */
    fcntl(fd , F_SETFL ,flag | FASYNC);

    
    while(1)
    {
       sleep(5);
    }
    close(fd);
    return 0;
}

