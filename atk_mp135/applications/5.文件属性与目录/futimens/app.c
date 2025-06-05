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
    
    _file_t *pf = _file_init(PATHNAME);
    if(pf == NULL){
        printf("pf is NULL\n");
        exit(-1);
    }
    if(_file_open(pf ,O_RDWR ,0) == -FILE_ERROR){
        FILE_CLOSE(pf);
        exit(-1);
    }
    
    if(argc > 1){
        /* 设置访问时间和内容修改时间为当前时间 */
        if(strcmp(argv[1] , "NULL") == 0){
            printf("\ntimes == NULL.\n");
            if(futimens(pf->fd ,NULL) == -1){
                FILE_CLOSE(pf);
                exit(-1);
            }
        }/* 设置访问时间为当前时间,内存修改时间不变 */
        else if(strcmp(argv[1] ,"timespec") == 0){
            printf("\ntimes == timespec.\n");
            struct timespec timsp_arr[2];
            memset(timsp_arr, 0, sizeof(timsp_arr));
            
            if(argc ==4 && strcmp(argv[2] ,"UTIME_NOW") == 0 && strcmp(argv[3] ,"UTIME_OMIT") == 0){
                printf("1-UTIME_NOW and 2-UTIME_OMIT.\n");
                timsp_arr[0].tv_nsec = UTIME_NOW;
                timsp_arr[1].tv_nsec = UTIME_OMIT;
                if(futimens(pf->fd ,timsp_arr) == -1){
                    FILE_CLOSE(pf);
                    exit(-1);
                }
            }
            else{
                timsp_arr[0].tv_nsec = 100;
                timsp_arr[0].tv_sec = 300;
                timsp_arr[1].tv_nsec = 100;
                timsp_arr[1].tv_sec = 300;
                if(futimens(pf->fd ,timsp_arr) == -1){
                    FILE_CLOSE(pf);
                    exit(-1);
                }
            }
        }
    }
    exit(0);
}
