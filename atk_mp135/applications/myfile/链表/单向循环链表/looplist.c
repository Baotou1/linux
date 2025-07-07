/**
 * @file    looplist.c
 * @brief   循环链表操作实现文件
 *
 * @details
 *  本文件实现了基于循环单链表的数据结构及其相关操作函数，包括链表初始化、
 *  节点添加、查找、删除、释放以及打印等功能。链表节点通过内部链表头结构体成员
 *  实现循环连接，支持高效遍历和灵活操作。
 *
 *  设计参考了内核链表思想，使用宏将链表节点转换回包含数据的外层结构体指针，
 *  方便用户操作数据元素。适合嵌入式和系统底层开发中对链表功能的需求。
 *
 * @author  baotou
 * @date    2025-06-18
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "looplist.h"

/**
 * @func   list_init
 * @brief  初始化循环链表，创建头节点
 *
 * @param[in] data 头节点数据
 *
 * @retval _list*  返回新链表头指针，失败返回 NULL
 *
 * @details
 *  创建一个新的循环链表，头节点的 next 指针指向自身，形成循环结构。
 */
_list* list_init(int data){
    _list* list = (_list*)malloc(sizeof(_list));
    if(!list) return NULL;

    list->data = data;
    list->list_h.next = &list->list_h;// 自我指向，形成循环链表

    return list;
}

/**
 * @func   list_add_nd
 * @brief  在循环链表尾部添加一个节点
 *
 * @param[in] list 循环链表头节点指针
 * @param[in] data 新节点数据
 *
 * @details
 *  创建一个新节点，将其插入链表尾部，保持循环链表结构不变。
 *  如果链表为空则不操作。
 */
void list_add_nd(_list *list ,int data){
    if(!list) return;
    
    _list *nd = (_list*)malloc(sizeof(_list));
    if(!nd) return;

    _list_h *p = &list->list_h;
    do{
        p = p->next; 
    }while(p->next != &list->list_h);
    
    nd->data = data;
    p->next = &nd->list_h;
    nd->list_h.next = &list->list_h;
}

/**
 * @func   list_find_nd
 * @brief  查找链表中第一个匹配数据的节点
 *
 * @param[in] list 循环链表头节点指针
 * @param[in] data 要查找的数据
 *
 * @retval _list* 找到返回节点指针，找不到返回 NULL
 */
_list* list_find_nd(_list *list ,int data){
    if(!list) return NULL;

    _list_h *p = &list->list_h;
    do{
        _list *nd = GET_LIST_NODE(p); 
        if(nd->data == data) return nd;
        p = p->next;
    }while(p != &list->list_h);
}

/**
 * @func   list_delete_nd
 * @brief  删除链表中第一个匹配数据的节点
 *
 * @param[in] list 循环链表头节点指针
 * @param[in] data 要删除的数据
 *
 * @retval _list* 返回删除节点后的链表头指针（可能更改）
 *
 * @details
 *  支持删除头节点和非头节点。
 *  如果删除头节点，返回新的头节点指针。
 *  若链表只剩一个节点且被删除，则返回 NULL。
 */
_list* list_delete_nd(_list *list ,int data){
    if(!list) return NULL;

    /* 是否删除头节点 */
    if(list->data == data){
        /* 链表只有一个头节点 */
        if(list->list_h.next == &list->list_h){
            free(list);
            return NULL;
        }
        else{
            _list_h *p = list->list_h.next;
            while(p->next != &list->list_h){
                p = p->next;
            }
            _list_h *head = list->list_h.next;
            p->next = head;
            free(list);
            return GET_LIST_NODE(head);
        }
    }
    else{
        _list_h *pr = &list->list_h;/* 记录p节点上一个位置 */
        _list_h *p = pr->next;      /* 匹配删除节点 */
        while(p != &list->list_h)
        {
            _list_h* pn = p->next;      /* 记录p节点下一个位置 */
            _list *nd = GET_LIST_NODE(p);
            if(nd->data == data){
                pr->next = pn;
                free(nd);
                break;
            }
            pr = p;
            p = pn;
        }
    }
    return list;
}

/**
 * @func   list_free
 * @brief  释放整个循环链表所有节点内存
 *
 * @param[in] list 链表头指针
 *
 * @details
 *  释放链表中所有节点的内存，包括头节点，防止内存泄漏。
 */
void list_free(_list* list)
{
    if(!list) return;

    _list_h *p = list->list_h.next;       /* 从第一个节点开始（跳过头节点） */
    while(p != &list->list_h){
        _list_h *next = p->next;                                        
        _list *nd = GET_LIST_NODE(p);     /* 使用宏将list_h指针转换回整个节点结构体的指针 */
        free(nd);
        p = next;
    }
    free(list); /* 释放循环链表头部 */
}

/**
 * @func   list_print
 * @brief  打印链表中所有节点数据
 *
 * @param[in] list 链表头指针
 *
 * @details
 *  按顺序遍历链表并打印节点中的整数数据，方便调试观察链表结构。
 */
void list_print(_list* list)
{
    if(!list) return;
    _list_h *p = &list->list_h;
    
    do{
        _list *nd = GET_LIST_NODE(p);
        printf("%d " ,nd->data);
        p = p->next;
    } while(p != &list->list_h);   
    
    printf("\n");
}


