#ifndef __SIGNAL_H
#define __SIGNAL_H

#include "file.h"
#include <signal.h>


#define IS_VALID_SIGNAL(__signum) ((__signum) > 0 && (__signum) < _NSIG)

struct _signal_struct
{
    int __num;
    struct sigaction *__act;
    struct sigaction *__oact;
};
typedef struct _signal_struct __sig_t;

sig_t _sig_signal(int signum, sig_t handler);
int _sig_sigaction(__sig_t *__sig);
int _sig_kill(__pid_t __pid, int __sig);
int _sig_raise(int __sig);
int _sig_pause(void);
__sig_t *_sig_init(void);
void _sig_free(__sig_t **__sig);
#endif