#include "file.h"

#define PATHNAME "./rm_dir"
int main(int argc ,char *argv[])
{
#if 1
    if(_file_remove(PATHNAME) == -FILE_ERROR)
        exit(-1);
#endif

    exit(0);
}