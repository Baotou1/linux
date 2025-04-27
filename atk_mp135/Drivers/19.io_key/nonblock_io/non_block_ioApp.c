#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include <sys/ioctl.h>
#include "poll.h"
/* 按键状态值 */
#define KEY_PRESS         (0x00)
#define KEY_RELEASE       (0x01)
#define KEY_KEEP          (0x02) /* 按键状态中间态 */

int main(int argc ,char *argv[])
{
    int fd;
    int ret;
    int key_sta;
    struct pollfd fds;
    
    if(argc != 2){
        printf("agrc != 2\n");
        return -1;
    }
    /* 以非阻塞访问读 */
    fd = open(argv[1] ,O_RDONLY | O_NONBLOCK);
    if(fd < 0){
        printf("open %s file error\n",argv[1]);
        return -1;
    }
    printf("open %s file ok\n",argv[1]);
    
    /* 设置文件描述符 */
    fds.fd = fd;
    /* 有数据可读（read不会阻塞）*/
    fds.events = POLLIN;
    
    while(1)
    {
        /* 轮询读，超时时间100 */
        ret = poll(&fds ,1 ,100);
        if(ret){
            /* 有事件发生，读取数据 */
            read(fd ,&key_sta ,sizeof(int));
            printf("key_sta = %d\n" ,key_sta);
            if(key_sta == KEY_PRESS)
            {
                printf("KEY_PRESS\n");
            }
            if(key_sta == KEY_RELEASE)
            {
                printf("KEY_RELEASE\n");
            }
        }
        else if(ret == 0){
            /* 超时 */
        }else{
            /* 错误 */
        }
    }
    close(fd);
    return 0;
}

