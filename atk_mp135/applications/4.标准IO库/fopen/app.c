#include "file.h"


int main(int argc ,char *argv[])
{
    _sfile_t *psf = _sfile_finit("/home/baotou/linux/atk_mp135/applications/4.staio/fopen/file1.c" ,"file1.c");
    if(_sfile_fopen(psf ,"r") == -FILE_ERROR)
        goto err;

    _sfile_fclose(psf);
    psf = NULL;
    return 0;
err:
    _sfile_fclose(psf);
    psf = NULL;
    return -1;
}