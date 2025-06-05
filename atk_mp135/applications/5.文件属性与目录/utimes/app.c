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
    
    struct timeval tmval_arr[2];

#if 1
    time_t cur_tim;
    time(&cur_tim);
    for(int i = 0 ;i < 2 ;i++){
        tmval_arr[i].tv_sec = cur_tim;
        tmval_arr[i].tv_usec = 0;
    }

    if(utimes(PATHNAME ,tmval_arr) == -1){
        PRINT_ERROR();
        exit(-1);
    }

#else
    if(utimes(PATHNAME ,NULL) == -1){
        PRINT_ERROR();
        exit(-1);
    }
#endif
    exit(0);
}
