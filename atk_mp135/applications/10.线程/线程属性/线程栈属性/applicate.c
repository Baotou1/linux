#include "applicate.h"
double __tim1 ,__tim2;
unsigned int __loop = 1 * 3;
int __ret = 10;
__thd_once_t __thd_once[1];
static void initialize_once(void) 
{
    fprintf(stdout ,"111\n");
}
void __fun(void)
{
    fprintf(stdout ,"_fun\n");
    __thread_once(&__thd_once[0]);
}

__thd_once_t __thd_once[1] = 
{
    {
        .__once_control = PTHREAD_ONCE_INIT,
        .__init_routine = initialize_once
    }
};

void *__thread_1(void *__arg)
{
    __thd_t *__pthd = (__thd_t *)__arg;
    THREAD_REFRESH_SCHED_INFO(__pthd);
    fprintf(stdout, "thread 1 running: policy=%d, priority=%d, stack_addr=%p, stack_sz=%.2f MB\n",
        __pthd->__policy,
        __pthd->__param.sched_priority,
        __pthd->__stack_addr,
        __pthd->__stack_sz / (1024.0 * 1024.0));

    while(1)
    {
        __tsync_sem_wait(&__sem ,__wait);
        fprintf(stdout ,"thread 1\n");
        __fun();
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
    fprintf(stdout, "thread 2 running: policy=%d, priority=%d, stack_addr=%p, stack_sz=%.2f MB\n",
        __pthd->__policy,
        __pthd->__param.sched_priority,
        __pthd->__stack_addr,
        __pthd->__stack_sz / (1024.0 * 1024.0));

    while(1)
    {
        __tsync_sem_wait(&__sem ,__wait);
        fprintf(stdout ,"thread 2\n");
        __fun();
    }

    /* 弹出清理函数并执行 */
    pthread_cleanup_push(thread_exit_handler, __pthd);
    pthread_cleanup_pop(1);
    return NULL;
}

