#include "applicate.h"

void *new_thread_start(void *arg)
{
    __thd_t *__pthd = (__thd_t *)arg;
    if(__thread_detach(__pthd->__id) != 0)
    {

    }
    printf("新线程 start\n");
    sleep(2);
    printf("新线程 end\n");
    if(__proc == NULL)
    {
        printf("1111\n");
    }
    LOG_PRINT("INFO", __proc, __pthd, "exit %s thread ,tid=%lu",
        __pthd->__name,
        __pthd->__id);
    __thread_exit(__proc ,__pthd ,(void *)50);
}

