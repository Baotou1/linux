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
#define LIST_HEAD (0x01)
/**
 * @def    GET_FILE_LIST_NODE
 * @brief  从链表头成员指针获取所属的文件链表节点结构体指针
 *
 * @param[in] ptr  指向 `_list_h` 类型成员的指针（即 __list_h）
 *
 * @return 指向所属的 `__flist_t` 类型节点结构体的指针
 *
 * @details
 *  本宏基于 `offsetof` 宏计算偏移，从嵌套的链表头成员 `__list_h`
 *  的地址还原出包含它的完整 `__flist_t` 文件链表节点结构体指针。
 *  适用于链表遍历过程中将 `_list_h*` 指针还原为节点结构体指针。
 */
#define GET_FILE_LIST_NODE(ptr)\
                            ((__flist_t *)((char *)(ptr) - offsetof(__flist_t, __list_h)))


__flist_t *__file_list_init(void);
void __file_list_free(__flist_t **__pfl);
int __file_list_add_nd(__flist_t *__pfl ,_file_t *__pf);
int __file_list_find_nd(__flist_t **__pfl ,const char *__pathname);
int __file_list_delete_nd(__flist_t **__pfl ,const char *__pathname);                     
#endif