#include "file.h"

#define PATHNAME "./new_dir/a"
int main(int argc ,char *argv[])
{
#if 0
    if(_file_unlink(PATHNAME) == -FILE_ERROR)
        exit(-1);
#endif
    if(_file_symlink(PATHNAME ,"softa") == -FILE_ERROR)
        exit(-1);

    char buf[100];
    if(_file_readlink("softa" ,buf ,100) == -FILE_ERROR)
        exit(-1);

    buf[20] = 'a';
    printf("%s\n",buf);
    exit(0);
}