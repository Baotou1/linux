#include "applicate.h"

unsigned int __loop = 10000 * 1000;
unsigned int __count = 0;
//__sync_mutex_t __mutex = {0};
__sync_cond_t __cond = {0};
void cleanup_1(void *__arg)
{
    fprintf(stdout ,"线程清理函数:%s\n" ,(char *)__arg);
    // 不建议调用 __thread_exit()，防止重复退出或引起未定义行为
    // 清理临时栈变量/关闭文件描述符/解除锁等可以在这里做
}

void *__thread_1(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;

    /* 分离线程，防止资源泄露，变成僵尸线程 */
    __thread_detach(__pthd->__id);

    /* 注册清理函数，确保线程被取消或异常退出时能释放资源 */
    pthread_cleanup_push(cleanup_1, __pthd->__name);

    for(;;)
    {
        __sync_mutex_lock(&__cond.__mutex);
        while((*(unsigned int *)__cond.__mutex.__data) < __loop)
            __sync_cond_wait(&__cond);
        (*(unsigned int *)__cond.__mutex.__data) = 0;
        fprintf(stdout ,"1111\n");
        __sync_mutex_unlock(&__cond.__mutex);
    }
    LOG_PRINT("INFO", __proc, __pthd, "exit %s thread ,tid=%lu",
              __pthd->__name,
              __pthd->__id);

    /* 弹出清理函数并执行 */
    pthread_cleanup_pop(1);

    /* 线程退出，释放 __pthd，写日志等 */
    __thread_exit(__proc, __pthd, (void *)50);

    /* 返回值（一般不会执行到这里） */
    return NULL;
}

void cleanup_2(void *__arg)
{
    fprintf(stdout ,"线程清理函数:%s\n" ,(char *)__arg);
}

void *__thread_2(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;

    __thread_detach(__pthd->__id);

    pthread_cleanup_push(cleanup_2, __pthd->__name);

    for(;;)
    {
        __sync_mutex_lock(&__cond.__mutex);
        (*(unsigned int *)__cond.__mutex.__data)++;
        __sync_mutex_unlock(&__cond.__mutex);
        if((*(unsigned int *)__cond.__mutex.__data) == __loop)
        {
            __sync_cond_signal(&__cond);
        }
    }

    LOG_PRINT("INFO", __proc, __pthd, "exit %s thread ,tid=%lu",
              __pthd->__name,
              __pthd->__id);

    pthread_cleanup_pop(1);

    __thread_exit(__proc, __pthd, (void *)50);

    return NULL;
}

