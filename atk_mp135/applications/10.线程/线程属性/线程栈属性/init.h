/**
 * @file    init.h
 * @brief   程序初始化模块头文件
 * 
 * 本头文件声明了程序启动时常用的初始化函数接口，包括日志系统初始化、进程结构初始化、
 * 线程初始化等。封装了基础设施的启动流程，方便主函数调用，确保系统稳定运行。
 * 
 * @note
 * - 推荐调用顺序：log_init → process_init → thread_init；
 * - 初始化失败时函数内部会处理错误（如直接退出或安全释放资源）；
 * - 该模块依赖 process.h、log.h、signal.h 等基础模块。
 * 
 * @author  baotou
 * @date    2025-06-28
 */
#ifndef __INIT_H
#define __INIT_H

#include "process.h"   /**< 进程管理模块，定义 __proc 结构体及其生命周期函数 */
#include "log.h"       /**< 日志系统模块，提供 _log_init、LOG_PRINT 等接口 */
#include "signal.h"    /**< 信号管理模块，用于注册退出处理函数等 */

/* 接口函数声明 */
void log_init(void);
void process_init(void);
void thread_sync_init(void);
void thread_init(void);

void thread_exit_handler(void *__arg);

extern unsigned int __count;
extern __tsync_rwlock_t __rwlock;
extern __tsync_sem_t __sem;

#endif /* __INIT_H */