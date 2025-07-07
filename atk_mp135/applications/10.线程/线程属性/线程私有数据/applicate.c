#include "applicate.h"
double __tim1 ,__tim2;
int __ret = 10;
static char *mystrerror(int errnum) ;
void __fun(int num)
{
    char *str = mystrerror(num);
    sleep(1);
    printf("主线程: str (%p) = %s\n", str, str);
}

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
        //__tsync_sem_wait(&__sem ,__wait);
        sleep(3);
        fprintf(stdout ,"thread 1:");
        __fun(1);
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
        //__tsync_sem_wait(&__sem ,__wait);
        sleep(2);
        fprintf(stdout ,"thread 2:");
        __fun(2);
    }

    /* 弹出清理函数并执行 */
    pthread_cleanup_push(thread_exit_handler, __pthd);
    pthread_cleanup_pop(1);
    return NULL;
}

#define MAX_ERROR_LEN 256
static void destructor(void *buf);
static void createkey_once(void) ;
__thd_tls_t __tls = 
{
    .__once = {
        .__once_control = PTHREAD_ONCE_INIT,
        .__init_routine = createkey_once
    },
    .__destructor = destructor
};

static void destructor(void *buf)
{
    free(buf); //释放内存
}

static void createkey_once(void) 
{
    if(__thread_key_create(&__tls) != 0)
        fprintf(stderr ,"error: createkey\n");
}

/**
 * @brief 获取错误码对应的错误描述字符串
 *
 * @param errnum 错误码
 * @return 错误描述字符串
 *
 * @note 使用了 _sys_errlist 和 _sys_nerr，它们是早期 GNU 扩展
 */
static char *mystrerror(int errnum) 
{
    char *__buf;
    int __ret;
    
    __ret = __thread_once(&__tls.__once);
    if(__ret == -1)
        return NULL;

    __buf = (char *)__thread_key_getspecific(&__tls);
    if(__buf == NULL)
    {
        __buf = (char *)calloc(1 ,MAX_ERROR_LEN);
        if(__buf == NULL)
            return NULL;

        __ret = __thread_key_setspecific(&__tls ,__buf);
        if(__ret == -1)
            return NULL;
    }

    if (errnum < 0 || errnum >= _sys_nerr || _sys_errlist[errnum] == NULL) {
        snprintf(__buf, MAX_ERROR_LEN, "Unknown error %d", errnum);
    } else {
        strncpy(__buf, _sys_errlist[errnum], MAX_ERROR_LEN - 1);
        __buf[MAX_ERROR_LEN - 1] = '\0'; // 确保以 null 结尾
    }

    return __buf;
}