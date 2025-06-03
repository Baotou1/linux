#include "./file.h"

int main(int argc ,char *argvp[])
{
    _file_t *pf1 = _file_init("./test_file.c");
    if(pf1 == NULL)
        exit(-FILE_ERROR);
    
    //打开文件
    if(_file_open(pf1 ,O_RDWR ,0) == -FILE_ERROR)
        exit(-FILE_ERROR);

#define wlen 10
#define ofs 5
    _file_data_init(pf1 ,wlen);
    unsigned char data[wlen];
    for(int i = 0; i < wlen ;i++)
        data[i] = 'b';

    if(_file_write(pf1 ,data ,ofs ,SEEK_SET ,wlen) == -FILE_ERROR)
        exit(-FILE_ERROR);

    _file_print(pf1 ,100 ,0);

    _file_close(pf1);
    
    exit(0);
}