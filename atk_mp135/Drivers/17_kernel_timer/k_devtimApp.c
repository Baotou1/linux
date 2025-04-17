#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include <sys/ioctl.h>
#define CMD_OPEN		  (_IO(0XEF, 0x01))	/* 打开定时器 */
#define CMD_SETPERIOD	  (_IO(0XEF, 0x02))	/* 设置定时器周期命令 */
#define CMD_CLOSE		  (_IO(0XEF, 0x03))	/* 关闭定时器 */
int main(char argc ,char* argv[])
{
    int fd ,ret;
    char* filename;
    char str[100];
    unsigned int cmd;
    unsigned int arg;
    filename = argv[1];
    /* 打开文件 */
    fd = open(filename ,O_RDWR);
    if(fd < 0){
        printf("open %s error\n",filename);
        return -1;
    }
    printf("open file success\r\n");

    while(1){
        printf("input cmd:");
        ret = scanf("%d",&cmd);
        if(ret != 1){
            /* 输入错误时，防止卡死，利用fgets清除输入内容 */
            fgets(str ,sizeof(str) ,stdin);
        }
        if(cmd == 1){/*打开定时器*/
            cmd = CMD_OPEN;
        }
        else if(cmd == 2){/*修改定时器时间*/
            printf("input timer period:");
            scanf("%d",&arg);
            if(ret != 1){
                /* 输入错误时，防止卡死，利用fgets清除输入内容 */
                fgets(str ,sizeof(str) ,stdin);
            }
            cmd = CMD_SETPERIOD;
        }
        else if(cmd == 3){/*关闭定时器*/
            cmd = CMD_CLOSE;
        }
        else if(cmd == 4){/*退出循环*/
            break;
        }
        ioctl(fd ,cmd ,arg);
    }
    /* 关闭文件 */
    ret = close(fd);
    if(ret < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
    return 0;
}
