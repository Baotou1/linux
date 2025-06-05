#include "file.h"


int main(int argc ,char *argv[])
{
    _file_t *pf1 = _file_init("./file1.c");
    if(pf1 == NULL)
        goto err;

    if(_file_open(pf1 ,O_RDWR ,0) == -FILE_ERROR)
        goto err;

    if(_file_print(pf1 ,0 ,20) == -FILE_ERROR)
        goto err;

    _file_close(pf1);
    exit(0);
err:
    _file_close(pf1);
    pf1 = NULL;
    exit(-1);
}
