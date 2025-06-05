#include "file.h"

/* 绝对路径 */
//#define PATHNAME "/home/baotou/linux/atk_mp135/applications/5.文件属性与目录/utimensat/file1.c"
/* 相对路径 */
#define PATHNAME "./file1.c"
int main(int argc ,char *argv[])
{
    char *res = _file_normalize_path(PATHNAME);
    if(res == NULL)
        exit(-1);

    if(_file_set_time(res ,NULL ,AT_SYMLINK_NOFOLLOW) == -1){
        free(res);
        exit(-1);
    }

    exit(0);
}
