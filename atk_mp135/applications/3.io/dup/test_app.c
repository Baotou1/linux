#include "./file.h"

int main(int argc, char *argv[]) {
    _file_t *pf1 = _file_init("./test_file.c");
    _file_t *pf2 = _file_init("./test_file.c");

    if (!pf1 || !pf2) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
    if (_file_dup(pf1 ,pf2 ,CP_FILE_DUP_1 ,0) == -FILE_ERROR) exit(-FILE_ERROR);

    printf("pf1->fd = %d\n" ,pf1->fd);
    printf("pf2->fd = %d\n" ,pf2->fd);

    unsigned char data1[4] = {0};
    unsigned char data2[4] = {0};
    data1[0] = 0x11;
    data1[1] = 0x22;
    data1[2] = 0x33;
    data1[3] = 0x44;

    data2[0] = 0x55;
    data2[1] = 0x66;
    data2[2] = 0x77;
    data2[3] = 0x88;

    if(_file_write(pf1 ,data1 ,0 ,SEEK_SET ,4) == -FILE_ERROR)
        goto err;

    write(pf2->fd ,data2 ,4);

    if(_file_print_u16(pf2 ,0 ,50) == -FILE_ERROR)
        goto err;
    exit(0);
err:
    _file_close(pf2);
    _file_close(pf1);
    exit(-FILE_ERROR);
}