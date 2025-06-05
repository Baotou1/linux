#include "file.h"

#define PATHNAME "./file1.c"
int main(int argc ,char *argv[])
{
    __UMASK(0003);
    __CHMOD(PATHNAME ,0774);
    int ret = 0;
    __ACCESS_MODE(PATHNAME , F_OK ,ret);
    if(ret == -1){
        printf("Error: %s file does not exist!\n", PATHNAME);
        exit(-1);
    }
    
    
    struct utimbuf utm_buf;

#if 1
    time_t cur_tim;
    time(&cur_tim);
    utm_buf.actime = cur_tim;
    utm_buf.modtime = cur_tim;

    if(utime(PATHNAME ,&utm_buf) == -1){
        PRINT_ERROR();
        exit(-1);
    }

#else
    if(utime(PATHNAME ,NULL) == -1){
        PRINT_ERROR();
        exit(-1);
    }
#endif
    exit(0);
}
