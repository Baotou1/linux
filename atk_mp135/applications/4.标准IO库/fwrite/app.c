#include "file.h"


int main(int argc ,char *argv[])
{
    char buf[] = "bbcccddddddd";
    _sfile_t *psf = _sfile_finit("/home/baotou/linux/atk_mp135/applications/4.staio/fwrite/file1.c" ,"file1.c");
    if(_sfile_fopen(psf ,"w+") == -FILE_ERROR)
        goto err;


    if(_sfile_fwrite(psf ,buf ,1 ,sizeof(buf)) == -FILE_ERROR)
        goto err;

    //_sfile_fflush(psf);
    //fsync(psf->pf->_fileno);
    if(psf->pf->_IO_write_ptr != NULL){
        char *wptr = psf->pf->_IO_write_ptr;
        printf("%s\n",wptr);
    }
    _sfile_fclose(psf);
    psf = NULL;
    exit(0);

err:
    _sfile_fclose(psf);
    psf = NULL;
    exit(-1);
}