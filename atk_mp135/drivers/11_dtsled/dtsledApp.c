#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
/***************************************************************
 * 文件名 : ledApp.c
 * 作者 : baotou
 * 版本 : V1.0
 * 描述 : led 驱动测试 APP
 * 日志 : 初版 V1.0 2025/3/24
***************************************************************/
int main(char argc ,char* argv[])
{
    int fd ,ret;
    char* filename;
    char opt;
    unsigned char readdata;
    if(argc != 3)
    {
        printf("User Parm Exceed Three\r\n");
        return -1;
    }
    filename = argv[1];
    fd = open(filename ,O_RDWR);
    if(fd < 0)
    {
        printf("open %s failed\r\n",filename);
        return -1;
    }   
    /* 当opt=1/2时，写入pi3，控制led */
    opt = atoi(argv[2]);
    if(opt == 0 || opt == 1)
    {
        ret = write(fd ,&opt ,sizeof(opt));
        if(ret < 0)
        {
            printf("write io failed\r\n");
            return -1;
        }
    }
    /* 当opt=2时，读取pi3 */
    else if(opt == 2)
    {
        ret = read(fd ,&readdata ,1);
        if(ret < 0)
        {
            printf("read io failed\r\n");
            return -1;
        }
        printf("read io == %d\r\n" ,readdata);
    }
    ret = close(fd);
    if(ret < 0)
    {
        printf("close %s failed\r\n",filename);
        return -1;
    }
    return 0;
}