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
sig_t _sig_signal(const int __signum, sig_t __handler)
{
    if(__handler == NULL)
        return SIG_ERR;

    if(!SIG_IS_VALID(__signum))
        return SIG_ERR;
 
    sig_t __sig = signal(__signum ,__handler);
    if(__sig == SIG_ERR)
    {
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
 * @param    __psig  指向已初始化的 __sig_t 结构体，必须包含有效的信号编号和动作指针。
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

int _sig_sigaction(__sig_t *__psig)
{
    if(__psig == NULL || __psig->__act == NULL)
        return -1; 

    if(!SIG_IS_VALID(__psig->__num))
        return -1;

    if(sigaction(__psig->__num ,__psig->__act ,__psig->__oact) == -1)
    {
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
 * @param    __signum   要发送的信号编号（如 SIGINT、SIGTERM 等）。
 *
 * @retval   0       信号发送成功。
 * @retval  -1       发送失败，信号编号非法或系统调用出错。
 *
 * @note
 *  - __signum 必须为合法信号编号（范围 1 ~ NSIG-1）；
 *  - __pid 为 0 表示当前进程组，为 -1 表示广播（权限受限）；
 *  - 若调用失败，使用 `PRINT_ERROR()` 宏打印系统错误（应基于 errno）。
 */
int _sig_kill(const __pid_t __pid, const int __signum)
{
    if(!SIG_IS_VALID(__signum))
        return -1;

    if(kill(__pid ,__signum) == -1)
    {
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
 * @param    __signum   要发送的信号编号（如 SIGINT、SIGTERM 等）。
 *
 * @retval   0       信号发送成功。
 * @retval  -1       发送失败（如信号编号非法或调用失败）。
 *
 * @note
 *  - __signum 必须为合法信号编号（范围 1 ~ NSIG-1）；
 *  - raise 不设置 errno，因此错误原因不可通过 perror 获取；
 *  - 使用场景包括触发自身中断、终止、重启等。
 */
int _sig_raise(const int __signum)
{
    if(!SIG_IS_VALID(__signum))
        return -1;

    if(raise(__signum) != 0)
    {
        return -1;
    }

    return 0;
}

/**
 * @function _sig_alarm
 * @brief    设置一个定时器，在指定秒数后向进程发送 SIGALRM 信号。
 *
 * 封装标准库函数 alarm()，用于在给定的秒数后向当前进程发送 SIGALRM 信号。
 * 如果先前已设置了闹钟，则新的调用将替换原有的定时器，并返回之前剩余的时间。
 *
 * @param    __seconds   多少秒后发送 SIGALRM 信号；0 表示取消先前设置的定时器。
 *
 * @retval   0           没有原有定时器；
 * @retval  >0           返回先前定时器剩余的秒数。
 *
 * @note
 *  - SIGALRM 到达时，若未处理则默认终止进程；
 *  - 可结合 signal() 或 sigaction() 设置 SIGALRM 的处理函数；
 *  - alarm() 只适用于秒级定时，如需更精细的定时建议使用 setitimer()。
 */
unsigned int _sig_alarm(const unsigned int __seconds)
{
    return alarm(__seconds);
}

/**
 * @function _sig_pause
 * @brief    挂起当前进程，直到捕获到一个信号并且其处理函数执行完毕。
 *
 * 封装标准库函数 pause()，使进程挂起（阻塞）直到收到一个信号，并完成其处理。
 * 通常与 alarm() 或 kill() 配合使用实现延时或信号控制。
 *
 * @retval  -1           始终返回 -1；
 *
 * @note
 *  - 返回时 errno 通常为 EINTR，表示被信号中断；
 *  - 若没有设置信号处理器（signal handler），则进程可能会终止；
 *  - 建议配合 sigaction() 明确设置信号处理函数。
 */
int _sig_pause(void)
{
    return pause();
}

/**
 * @function _sig_sigemptyset
 * @brief    初始化一个空的信号集。
 *
 * 封装 sigemptyset，用于将指定信号集清空（不包含任何信号）。
 *
 * @param    __set   指向 sigset_t 结构体的指针。
 *
 * @retval   0       成功清空信号集。
 * @retval  -1       失败，参数为 NULL 或调用 sigemptyset 出错。
 *
 * @note
 *  - 该函数通常用于初始化信号掩码（如配合 sigprocmask 使用）；
 *  - 清空后，可通过 sigaddset 添加需要阻塞的信号；
 *  - 若调用失败，可通过 PRINT_ERROR() 输出 errno。
 */
int _sig_sigemptyset(sigset_t *__set)
{
    if(__set == NULL)
        return -1;

    if(sigemptyset(__set) == -1)
    {
        PRINT_ERROR();
        return -1;
    }

    return 0;
}

/**
 * @function _sig_sigaddset
 * @brief    向信号集中添加一个信号。
 *
 * 封装 sigaddset，用于将指定信号添加到信号集中。
 *
 * @param    __set      指向 sigset_t 的指针。
 * @param    __signum   要添加的信号编号。
 *
 * @retval   0          添加成功。
 * @retval  -1          添加失败，参数无效或添加出错。
 *
 * @note
 *  - __signum 必须为合法信号编号；
 *  - 添加后该信号将包含在信号集里（如用于阻塞等操作）。
 */
int _sig_sigaddset(sigset_t *__set ,const int __signum)
{
    if(__set == NULL)
        return -1;

    if(!SIG_IS_VALID(__signum))
        return -1;

    if(sigaddset(__set ,__signum) == -1)
    {
        PRINT_ERROR();
        return -1;
    }

    return 0;
}

/**
 * @function _sig_sigprocmask
 * @brief    修改进程的信号屏蔽字（信号掩码）。
 *
 * 封装 sigprocmask，可实现对进程信号掩码的添加、删除、覆盖。
 *
 * @param    __how   操作类型（SIG_BLOCK / SIG_UNBLOCK / SIG_SETMASK）。
 * @param    __set   指向新信号集的指针。
 * @param    __oset  可选，用于保存旧的信号集（可为 NULL）。
 *
 * @retval   0       设置成功。
 * @retval  -1       设置失败，参数非法或系统调用错误。
 *
 * @note
 *  - 用于阻塞、解除阻塞或替换当前信号掩码；
 *  - 若传入 __oset，可获取原掩码备份；
 *  - 常与 sigemptyset/sigaddset 配合使用；
 *  - 错误信息通过 PRINT_ERROR() 打印。
 */
int _sig_sigprocmask(int __how, const sigset_t *__set, sigset_t *__oset)
{
    if(__how != SIG_BLOCK && __how != SIG_UNBLOCK && __how != SIG_SETMASK)
        return -1;

    if(__set == NULL)
        return -1;    

    if(sigprocmask(__how ,__set ,__oset) == -1)
    {
        PRINT_ERROR();
        return -1;
    }
    return 0;
}
/**
 * @function _sig_free
 * @brief    释放 __sig_t 结构体及其内部资源。
 *
 * 用于释放通过 `_sig_init()` 创建的信号处理结构体 __sig_t，包括其动态分配的
 * struct sigaction 成员 __act 和 __oact，确保无内存泄漏。
 *
 * @param    __psig   二级指针，指向 __sig_t 指针的地址（即 __sig_t **），
 *                   释放后该指针会被置为 NULL，防止野指针。
 *
 * @note
 *  - 调用者需确保传入的 __psig 是通过 `_sig_init()` 初始化的；
 *  - 函数内部会检查 NULL 指针，调用安全；
 *  - 可多次调用，不会造成重复释放或崩溃；
 *  - 推荐传入二级指针，便于释放后置空原始指针。
 */
void _sig_free(__sig_t **__psig)
{
    if(__psig == NULL || (*__psig) == NULL)
        return;

    if((*__psig)->__act != NULL){
        free((*__psig)->__act);
        (*__psig)->__act = NULL;
    }

    if((*__psig)->__oact != NULL){
        free((*__psig)->__oact);
        (*__psig)->__oact = NULL;
    }

    if((*__psig)->__sig_set != NULL){
        free((*__psig)->__sig_set);
        (*__psig)->__sig_set = NULL;
    }

    free((*__psig));
    (*__psig) = NULL;
}

/**
 * @function _sig_init
 * @brief    初始化信号处理结构体 __sig_t。
 *
 * 分配并初始化一个信号处理结构体及其内部成员，用于封装信号处理操作。
 *
 * @retval   非 NULL 指针：初始化成功，返回指向新建 __sig_t 的指针；
 * @retval   NULL        ：内存分配失败。
 *
 * @note
 *  - 使用 calloc 分配内存，结构体成员会自动清零；
 *  - 若任何一步失败，会自动释放已分配的资源；
 *  - 使用结束后应调用 `_sig_free(&__sig)` 释放。
 */
__sig_t *_sig_init(void)
{
    __sig_t *__psig = (__sig_t *)calloc(1 ,sizeof(__sig_t));
    if(__psig == NULL)
       return NULL;

    __psig->__act = (struct sigaction *)calloc(1 ,sizeof(struct sigaction));    
    if(__psig->__act == NULL)
    {
        _sig_free(&__psig);
        return NULL;
    }

    __psig->__sig_set = (sigset_t *)calloc(1 ,sizeof(sigset_t));
    if(__psig->__sig_set == NULL)
    {
        _sig_free(&__psig);
        return NULL;
    }

    return __psig;
}