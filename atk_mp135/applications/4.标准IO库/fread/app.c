#include "file.h"


int main(int argc ,char *argv[])
{
    char buf[] = "ggggggg";
    _sfile_t *psf = _sfile_finit("/home/baotou/linux/atk_mp135/applications/4.staio/fseek/file1.c" ,"file1.c" ,"a+");
    if(_sfile_fopen(psf) == -FILE_ERROR)
        goto err;

    if(_sfile_fwrite(psf ,buf ,1 ,sizeof(buf) ,0 ,SEEK_SET) == -FILE_ERROR)
        goto err;

    _sfile_fflush(psf); // 刷新文件流的缓冲区
    //fsync(psf->pf->_fileno);//将文件数据同步到磁盘

    if(_sfile_fread(psf ,1 ,100 ,0 ,SEEK_SET) == -FILE_ERROR)
        goto err;

    if(_sfile_print(psf ,1 ,100 ,0 ,SEEK_SET) == -FILE_ERROR)
        goto err;

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
