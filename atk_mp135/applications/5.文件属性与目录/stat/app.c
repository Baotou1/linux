#include "file.h"


int main(int argc ,char *argv[])
{
    _file_t *fd1 = _file_init("file1.c");
    if(fd1 == NULL)
        goto err;

    if(_file_open(fd1 ,O_RDWR ,0) == -FILE_ERROR)
        goto err;

    if(_file_print(fd1 ,0 ,20) == -FILE_ERROR)
        goto err;

    printf("st_mode = %o\n" ,fd1->st.st_mode);
    _file_close(fd1);
    exit(0);
err:
    _file_close(fd1);
    fd1 = NULL;
    exit(-1);
}
