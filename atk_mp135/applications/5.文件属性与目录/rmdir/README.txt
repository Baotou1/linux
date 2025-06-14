rmdir

#include <dirent.h>
#include <string.h>

/**
 * @brief 判断目录是否为空
 * 
 * @param path 目录路径
 * @return 1 表示为空，0 表示不为空，-1 表示出错
 */
int _dir_is_empty(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        PRINT_ERROR();
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 忽略当前目录 "." 和父目录 ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            return 0;  // 目录不为空
        }
    }

    closedir(dir);
    return 1;  // 目录为空
}
