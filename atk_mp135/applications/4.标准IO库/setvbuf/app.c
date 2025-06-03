#include "file.h"

char a[1024];
int main(int argc ,char *argv[])
{
    char buf[] = "gggggggaaaa";
    _sfile_t *psf = _sfile_finit("/home/baotou/linux/atk_mp135/applications/4.staio/setvbuf/file1.c" ,"file1.c" ,"w+");
    if(_sfile_fopen(psf) == -FILE_ERROR)
        goto err;

    setvbuf(stdout , a,_IOFBF ,1024);    
    //if(_sfile_fwrite(psf ,buf ,1 ,sizeof(buf) ,0 ,SEEK_SET) == -FILE_ERROR)
        //goto err;

    fwrite(buf ,1 ,sizeof(buf) ,psf->pf);
    //_sfile_fflush(psf); // 刷新文件流的缓冲区

 #if 0
    if(psf->pf->_IO_write_ptr != NULL){
        char *wptr = psf->pf->_IO_write_ptr;
        printf("%s\n",wptr);
    }
#endif
    _sfile_fclose(psf);

    psf = NULL;
    exit(0);

err:
    _sfile_fclose(psf);
    psf = NULL;
    exit(-1);
}
