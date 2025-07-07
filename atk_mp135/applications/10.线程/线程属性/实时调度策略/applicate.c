#include "applicate.h"
double __tim1 ,__tim2;
unsigned int __loop = 1 * 3;

void *__thread_1(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;
    /* 设置线程为分离状态 + 注册退出清理函数 */
    __thread_detach(__pthd->__id);
    pthread_cleanup_push(thread_exit_handler, __pthd->__name);

    THREAD_REFRESH_SCHED_INFO(__pthd);
    printf("thread 1 running: policy=%d, priority=%d\n", __pthd->__policy, __pthd->__param.sched_priority);

    /* 执行代码 */
    while(1)
    {
        __tsync_sem_wait(&__sem ,__wait);
        fprintf(stdout ,"thread 1\n");
    }

    /* 弹出清理函数并执行 */
    __pthd->__ret = (void *)0;
    pthread_cleanup_pop(1);
    return NULL;
}

void *__thread_2(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;
    /* 设置线程为分离状态 + 注册退出清理函数 */
    __thread_detach(__pthd->__id);
    pthread_cleanup_push(thread_exit_handler, __pthd->__name);

    THREAD_REFRESH_SCHED_INFO(__pthd);
    printf("thread 2 running: policy=%d, priority=%d\n", __pthd->__policy, __pthd->__param.sched_priority);

    while(1)
    {
        __tsync_sem_wait(&__sem ,__wait);
        fprintf(stdout ,"thread 2\n");
    }

    /* 弹出清理函数并执行 */
    __pthd->__ret = (void *)0;
    pthread_cleanup_pop(1);
    return NULL;
}

