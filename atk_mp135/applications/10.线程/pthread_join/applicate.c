#include "applicate.h"

void *new_thread_start(void *arg)
{
    __thd_t *__pthd = (__thd_t *)arg;
    for(;;)
        sleep(1);
    LOG_PRINT("INFO", __proc, __pthd, "exit %s thread ,tid=%lu",
        __pthd->__name,
        __pthd->__id);
    __thread_exit(__proc ,__pthd ,(void *)50);
}

