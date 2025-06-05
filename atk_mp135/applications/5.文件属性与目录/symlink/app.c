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

    char *res = _file_normalize_path(PATHNAME);
    if(res == NULL){
        PRINT_ERROR();
        exit(-1);
    }
    
    if(_file_symlink(res ,"./soft") == -1){
        free(res);
        exit(-1);
    }

    free(res);
    exit(0);
}
