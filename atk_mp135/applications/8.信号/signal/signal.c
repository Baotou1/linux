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