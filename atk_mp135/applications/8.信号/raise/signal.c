#include "signal.h"

/**
 * @function _sig_signal
 * @brief    为指定信号注册一个处理函数（信号处理器）。
 *
 * @param    __signum   要处理的信号编号（如 SIGINT、SIGTERM 等）。
 * @param    __handler  信号处理函数指针（类型为 sig_t，即 void (*)(int)）。
 *
 * @retval   原信号处理函数指针，或SIG_ERR（如果参数无效或注册失败）。
 *
 * @note
 *  - 会检查信号编号是否在有效范围（1 ~ _NSIG - 1）；
 *  - 会检查处理函数是否为 NULL；
 *  - 若注册失败，返回 NULL 并打印错误信息；
 *  - 该函数封装了标准库的 signal() 调用。
 */
sig_t _sig_signal(int __signum, sig_t __handler)
{
    if(__handler == NULL)
        return SIG_ERR;

    if(!IS_VALID_SIGNAL(__signum))
        return SIG_ERR;
 
    sig_t __sig = signal(__signum ,__handler);
    if(__sig == SIG_ERR){
        PRINT_ERROR();
        return SIG_ERR;
    }

    return __sig;
}

/**
 * @function _sig_sigaction
 * @brief    使用 sigaction 注册指定信号的处理动作。
 *
 * 封装标准系统调用 sigaction，用于设置信号处理函数。通过传入已初始化的 __sig_t 结构体，
 * 注册指定编号的信号和对应的处理行为（包含阻塞掩码、处理器等）。
 *
 * @param    __sig   指向已初始化的 __sig_t 结构体，必须包含有效的信号编号和动作指针。
 *
 * @retval   0       注册成功。
 * @retval  -1       注册失败，参数无效或 sigaction 出错。
 *
 * @note
 *  - 使用前必须调用 `_sig_init()` 初始化 __sig_t；
 *  - __sig->__num 必须是合法信号编号；
 *  - __sig->__act 不能为空；
 *  - 若需要保存旧信号处理行为，可在 __sig->__oact 中获取；
 *  - 内部调用 `PRINT_ERROR()` 输出错误信息（建议实现该宏打印 errno 或 strerror）。
 */

int _sig_sigaction(__sig_t *__sig)
{
    if(__sig == NULL || __sig->__act == NULL)
        return -1; 

    if(!IS_VALID_SIGNAL(__sig->__num))
        return -1;

    if(sigaction(__sig->__num ,__sig->__act ,__sig->__oact) == -1){
        PRINT_ERROR();
        return -1;
    }

    return 0;
}

/**
 * @function _sig_kill
 * @brief    向指定进程发送信号。
 *
 * 封装标准系统调用 kill，用于向指定进程（或进程组）发送一个信号。
 *
 * @param    __pid   接收信号的进程 ID（正数）或进程组 ID（负数）。
 * @param    __sig   要发送的信号编号（如 SIGINT、SIGTERM 等）。
 *
 * @retval   0       信号发送成功。
 * @retval  -1       发送失败，信号编号非法或系统调用出错。
 *
 * @note
 *  - __sig 必须为合法信号编号（范围 1 ~ NSIG-1）；
 *  - __pid 为 0 表示当前进程组，为 -1 表示广播（权限受限）；
 *  - 若调用失败，使用 `PRINT_ERROR()` 宏打印系统错误（应基于 errno）。
 */
int _sig_kill(__pid_t __pid, int __sig)
{
    if(!IS_VALID_SIGNAL(__sig))
        return -1;

    if(kill(__pid ,__sig) == -1){
        PRINT_ERROR();
        return -1;
    }

    return 0;
}

/**
 * @function _sig_raise
 * @brief    向当前进程自身发送一个信号。
 *
 * 封装标准库函数 raise，用于向当前进程自身发送一个信号，相当于 `kill(getpid(), sig)`。
 *
 * @param    __sig   要发送的信号编号（如 SIGINT、SIGTERM 等）。
 *
 * @retval   0       信号发送成功。
 * @retval  -1       发送失败（如信号编号非法或调用失败）。
 *
 * @note
 *  - __sig 必须为合法信号编号（范围 1 ~ NSIG-1）；
 *  - raise 不设置 errno，因此错误原因不可通过 perror 获取；
 *  - 使用场景包括触发自身中断、终止、重启等。
 */
int _sig_raise(int __sig)
{
    if(!IS_VALID_SIGNAL(__sig))
        return -1;

    if(raise(__sig) != 0){
        return -1;
    }

    return 0;
}

int _sig_pause(void)
{
    return puase();
}

/**
 * @function _sig_init
 * @brief    初始化信号处理结构体 __sig_t。
 *
 * 分配并初始化一个信号处理结构体和其内部的 sigaction 结构体，用于后续信号处理操作的封装。
 *
 * @retval   非 NULL 指针：初始化成功，返回指向新建 __sig_t 结构体的指针；
 * @retval   NULL        ：内存分配失败。
 *
 * @note
 *  - 使用 calloc 进行内存分配，结构体成员自动置零；
 *  - 若内部 sigaction 分配失败，会释放外层结构体；
 *  - 调用者需要在使用结束后调用相应的释放函数释放资源（如 `_sig_free(__sig)`）。
 */
__sig_t *_sig_init(void)
{
    __sig_t *__sig = (__sig_t *)calloc(1 ,sizeof(__sig_t));
    if(__sig == NULL)
       return NULL;

    __sig->__act = (struct sigaction *)calloc(1 ,sizeof(struct sigaction));    
    if(__sig->__act == NULL){
        free(__sig);
        __sig = NULL;
        return NULL;
    }

    __sig->__oact = NULL;
    __sig->__num = 0;

    return __sig;
}

/**
 * @function _sig_free
 * @brief    释放 __sig_t 结构体及其内部资源。
 *
 * 用于释放通过 `_sig_init()` 创建的信号处理结构体 __sig_t，包括其动态分配的
 * struct sigaction 成员 __act 和 __oact，确保无内存泄漏。
 *
 * @param    __sig   二级指针，指向 __sig_t 指针的地址（即 __sig_t **），
 *                   释放后该指针会被置为 NULL，防止野指针。
 *
 * @note
 *  - 调用者需确保传入的 __sig 是通过 `_sig_init()` 初始化的；
 *  - 函数内部会检查 NULL 指针，调用安全；
 *  - 可多次调用，不会造成重复释放或崩溃；
 *  - 推荐传入二级指针，便于释放后置空原始指针。
 */
void _sig_free(__sig_t **__sig)
{
    if(__sig == NULL || (*__sig) == NULL)
        return;

    if((*__sig)->__act != NULL){
        free((*__sig)->__act);
        (*__sig)->__act = NULL;
    }

    if((*__sig)->__oact != NULL){
        free((*__sig)->__oact);
        (*__sig)->__oact = NULL;
    }

    free((*__sig));
    (*__sig) = NULL;
}
