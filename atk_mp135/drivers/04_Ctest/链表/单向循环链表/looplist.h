#ifndef __LOOPLIST_H
#define __LOOPLIST_H

struct _list_head{
    struct _list_head *next;
};
typedef struct _list_head _list_h;

struct __list{
    int data;
    _list_h list_h;
};
typedef struct __list _list;

#define GET_LIST_NODE(ptr) \
                                (\
                                    (_list*) \
                                    ((char*)(ptr) - offsetof(_list, list_h))\
                                )
_list* list_init(int data);
void list_add_nd(_list *list ,int data);
_list* list_find_nd(_list *list ,int data);
_list* list_delete_nd(_list *list ,int data);
void list_free(_list* list);
void list_print(_list* list);

#endif
