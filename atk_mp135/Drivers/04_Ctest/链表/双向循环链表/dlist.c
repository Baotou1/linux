#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "dlist.h"

_dlist* dlist_init(int data){
    _dlist *dlist = (_dlist*)malloc(sizeof(_dlist));
    if(!dlist) return NULL;
    dlist->data = data;
    dlist->dlist_h.next = &dlist->dlist_h;
    dlist->dlist_h.prev = &dlist->dlist_h;
    return dlist;
}

void dlist_add_nd(_dlist* dlist ,int data){
    if(!dlist) return;

    _dlist *nd = (_dlist*)malloc(sizeof(_dlist));
    if(!nd) return;

    _dlist_h *pp = dlist->dlist_h.prev;
    
    nd->data = data;
    pp->next = &nd->dlist_h;
    nd->dlist_h.prev = pp;
    nd->dlist_h.next = &dlist->dlist_h;
    dlist->dlist_h.prev = &nd->dlist_h;
}

_dlist* dlist_find_nd(_dlist* dlist ,int data){
    if(!dlist) return NULL;

    _dlist_h* p = &dlist->dlist_h;
    do
    {
        _dlist *nd = GET_LIST_NODE(p);
        if(nd->data == data)
        {
            return nd;
        }
        p = p->next;
    }while (p != &dlist->dlist_h);
    return NULL;
}

void dlist_delete_nd(_dlist* dlist ,int data){
    if(!dlist) return;

    _dlist_h *p = dlist->dlist_h.next;
    /* 头节点 */
    if(dlist->data == data){
        return;
    }
    do{
        _dlist *nd = GET_LIST_NODE(p);
        if(nd->data == data)
        {
            p->prev->next = p->next;
            p->next->prev = p->prev;
            free(nd);
            return;
        }
        p = p->next;
    }while(p != &dlist->dlist_h);
}

void dlist_free(_dlist *dlist){
    if(!dlist) return;
    if((dlist->dlist_h.next == &dlist->dlist_h) && (dlist->dlist_h.prev == &dlist->dlist_h)){
        free(dlist);
        return;
    } 
    _dlist_h *p = dlist->dlist_h.next; /* 从第一个节点开始，跳过头节点 */
    while(p != &dlist->dlist_h){
        _dlist_h *pn = p->next;
        _dlist_h *pp = p->prev;
        pp->next = pn;
        pn->prev = pp;
        _dlist *nd = GET_LIST_NODE(p);
        free(nd);
        p = pn;
    }
    free(dlist);
}

void dlist_print(_dlist *dlist){
    if(!dlist) return;
    _dlist_h *p = &dlist->dlist_h;
    do
    {
        _dlist *nd = GET_LIST_NODE(p);
        printf("%d " ,nd->data);
        p = p->next;
    } while (p != &dlist->dlist_h);
    printf("\n");
}