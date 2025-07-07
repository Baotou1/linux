/**
 * @file    list_head.h
 * @brief   链表头节点结构体定义头文件
 *
 * @details
 *  本文件定义了用于实现单向链表和循环链表的基础链表头结构体 `_list_head`。
 *  结构体仅包含一个指针成员 `next`，指向链表中的下一个节点，形成节点之间的链接。
 *  
 *  该结构体设计灵活，适合嵌入其他数据结构中以实现链表功能，
 *  支持链表的动态添加、删除、遍历等操作。
 *  
 *  使用本头文件定义的 `_list_h` 类型，可以方便地实现各种链表算法和数据结构，
 *  适合嵌入式系统、操作系统内核以及用户态程序中对链表的需求。
 *
 * @note
 *  仅定义链表头结构，未包含具体链表操作函数，需结合相应实现文件使用。
 *  
 * @author
 * @date
 */
#ifndef __LIST_HEAD_H
#define __LIST_HEAD_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/**
 * @struct _list_head
 * @brief 链表头节点结构体，用于构建单向链表的节点连接
 *
 * @details
 *  该结构体只包含一个指针成员 next，指向下一个链表节点的头部。
 *  它是实现循环链表或单链表的基础，用于链表节点间的链接。
 *  具体的数据结构体通常会将此结构体作为成员嵌入，实现链表操作时通过
 *  链表头节点的指针遍历和管理。
 */
struct _list_head{
    struct _list_head *next;  ///< 指向链表中下一个节点的指针，形成链表连接
};
typedef struct _list_head _list_h;

#endif