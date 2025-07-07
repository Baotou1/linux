#ifndef __APPLICATE_H
#define __APPLICATE_H

#include "process.h"
#include "log.h"
#include "signal.h"

extern __sync_cond_t __cond;
extern unsigned int __count;
void *__thread_1(void *arg);
void *__thread_2(void *__arg);
#endif