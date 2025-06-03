#include "./file.h"

int main(int argc ,char *argvp[])
{
    _file_t *pft1 = _file_init("./hole_file.c" ,1024);
    if(pft1 == NULL)
        exit(-1);
    
    //打开文件
    pft1->fd = _file_open(pft1 ,O_RDWR | CREAT_NEWFILE ,S_IRWXU | S_IRGRP | S_IROTH);    
    if(pft1->fd == -1)
        exit(pft1->fd);
#if 1
    unsigned char *data = pft1->data;
    for(int i = 0; i < 1024 ;i++)
        data[i] = 'a';
    
    int ofs = 4096;
    //在偏移到4096字节处，写入4096字节
    for(int i = 0; i < 4;i++){
        pft1->ret = _file_write(NULL ,pft1 ,ofs ,SEEK_SET);
        if(pft1->ret == -1)
            exit(pft1->ret);
        ofs += 1024;
    }
    
    pft1->ret = _file_read(pft1 ,4096 ,SEEK_SET);
    if(pft1->ret == -1)
            exit(pft1->ret);

    _file_print(pft1);
#endif
	
    _file_close(pft1);
    exit(0);
}
