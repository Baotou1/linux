#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"

int main(char argc ,char* argv[])
{
    int fd,ret;
    int cnt = 0;
    char* filename;
    unsigned char databuf[1];
    unsigned char readdata;
    if(argc != 3){
        return -1;
    }
    filename = argv[1];
    /* 打开文件/dev/led_pi3 */
    fd = open(filename ,O_RDWR);
    if(fd < 0){
        printf("open %s error\n",filename);
        return -1;
    }
    databuf[0] = atoi(argv[2]);
    if(databuf[0] == 0 || databuf[0] == 1){
        ret = write(fd ,databuf ,1);
        if(ret < 0){
            printf("write error\n");
            close(fd);
            return -1;
        }
    }
    else if(databuf[0] == 2){
        ret = read(fd ,&readdata ,1);
        if(ret < 0){
            printf("read error\n");
            close(fd);
            return -1;
        }
        printf("readdata = %d\n",readdata);
    }
    printf("\n");
    /* 模拟占用led */
    while(1)
    {
        printf("App Running times: %d\n",cnt);
        sleep(3);
        cnt++;
        if(cnt == 5){
            break;
        }
    }
    ret = close(fd);
    return 0;
}
