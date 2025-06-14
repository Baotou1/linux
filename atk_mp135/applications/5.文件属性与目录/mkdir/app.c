#include "file.h"

#define DPATHNAME "./new_dir"
int main(int argc ,char *argv[])
{
    __UMASK(0002);
    if(_dfile_mkdir(DPATHNAME ,0774) == -1)
        exit(-1);
        
    exit(0);
}
