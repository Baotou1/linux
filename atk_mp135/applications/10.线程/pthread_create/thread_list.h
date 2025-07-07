/**
 * @file    thread_list.h
 * @brief   线程双向循环链表模块头文件
 *
 * @details
 * 本模块用于实现一个轻量级的线程节点链表结构，便于对多个线程对象进行组织管理，
 * 例如线程池、线程调度表、任务线程映射等。提供线程节点的创建、插入、查找、删除、
 * 释放等基本操作接口。
 *
 * 核心设计为一个封装线程对象（__thd_t *）的结构体 __tlist_t，并使用嵌套的双向链表头
 * (_dlist_h) 实现节点之间的循环连接。支持 O(n) 的查找，O(1) 的头尾插入操作。
 *
 * 本头文件中包含：
 *  - __tlist_t：线程循环链表节点结构体定义
 *  - 接口函数声明：
 *      - __thd_list_init          初始化链表头
 *      - __thd_list_add_nd        添加线程节点
 *      - __thd_list_find_nd       查找线程节点
 *      - __thd_list_delete_nd     删除线程节点
 *      - __thd_list_free          释放整个链表
 *  - 宏定义：
 *      - GET_FILE_LIST_NODE       从链表成员指针获取完整结构体指针的便捷宏
 *
 * @note
 * 本模块依赖以下外部模块：
 *  - thread.h      定义了 __thd_t 结构体（线程对象）
 *  - list_head.h   提供 _dlist_h 结构体（双向循环链表结构）
 *
 * 使用者需确保所有线程节点使用动态分配（malloc/calloc）创建，避免释放非法指针。
 */

 #ifndef __THREAD_LIST_H
 #define __THREAD_LIST_H
 
 #include "thread.h"
 #include "list_head.h"
 
 /**
  * @struct __thread_looplist_struct
  * @brief  线程循环链表节点结构体
  *
  * @details
  * 用于组织和管理线程对象的循环链表结构体。
  * 每个节点封装一个线程实体 (__thd_t)，并通过链表指针 (__dlist_h)
  * 与其他节点相连，构成循环链表。该结构可用于线程池、线程调度器等模块。
  *
  * 成员说明：
  *  - __pthd    : 线程对象，封装 pthread_t、状态、回调参数等；
  *  - __dlist_h : 链表结构头，用于将多个节点连接成循环链表；
  *  - __index   : 节点标识，可用于标记“头节点”或作为唯一索引号。
  */
 struct __thread_looplist_struct 
 {
     __thd_t *__pthd;          ///< 当前节点关联的线程对象
     _dlist_h __dlist_h;       ///< 双向循环链表指针域
     int __index;              ///< 节点标识或索引，用于识别链表头
     int __num;
 };
 typedef struct __thread_looplist_struct __tlist_t;
 
/**
 * @def   GET_TLIST_NODE
 * @brief 从链表节点指针还原其所属的线程链表结构体指针
 *
 * @param[in] __ptr  指向 _dlist_h 成员的指针（通常为 __dlist_h）
 * @return     __tlist_t* 类型的完整结构体指针
 *
 * @details
 * 使用 offsetof 宏计算 __dlist_h 在 __tlist_t 中的偏移量，
 * 通过指针减法从 _dlist_h 指针反向还原出所属的 __tlist_t 结构体指针。
 * 常用于链表遍历中，将链表节点转换为包含它的结构体。
 */
 #define GET_TLIST_NODE(__ptr)\
                                 ((__tlist_t *)((char *)(__ptr) - offsetof(__tlist_t, __dlist_h)))

/**
 * @def   TLIST_FIND_HEAD
 * @brief 在线程链表中查找 __index == LIST_HEAD 的头节点
 *
 * @param[in,out] __h   当前链表节点指针（类型为 _dlist_h*），遍历后指向头节点
 * @param[out]    __nd  输出参数，指向找到的 __tlist_t 结构体指针
 *
 * @details
 * 该宏用于从任意链表节点开始，遍历查找标记为头节点（__index == LIST_HEAD）的节点。
 * 遍历过程使用 GET_TLIST_NODE 宏将链表节点还原为结构体指针进行判断。
 * 若发生回到起始节点仍未找到头节点的情况，将终止循环，避免死循环。
 *
 * 注意事项：
 * - 要求链表结构为循环链表；
 * - __h 必须为 _dlist_h* 类型，且结构中包含 __index 字段；
 * - __nd 必须为 __tlist_t* 类型；
 * - 需用于返回值为 int 的函数中，若查找失败应额外处理；
 * - 为防止指针截断，使用 uintptr_t 比较地址；
 * - 宏内不包含 return，请在调用宏后手动处理查找失败情况（如 __nd == NULL）。
 */
#define TLIST_FIND_HEAD(__h,__nd)\
                                do{\
                                    uintptr_t __p=(uintptr_t)(__h);           \
                                    while(1){                                 \
                                        (__h) = (__h)->__next;                \
                                        (__nd) = GET_TLIST_NODE(__h);         \
                                        if((__nd)->__index == LIST_HEAD)      \
                                            break;                            \
                                        if(__p == (uintptr_t)(__h))           \
                                            break;                            \
                                    }                                         \
                                }while(0)
 
 /* 链表接口函数声明 */
__tlist_t *__thd_list_init(void);
void __thd_list_free(__tlist_t **__pl);
int __thd_list_add_nd(__tlist_t *__pl,__thd_t *__pthd);
int __thd_list_find_nd(__tlist_t **__pl, const char *__name);
int __thd_list_delete_nd(__tlist_t **__pl, const char *__name);
/**
 * @macro  THREAD_INFO
 * @brief  打印线程信息日志（支持不同操作类型如 ADD/EXIT）
 *
 * @param[in]  __proc  指向当前进程结构体的指针（包含线程链表等）
 * @param[in]  __act   日志动作标识，例如 ADD、EXIT，会自动转换为字符串插入日志标签中
 *
 * @details
 *  该宏用于统一输出线程操作日志，格式如下：
 *      [THREAD][动作] info: name=线程名, pid=进程ID, tid=线程ID
 *  
 *  使用示例：
 *      THREAD_INFO(__proc, ADD);    // 打印添加线程信息
 *      THREAD_INFO(__proc, EXIT);   // 打印退出线程信息
 *
 *  注意：
 *  - `__act` 是一个标识符（非字符串），宏中会使用 `#` 操作符自动转换为字符串；
 *  - `__proc->__pthdl->__pthd` 应指向当前线程节点；
 *  - 外部调用时需确保 `__proc`、`__pthdl`、`__pthd` 均已初始化。
 */
//(__proc)->__pid
#define THREAD_INFO_LOG(__pthd, __act)\
                                do{\
                                    _log_write("[THREAD][" #__act "]", "info: name->%s, pid->%d ,tid->%lu", \
                                        __pthd->__name, \
                                        getpid(),\
                                        __pthd->__id); \
                                }while(0)


#endif /* __THREAD_LIST_H */