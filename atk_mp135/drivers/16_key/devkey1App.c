#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
/* 定义按键值 */
#define KEY0VALUE	0XF0
#define INVAKEY		0X00
int main(char argc ,char* argv[])
{
    int fd,ret;
    char* filename;
    unsigned char keyvalue;
    filename = argv[1];
    /* 打开文件 */
    fd = open(filename ,O_RDWR);
    if(fd < 0){
        printf("open %s error\n",filename);
        return -1;
    }
    printf("open file success\r\n");
    while(1){
        read(fd ,&keyvalue ,1);
        if(keyvalue == KEY0VALUE){
            printf("KEY1 Press, value = %#X\r\n", keyvalue);
        }
    }
    /* 关闭文件 */
    ret = close(fd);
    if(ret < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
    return 0;
}
