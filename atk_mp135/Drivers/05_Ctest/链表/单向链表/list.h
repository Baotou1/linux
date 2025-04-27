#ifndef __LIST_H
#define __LIST_H

struct _list_t
{
    int data;
    struct _list_t *nd;
    
};
typedef struct _list_t _list;

_list* list_init_head(int data);
void list_add_nd(_list* plist1 ,int data);
_list* list_add_list(_list* plist1 ,_list* plist2);
_list* list_find_nd(_list* plist ,int data);
_list* list_delete_nd(_list* plist ,int data);
void list_free(_list* plist);
void list_print(_list* plist);

#endif