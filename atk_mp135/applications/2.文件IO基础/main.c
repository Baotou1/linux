/**
 ******************************************************************************
 * @file           : app_1.c
 * @brief          : 测试io基础
 ******************************************************************************
 * @attention
 * 作者 Author       : baotou
 * 日期 Date         : 2025-05-16
 * 版本 Version      : v1.0
 * 平台 Platform     : linux
 * 说明 Description  : 
 *   - 打开一个存在文件（例如：src_file），使用只读方式
 *   - 打开一个新建文件（例如：dest_file），使用读写方式
 *   - 新建文件权限设置：
 *                   文件所属者（U）：读、写、执行；
 *                   同组用户（G）与其他用户（O）：读；
 *   - 从src_file文件偏移头部 500 个字节位置开始读取 1Kbyte 字节数据，然后将读取出来的数据写入到dest_file文件中
 *   - 从文件开头部开始写入，1Kbyte 字节大小，操作完成之后使用close显式关闭所有文件，然后退出程序
 * 修改记录 Change Log:
 *   日期 Date        作者 Author    说明 Description
 *   ------------     ------------   -------------------------------
 *   2025-05-16       baotou        
 *
 ******************************************************************************
 */

#include "./file.h"

int main(int argc ,char *argv[])
{
    //init;
    _file_t *pf1 = _file_init("./src_file.c" ,1024);
    if(pf1 == NULL)
        return -1;
    _file_t *pf2 = _file_init("./dest_file.c" ,2048);
    if(pf2 == NULL)
        return -1;
    _file_t *pf3 = _file_init("./test_file.c" ,20);
    if(pf3 == NULL)
        return -1;

    //open pf1
    pf1->fd = _file_open(pf1 ,O_RDONLY ,0);
    if(pf1->fd == -1)
        return -1;
    
    //create pf2
    pf2->fd = _file_open(pf2 ,O_RDWR | CREAT_NEWFILE ,S_IRWXU | S_IRGRP | S_IROTH);
    if(pf2->fd == -1)
        return -1;  
    
    //create pf3
    pf3->fd = _file_open(pf3 ,O_WRONLY | CREAT_NEWFILE ,S_IRWXU | S_IRGRP | S_IROTH);
    if(pf3->fd == -1)
         return -1;  

    //read pf1
    if(_file_read(pf1 ,500 ,SEEK_CUR) == -1)
        return -1;

    //write pf2
    if(_file_write(pf1 ,pf2 ,500 ,SEEK_SET) == -1)
        return -1;
    
    //read pf2
    if(_file_read(pf2 , 0 ,SEEK_SET) == -1)
        return -1;

    //write pf3
    unsigned char *pdata = pf3->data;
    for(int i = 0 ;i < 20 ;i++)
        pdata[i] = 'a';

    if(_file_write(NULL ,pf3 ,0 ,SEEK_SET) == -1)
        return -1;

    //printf pf1 pf2
    //_file_print(pf1);
    _file_print(pf2);

    //close pf1 pf2
    _file_close(pf1);
    _file_close(pf2);
    _file_close(pf3);

    return 0;
}

