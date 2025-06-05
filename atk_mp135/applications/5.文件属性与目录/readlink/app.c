#include "file.h"

#define PATHNAME "./soft"
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

    char buf[20];
    if(_file_readlink(PATHNAME ,buf ,20) == -1){
        exit(-1);
    }

    printf("soft = %s\n" ,buf);
    exit(0);
}
