#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "looplist.h"

_list* list_init(int data){
    _list* list = (_list*)malloc(sizeof(_list));
    if(!list) return NULL;

    list->data = data;
    list->list_h.next = &list->list_h;// 自我指向，形成循环链表

    return list;
}

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

_list* list_find_nd(_list *list ,int data){
    if(!list) return NULL;

    _list_h *p = &list->list_h;
    do{
        _list *nd = GET_LIST_NODE(p); 
        if(nd->data == data) return nd;
        p = p->next;
    }while(p != &list->list_h);
}

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


