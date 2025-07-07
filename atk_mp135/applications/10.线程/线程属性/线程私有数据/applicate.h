#ifndef __APPLICATE_H
#define __APPLICATE_H

#include "init.h"

extern unsigned int __count;
extern double __tim1 ,__tim2;
extern __thd_tls_t __tls;
void *__thread_1(void *arg);
void *__thread_2(void *__arg);
#endif