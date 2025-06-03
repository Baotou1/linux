#include "./file.h"

int main(int argc, char *argv[]) {
    _file_t *pf1 = _file_init("./file1.c");
    _file_t *pf2 = _file_init("./file2.c");
    if (!pf1 || !pf2) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
    if (_file_open(pf2, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);

    if(_file_truncate(pf1 ,10 ,5 ,FILE_TRUNCATE ,"./file1.c") == -FILE_ERROR)
        exit(-FILE_ERROR);

    if(_file_truncate(pf2 ,1024 ,5 ,FILE_TRUNCATE ,"./file2.c") == -FILE_ERROR)
        exit(-FILE_ERROR);

    exit(0);
err:
    _file_close(pf1);
    exit(-FILE_ERROR);
}