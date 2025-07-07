/**
 * @file    file_looplist.h
 * @brief   文件循环链表节点及操作接口定义
 *
 * 本头文件定义了用于管理文件循环链表的节点结构体、相关宏及链表操作函数接口。
 * 主要内容包括：
 *  - 文件循环链表节点结构体 (__flist_t) 的定义，包含文件指针、链表头及节点标识；
 *  - 用于根据链表头成员指针还原节点指针的宏 GET_FILE_LIST_NODE；
 *  - 链表初始化、添加、查找、删除、释放等接口函数声明。
 *
 * 设计说明：
 *  - 采用单向循环链表结构管理文件节点，头节点通过 __index 字段标识为 LIST_HEAD。
 *  - 链表操作函数支持自动寻找头节点，确保链表操作的一致性和安全性。
 *  - 提供简单的链表节点操作接口，方便文件资源的动态管理。
 *
 * 依赖：
 *  - 依赖 "file.h" 定义文件结构体及文件操作接口。
 *  - 依赖 "list_head.h" 定义链表头结构体及相关宏。
 *
 * @author  
 * @date    
 */
#ifndef __FILE_LOOPLIST_H
#define __FILE_LOOPLIST_H

#include "file.h"
#include "list_head.h"

/**
 * @struct __file_looplist_struct
 * @brief  文件循环链表节点结构体
 *
 * @details
 *  表示循环链表中的一个节点，用于管理文件对象的链表结构。
 *  每个节点包含：
 *    - 指向文件结构体的指针 (__pf)，用于关联具体的文件信息。
 *    - 内嵌的链表头结构体 (__list_h)，实现节点之间的循环连接。
 *    - 整型字段 (__index)，用于标记节点身份（例如是否为链表头节点）或记录节点索引。
 */
struct __file_looplist_struct 
{
    _file_t *__pf;     ///< 指向当前节点关联的文件结构体指针
    _list_h __list_h;  ///< 链表头结构体，用于连接到下一个链表节点
    int __index;       ///< 节点标识或索引，可用于区分头节点或普通节点
};
typedef struct __file_looplist_struct __flist_t;

/**
 * @def   GET_FLIST_NODE
 * @brief 从链表节点指针还原其所属的 __flist_t 文件链表结构体指针
 *
 * @param[in] ptr  指向 `_list_h` 成员（通常为 __list_h）的指针
 * @return         指向所属的 `__flist_t` 结构体的指针
 *
 * @details
 * 使用 `offsetof(__flist_t, __list_h)` 计算链表节点在结构体中的偏移量，
 * 再通过指针减法还原出完整的 `__flist_t*` 指针。该宏常用于链表遍历中，
 * 从 `_list_h*` 指针还原出其包含的结构体，便于访问业务字段。
 */
#define GET_FLIST_NODE(ptr)\
                            ((__flist_t *)((char *)(ptr) - offsetof(__flist_t, __list_h)))
/**
 * @def   FLIST_FIND_HEAD
 * @brief 遍历文件循环链表，查找标记为头节点的结构体 (__index == LIST_HEAD)
 *
 * @param[in,out] __h   当前链表节点指针（类型为 _list_h*），遍历后更新为头节点位置
 * @param[out]    __nd  输出参数，指向查找到的 `__flist_t*` 结构体指针
 *
 * @details
 * 从任意节点出发，循环查找 `__index == LIST_HEAD` 的节点，并将其作为头节点返回。
 * 使用 `GET_FLIST_NODE` 将 `_list_h*` 转换为对应的 `__flist_t*` 结构体指针以判断。
 *
 * 注意事项：
 * - 链表必须是循环链表；
 * - `__h` 必须为 `_list_h*` 类型，且结构体中包含 `__index` 成员；
 * - 该宏应在返回类型为 `int` 的函数中使用，循环失败后需手动判断处理；
 * - 指针比较使用 `uintptr_t`，防止 64 位系统下地址截断问题；
 * - 若遍历一圈仍未找到头节点，将自动退出循环，避免死循环。
 */
#define FLIST_FIND_HEAD(__h,__nd)\
                                do{\
                                    uintptr_t __p=(uintptr_t)(__h);           \
                                    while(1){                                 \
                                        (__h) = (__h)->__next;                \
                                        (__nd) = GET_FLIST_NODE(__h);         \
                                        if((__nd)->__index == LIST_HEAD)      \
                                            break;                            \
                                        if(__p == (uintptr_t)(__h))           \
                                            break;                            \
                                    }                                         \
                                }while(0)


 /* 链表接口函数声明 */
__flist_t *__file_list_init(void);
void __file_list_free(__flist_t **__pl);
int __file_list_add_nd(__flist_t *__pl ,_file_t *__pf);
int __file_list_find_nd(__flist_t **__pl ,const char *__pathname);
int __file_list_delete_nd(__flist_t **__pl ,const char *__pathname);   

#endif