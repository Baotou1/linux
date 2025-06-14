#ifndef __SIGNAL_H
#define __SIGNAL_H

#include "file.h"
#include <signal.h>

struct _signal_struct
{
    int __num;
    int __seconds;
    int __pid;
    struct sigaction *__act;
    struct sigaction *__oact;
    sigset_t *__sig_set;
    //sigset_t *__mask;
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
__sig_t *_sig_init(void);
void _sig_free(__sig_t **__psig);


#define SIG_IS_VALID(__signum)\
                                 ((__signum) > 0 && (__signum) < _NSIG)
                                 
#define SIG_EXIT(__psig)\
                        do{\
                            _sig_free(&__psig);\
                            exit(-1);\
                        }while(0)\


#endif