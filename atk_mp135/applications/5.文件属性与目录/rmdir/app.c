#include "file.h"

#define DPATHNAME "./new_dir"
int main(int argc ,char *argv[])
{
#if 1
    //__UMASK(0002);
    _dfile_mkdir(DPATHNAME ,0774);

    _dfile_t *_pdf = _dfile_init(DPATHNAME);
    if(_pdf == NULL)
        exit(-1);

    if(_dfile_open(_pdf) == -FILE_ERROR){
        FREE_DFILE(_pdf);
        exit(-1);
    }

    _dfile_close(_pdf->__dirp);
    FREE_DFILE(_pdf);
#endif
    exit(0);
}
