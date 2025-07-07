#include "applicate.h"

void *new_thread_start(void *arg)
{
    __thd_t *__thd = (__thd_t *)arg;

    sleep(1);
    __thread_exit(__proc ,__thd ,NULL);
}

