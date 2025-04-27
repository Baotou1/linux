#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include <sys/ioctl.h>

/* 按键状态值 */
#define KEY_PRESS         (0x00)
#define KEY_RELEASE       (0x01)
#define KEY_KEEP          (0x02) /* 按键状态中间态 */

int main(int argc ,char *argv[])
{
    int fd;
    int key_sta;
    if(argc != 2){
        printf("agrc != 2\n");
        return -1;
    }
    fd = open(argv[1] ,O_RDWR);
    if(fd < 0){
        printf("open %s file error\n",argv[1]);
        return -1;
    }
    printf("open %s file ok\n",argv[1]);
    while(1)
    {
        read(fd ,&key_sta ,sizeof(int));
        if(key_sta == KEY_PRESS)
        {
            printf("KEY_PRESS\n");
        }
        if(key_sta == KEY_RELEASE)
        {
            printf("KEY_RELEASE\n");
        }
    }
    close(fd);
    return 0;
}

