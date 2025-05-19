#ifndef __LIST_H
#define __LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
/* 双向链表 */
struct __list_head{
    struct __list_head *next ,*prev;
};

/* 双向链表节点 */
struct __list_node{ 
    char *name;
    struct __list_head nd;
};

#define __LIST_INITIALIZER(str ,node) { \
    .name = str,                        \
    .nd = {&(node.nd) ,&(node.nd)}      \
}

/* 创建一个链表头 */
#define DECLARE_LIST_HEAD(str ,node) \
        struct __list_node node = __LIST_INITIALIZER(str ,node)

#define GET_LIST_NODE(p) \
        ((struct __list_node *)((char *)(p) - offsetof(struct __list_node, nd)))

void list_free(struct __list_node *head);
void list_add_nd(struct __list_node *head ,char *name);
struct __list_node* list_find_nd(struct __list_node *head ,char *name);
void list_delete_nd(struct __list_node *head ,char *name);
int list_length(struct __list_node *head);
void list_print_nd(struct __list_node *head);
#endif
