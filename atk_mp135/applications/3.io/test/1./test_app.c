#include "./file.h"

int main(int argc, char *argv[]) {
    _file_t *pf1 = _file_init("./test_file.c");
    _file_t *pf2 = _file_init("./test_file.c");
    _file_t *pf3 = _file_init("./test_file.c");

    if (!pf1 || !pf2 || !pf3) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
    if (_file_open(pf2, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
    if (_file_open(pf3, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);

    printf("fd1: %d\nfd2: %d\nfd3: %d\n", pf1->fd, pf2->fd, pf3->fd);

    _file_close(pf3);
    _file_close(pf2);
    _file_close(pf1);

    return 0;
}