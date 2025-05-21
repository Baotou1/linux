#include "./file.h"

int main(int argc ,char *argvp[])
{
    _file_t *pf1 = _file_init("./test_file.c");
    if(pf1 == NULL)
        exit(-FILE_ERROR);
    
    //打开文件
    if(_file_open(pf1 ,O_RDWR | O_TRUNC ,0) == -FILE_ERROR)
        exit(-FILE_ERROR);

    _file_close(pf1);
    
    exit(0);
}