#include "process.h"


/**
 * @name  _proc_atexit
 * @brief 注册一个函数，在进程退出时调用（等价于 atexit）
 *
 * @param __fun 要在退出时调用的函数指针
 *
 * @return 成功返回 PROC_EOK，失败返回 -PROC_ERROR
 */
int _proc_atexit(void (*__fun)(void))
{
    if(__fun == NULL)
        return -PROC_ERROR;

    if(atexit(__fun) != 0)
        return -PROC_ERROR;

    return -PROC_EOK;
}
