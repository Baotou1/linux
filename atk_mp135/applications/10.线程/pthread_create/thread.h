/**
 * @file    thread.h
 * @brief   线程模块头文件，定义线程结构体及线程操作接口
 *
 * @details
 * 本文件定义线程相关的数据结构和函数接口，封装线程的基本信息
 * 及线程的创建、管理和退出操作。通过前向声明避免进程结构体
 * 的循环依赖问题，提升模块耦合度。
 *
 * 主要内容：
 *  - __proc_t 的前向声明，用于避免与进程模块的循环依赖；
 *  - __thread_struct 线程结构体定义，封装线程名称、线程ID、
 *    线程属性和线程执行入口函数；
 *  - 线程相关接口函数声明：
 *      - __thread_getid       : 获取当前线程ID；
 *      - __thread_init        : 初始化线程结构体；
 *      - __thread_create      : 创建线程（封装 pthread_create）；
 *      - __thread_free        : 释放线程结构体内存；
 *      - __thread_exit        : 线程退出并清理资源。
 *
 * @note
 * 线程入口函数的形参和返回值均为 void*，兼容 pthread 标准。
 */

 #ifndef __THREAD_H
 #define __THREAD_H
 
 #include "file.h"
 #include <pthread.h>
 
 /* 
  * 前向声明及类型别名定义：
  * 声明 __proc_t 为 struct __proc_struct 的别名，
  * 以避免循环依赖，便于使用指向该结构体的指针。
 
#ifndef __THREAD_H
#define __THREAD_H

#include "file.h"
#include <pthread.h>

/* 
 * 前向声明及类型别名定义：
 * 告诉编译器 __proc_t 是 struct __proc_struct 的别名，
 * 但此时并未定义 struct __proc_struct 的具体内容。
 * 这样可以在不暴露结构体内部细节的情况下，
 * 使用指向该结构体的指针（如 __proc_t *）进行函数参数声明，
 * 以避免头文件之间的循环依赖和耦合。
 */
typedef struct __proc_struct __proc_t;

/**
 * @struct __thread_struct
 * @brief  线程结构体，封装线程相关信息
 * 
 * @details
 * 封装线程名称、线程ID、线程属性和线程执行函数指针，
 * 用于线程的创建、管理和调度。
 */
struct __thread_struct
{
    char __name[20];                      ///< 线程名称，最多19字符+终止符
    pthread_t __id;                       ///< 线程ID，由 pthread_create 生成
    pthread_attr_t *__attr;               ///< 线程属性指针，可为 NULL 表示默认属性
    void *(*__start_routine) (void *);    ///< 线程入口函数指针，线程执行的函数
};
typedef struct __thread_struct __thd_t;

 /* 链表接口函数声明 */
pthread_t __thread_getid(void);
__thd_t *__thread_init(char *__name ,void *(*__start_routine) (void *));
int __thread_create(__thd_t *__pthd);
void __thread_free(__thd_t **__pthd);
void __thread_exit(__proc_t *__proc ,__thd_t *__pthd ,void *__ret);

#endif

