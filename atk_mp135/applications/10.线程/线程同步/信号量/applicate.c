#include "applicate.h"
double __tim1 ,__tim2;
unsigned int __loop = 1 * 3;

/* 写加锁 */
void *__thread_1(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;
    /* 设置线程为分离状态 + 注册退出清理函数 */
    __thread_detach(__pthd->__id);
    pthread_cleanup_push(thread_exit_handler, __pthd->__name);

    /* 执行代码 */
    while(1)
    {
        sleep(2);
        __sync_sem_post(&__sem);
    }

    /* 弹出清理函数并执行 */
    __pthd->__ret = (void *)50;
    pthread_cleanup_pop(1);
    return NULL;
}

/* 读加锁 */
void *__thread_2(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;
    /* 设置线程为分离状态 + 注册退出清理函数 */
    __thread_detach(__pthd->__id);
    pthread_cleanup_push(thread_exit_handler, __pthd->__name);

    while(1)
    {
        __sync_sem_wait(&__sem ,__wait);
        fprintf(stdout ,"thread 2\n");
    }

    /* 弹出清理函数并执行 */
    __pthd->__ret = (void *)50;
    pthread_cleanup_pop(1);
    return NULL;
}

