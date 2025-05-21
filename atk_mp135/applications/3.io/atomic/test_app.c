#include "./file.h"

int main(int argc, char *argv[]) {
    _file_t *pf1 = _file_init("./test_file.c");

    if (!pf1) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
  

    if(_file_pread(pf1 ,50 ,4) == -FILE_ERROR)
        exit(-FILE_ERROR);

    unsigned char data[3];
    data[0] = 'z';
    data[1] = 'q';
    data[2] = 'z';

    if(_file_pwrite(pf1 ,data ,3 ,pf1->fs) == -FILE_ERROR)
        exit(-FILE_ERROR);

    exit(0);
err:

    _file_close(pf1);
    exit(-FILE_ERROR);
}