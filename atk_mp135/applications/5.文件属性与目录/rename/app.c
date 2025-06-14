#include "file.h"

#define OLD_PATHNAME "./new_dir"
#define NEW_PATHNAME "./newnew_dir"
int main(int argc ,char *argv[])
{
#if 1
    if(_file_rename(OLD_PATHNAME ,NEW_PATHNAME) == -FILE_ERROR)
        exit(-1);
#endif

    exit(0);
}