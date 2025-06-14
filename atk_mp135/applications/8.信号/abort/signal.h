#ifndef __SIGNAL_H
#define __SIGNAL_H

#include "file.h"
#include <signal.h>
/**
 * @struct __sig_t
 * @brief 封装信号管理所需参数的结构体，用于统一管理信号处理、屏蔽、发送等行为。
 */
struct _signal_struct {
    int __num;                    ///< 信号编号（如 SIGINT、SIGUSR1）
    int __seconds;               ///< 定时信号使用的秒数（如 alarm/setitimer）
    int __pid;                   ///< 目标进程 PID（用于 kill、sigqueue 等）
    struct sigaction *__act;     ///< 要注册的信号处理结构体（sigaction）
    struct sigaction *__oact;    ///< 保存旧的信号处理结构体（用于恢复）
    sigset_t *__sig_set;         ///< 信号集（用于屏蔽、等待、查询等）
    sigval_t __val;          ///< 附带的数据（用于实时信号 sigqueue 或 sigevent）
};
typedef struct _signal_struct __sig_t;

sig_t _sig_signal(const int signum, sig_t handler);
int _sig_sigaction(__sig_t *__psig);
int _sig_kill(const __pid_t __pid, const int __signum);
int _sig_raise(const int __signum);
unsigned int _sig_alarm(const unsigned int __seconds);
int _sig_pause(void);
int _sig_sigemptyset(sigset_t *__set);
int _sig_sigaddset(sigset_t *__set ,const int __signum);
int _sig_sigprocmask(int __how, const sigset_t *__set, sigset_t *__oset);
int _sig_sigsuspend(const sigset_t *__sig_set);
int _sig_sigismember(const sigset_t *__sig_set, int __signum);
int _sig_sigpending(sigset_t *__sig_set);
int _sig_sigqueue(__pid_t __pid, int __signum, const union sigval __val);
void _sig_free(__sig_t **__psig);
__sig_t *_sig_init(void);

/**
 * @macro   SIG_EXIT
 * @brief   释放信号资源并立即退出程序。
 */                             
#define SIG_EXIT(__psig ,__ret)\
                        do{\
                            _sig_free(&__psig);\
                            exit(__ret);\
                        }while(0)\

/**
 * @macro   SIG_CHECK_NUM
 * @brief   判断信号编号是否在有效范围内。
 */
#define SIG_CHECK_NUM(__signum)\
                                ((__signum) >= 0 && (__signum) < _NSIG)
/**
 * @macro SIG_CHECK_PROCESS
 * @brief 检查指定进程是否存在（可访问）。
 */
#define SIG_CHECK_PROCESS(__pid)\
                                (_sig_kill(__pid ,0) == -1 ? -1 : 0)
                                                   
#define __abort     abort()
#endif