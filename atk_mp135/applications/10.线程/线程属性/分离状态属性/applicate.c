#include "applicate.h"
double __tim1 ,__tim2;
unsigned int __loop = 1 * 3;
int __ret = 10;
void *__thread_1(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;
    THREAD_REFRESH_SCHED_INFO(__pthd);
    fprintf(stdout ,"thread 1 running: policy=%d, priority=%d\n", __pthd->__policy, __pthd->__param.sched_priority);

    while(0)
    {
        //__tsync_sem_wait(&__sem ,__wait);
        fprintf(stdout ,"thread 1\n");
    }

    /* 弹出清理函数并执行 */
    pthread_cleanup_push(thread_exit_handler, __pthd);
    pthread_cleanup_pop(1);
    return NULL;
}

void *__thread_2(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;
    THREAD_REFRESH_SCHED_INFO(__pthd);
    fprintf(stdout ,"thread 2 running: policy=%d, priority=%d\n", __pthd->__policy, __pthd->__param.sched_priority);

    while(0)
    {
        //__tsync_sem_wait(&__sem ,__wait);
        fprintf(stdout ,"thread 2\n");
    }

    /* 弹出清理函数并执行 */
    pthread_cleanup_push(thread_exit_handler, __pthd);
    pthread_cleanup_pop(1);
    return NULL;
}

