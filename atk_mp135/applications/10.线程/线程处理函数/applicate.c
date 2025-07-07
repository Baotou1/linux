#include "applicate.h"

void cleanup(void *__arg)
{
    fprintf(stdout ,"线程清理函数:%s\n" ,(char *)__arg);
    // 不建议调用 __thread_exit()，防止重复退出或引起未定义行为
    // 清理临时栈变量/关闭文件描述符/解除锁等可以在这里做
}

void *new_thread_start(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;

    /* 分离线程，防止资源泄露，变成僵尸线程 */
    __thread_detach(__pthd->__id);

    fprintf(stdout, "执行新线程 start\n");

    /* 注册清理函数，确保线程被取消或异常退出时能释放资源 */
    pthread_cleanup_push(cleanup, "new_thread_start");

    sleep(2);

    fprintf(stdout, "结束新线程 end\n");

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


