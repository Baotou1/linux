#include <stdio.h>
#include <stdlib.h>

struct _list_t
{
    int data;
    struct _list_t *nd;
    
};
typedef struct _list_t _list;

/* 初始化链表 */
_list* list_init_head(int data)
{
    _list *plist = (_list*)malloc(sizeof(_list));
    if(plist == NULL)
        return NULL;
    plist->data = data;
    plist->nd = NULL;
    return plist;
}

/* 释放链表 */
void list_free(_list* plist)
{
    if(plist == NULL)
        return;
    _list* templist;
    while(plist != NULL)
    {
        templist = plist;
        plist = plist->nd;
        free(templist);
    }
}
/* 链表尾插法加入节点 */
void list_add_nd(_list* plist1 ,int data)
{
    if(plist1 == NULL)
        return;
    _list *newplist = (_list*)malloc(sizeof(_list));
    if(newplist == NULL)
        return;
    while(plist1->nd != NULL){
        plist1 = plist1->nd;
    }
    newplist->data = data;
    newplist->nd = plist1->nd;
    plist1->nd = newplist;
}
/* 链表组合 */
_list* list_add_list(_list* plist1 ,_list* plist2)
{
    if(plist1 == NULL || plist2 == NULL)
        return NULL;

    _list *tail = plist1;
    while(tail->nd != NULL)
    {
        tail = tail->nd;
    }
    tail->nd = plist2;
    return plist1;
}
/* 查找节点 */
_list* list_find_nd(_list* plist ,int data)
{
    if(plist == NULL)
        return NULL;
    while(plist != NULL)
    {
        if(plist->data == data)
            return plist;
        plist = plist->nd;
    }
    return NULL;
}
/* 删除指定节点 */
_list* list_delete_nd(_list* plist ,int data)
{
    if(plist == NULL)
        return NULL;
    /* 删除头节点 */
    if(plist->data == data){
        /* 只有一个头节点 */
        if(plist->nd == NULL){
            free(plist);
            return NULL;
        }
        else{
            _list *p = plist;
            plist = plist->nd;
            free(p);
        }
    }
    else{
        _list *pr = plist;
        _list *p = pr->nd;
        while(p != NULL){
            if(p->data == data){
                pr->nd = p->nd;
                free(p);
                break;
            }
            pr = p;
            p = pr->nd;
        }
    }
    return plist;
} 
/* 打印链表 */
void list_print(_list* plist)
{
    if(plist == NULL)
        return;
    while(plist != NULL)
    {
        printf("%d ", plist->data);
        plist = plist->nd;
    }
    printf("\n");
}
