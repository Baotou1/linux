#include "./file.h"

int main(int argc, char *argv[]) {
    _file_t *pf1 = _file_init("./test_file.c");
    _file_t *pf2 = _file_init("./test_file.c");

    if (!pf1 || !pf2) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
    if (_file_dup(pf1 ,pf2 ,CP_FILE_DUP_2 ,100) == -FILE_ERROR) exit(-FILE_ERROR);

    printf("pf1->fd = %d\n" ,pf1->fd);
    printf("pf2->fd = %d\n" ,pf2->fd);
    exit(0);
err:
    _file_close(pf2);
    _file_close(pf1);
    exit(-FILE_ERROR);
}