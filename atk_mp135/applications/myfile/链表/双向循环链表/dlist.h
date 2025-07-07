#ifndef __DLIST_H
#define __DLIST_H

struct __dlist_head{
    struct __dlist_head *next ,*prev;
};
typedef struct __dlist_head _dlist_h;

struct __dlist{
    int data;
    _dlist_h dlist_h;
};
typedef struct __dlist _dlist;

#define GET_LIST_NODE(ptr) \
                                (\
                                    (_dlist*) \
                                    ((char*)(ptr) - offsetof(_dlist, dlist_h))\
                                )
_dlist* dlist_init(int data);
void dlist_add_nd(_dlist* dlist ,int data);
_dlist* dlist_find_nd(_dlist* dlist ,int data);
void dlist_delete_nd(_dlist* dlist ,int data);
void dlist_free(_dlist *dlist);
void dlist_print(_dlist *dlist);
#endif