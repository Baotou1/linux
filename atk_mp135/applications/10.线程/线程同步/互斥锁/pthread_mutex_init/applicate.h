#ifndef __APPLICATE_H
#define __APPLICATE_H

#include "process.h"
#include "log.h"
#include "signal.h"

extern unsigned int g_count;
extern __sync_lock_t lock;;
void *__thread_1(void *arg);
void *__thread_2(void *__arg);
#endif