#include "./file.h"

int main(int argc, char *argv[]) {
    _file_t *pf1 = _file_init("./test_file.c");
    _file_t *pf2 = _file_init("./test_file.c");

    if (!pf1 || !pf2) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
    if (_file_open(pf2, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);

    unsigned char data[4] = {0};
    data[0] = 0x11;
    data[1] = 0x22;
    data[2] = 0x33;
    data[3] = 0x44;

    if(_file_write(pf1 ,data ,0 ,SEEK_SET ,4) == -FILE_ERROR)
        goto err;
    
    if(_file_print_u16(pf1 ,0 ,4) == -FILE_ERROR)
        goto err;

    data[0] = 0x55;
    data[1] = 0x66;
    data[2] = 0x77;
    data[3] = 0x88;
    if(_file_write(pf2 ,data ,0 ,SEEK_SET ,4) == -FILE_ERROR)
        goto err;
    
    if(_file_print_u16(pf2 ,0 ,4) == -FILE_ERROR)
        goto err;
    exit(0);
err:
    _file_close(pf2);
    _file_close(pf1);
    exit(-FILE_ERROR);
}