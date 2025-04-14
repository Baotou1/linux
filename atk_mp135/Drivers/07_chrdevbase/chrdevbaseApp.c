#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
/***************************************************************
 * 
 * 文件名 : chrdevbaseApp.c
 * 作者 : baotou
 * 版本 : V1.0
 * 描述 : chrdevbase 驱测试 APP
 * 日志 : 初版 V1.0 2025/3/18
 *  
***************************************************************/
static char userdata[] = {"renjunlong"}; 
/*
* @description : main 主程序
* @param - argc : argv 数组元素个数不能超过3个
* @param - argv : 具体参数（0 - 文件名 ，1 - 驱动名称 ，2 - 驱动命令）
* @return : 0 成功;其他 失败
*/
int main(int argc ,char* argv[])
{
    int fd ,retvalue;/*文件描述符*/
    char* filename;
    char readbuf[100],writebuf[100];
    /*判断传入命令行参数个数是否越界*/
    if(argc != 3)
    {
        printf("User Parm Exceed Three\r\n");
        return -1;        
    }
	filename = argv[1];
    /*打开驱动文件*/
	fd = open(filename ,O_RDWR);
    if(fd < 0) 
    {
        printf("Can't Open %s File\r\n",filename);
		return -1;
    }

    /*从驱动文件读取数据*/
    if(atoi(argv[2]) == 1)
    {
		retvalue = read(fd ,readbuf ,50);
        if(retvalue == 0){
            printf("read data:%s\r\n",readbuf);
		}else{
			/*  读取成功，打印出读取成功的数据 */
			printf("read file %s failed!\r\n", filename);
		}
    }
    /*向驱动文件写入数据*/
    else if(atoi(argv[2]) == 2)
    {
        memcpy(writebuf ,userdata ,sizeof(userdata));
        retvalue = write(fd ,writebuf ,50);
        if(retvalue < 0)
        {
            printf("write file %s failed!\r\n", filename);          
        }
    }
    /*关闭驱动设备*/
    if(close(fd) < 0)
    {
        printf("Can't Close %s File\r\n",filename);
        return -1;
    }
    return 0;    
}



