#include "./file.h"

int main(int argc, char *argv[]) {
#if 0
    _file_t *pf1 = _file_init("./test_file.c");
    _file_t *pf2 = _file_init("./test_file.c");
    _file_t *pf3 = _file_init("./test_file.c");

    if (!pf1 || !pf2 || !pf3) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);
  
    if(_file_cpfd(pf1 ,pf2 ,CP_FILE_FCNTL_3 ,0) == -FILE_ERROR)
        exit(-FILE_ERROR);

    if(_file_cpfd(pf1 ,pf3 ,CP_FILE_FCNTL_3 ,100) == -FILE_ERROR)
        exit(-FILE_ERROR);    

    printf("pf1->fd = %d\n" ,pf1->fd);
    printf("pf2->fd = %d\n" ,pf2->fd);
    printf("pf3->fd = %d\n" ,pf3->fd);

    exit(0);
err:
    _file_close(pf3);
    _file_close(pf2);
    _file_close(pf1);
    exit(-FILE_ERROR);
#endif
    _file_t *pf1 = _file_init("./test_file.c");

    if (!pf1) exit(-FILE_ERROR);

    if (_file_open(pf1, O_RDWR, 0) == -FILE_ERROR) exit(-FILE_ERROR);

    if(_file_status_fcntl(pf1 ,F_GETFL) == -FILE_ERROR)
        exit(-FILE_ERROR);

    printf("pf1->fg = 0x%02X\n" ,pf1->fg);

    if(_file_status_fcntl(pf1 ,F_SETFL ,O_APPEND) == -FILE_ERROR)
        exit(-FILE_ERROR);

    printf("pf1->fg = 0x%02X\n" ,pf1->fg);

    exit(0);

err:
    _file_close(pf1);
    exit(-FILE_ERROR);
}