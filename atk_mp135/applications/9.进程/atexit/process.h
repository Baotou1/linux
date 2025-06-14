#ifndef __PROCESS_H
#define __PROCESS_H

#include "file.h"

#define PROC_EOK   0x00
#define PROC_ERROR 0x01

struct __proc_struct
{
    void (*__function)(void);
};
typedef struct __proc_struct __proc_t;

int _proc_atexit(void (*__fun)(void));
#endif
