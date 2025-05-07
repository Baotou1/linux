#include "list.h"

/**
 * @brief     释放所有节点(不包含头节点)
 * @param[in] param1  链表头节点
 * @return    无
 */
void list_free(struct __list_node *head){
    if(!head){
        return;
    }

    /* 跳过头节点 */
    if((head->nd.next == &head->nd) && (head->nd.prev == &head->nd)){
        return;
    }

    struct __list_head *p = head->nd.next;
    while(p != &head->nd){
        struct __list_head *pn = p->next;
        struct __list_node *nd = GET_LIST_NODE(p);
        free(nd);
        p = pn;
    }
    head->nd.next = &head->nd;
    head->nd.prev = &head->nd;
}
/**
 * @brief     链表中增加节点
 * @param[in] param1  链表头节点
 * @param[in] param2  节点名字
 * @return    无
 */
void list_add_nd(struct __list_node *head ,char *name){
    if((!head) || (!name)){
        return;
    }

    struct  __list_node *nd = (struct __list_node*)malloc(sizeof(struct __list_node));
    if(!nd){
        return;
    }    
    nd->name = name;
    
    struct __list_head *tail = head->nd.prev;

    nd->nd.prev = tail;
    nd->nd.next = &head->nd;
    tail->next = &nd->nd;
    head->nd.prev = &nd->nd;
}

/**
 * @brief     查找链表中指定节点(不包含头节点,以防头节点被修改)
 * @param[in] param1  链表头节点
 * @param[in] param2  节点名字
 * @return    无
 */
struct __list_node* list_find_nd(struct __list_node *head ,char *name){
    if((!head) || (!name)){
        return NULL;
    }

    struct __list_head *p = head->nd.next;
    while(p != &head->nd){
        struct __list_node *nd = GET_LIST_NODE(p);
        if(strcmp(nd->name ,name) == 0){
            return nd;
        }
        p = p->next;
    }
    return NULL;
}
/**
 * @brief     删除链表中指定节点(不包含头节点)
 * @param[in] param1  链表头节点
 * @param[in] param2  节点名字
 * @return    无
 */
void list_delete_nd(struct __list_node *head ,char *name){
    if((!head) || (!name)){
        return;
    }

    if(strcmp(head->name ,name) == 0){
        return;
    }

    struct __list_head *p = head->nd.next;
    while(p != &head->nd){
        struct __list_node *nd = GET_LIST_NODE(p);

        if(strcmp(nd->name ,name) == 0){
            p->next->prev = p->prev;
            p->prev->next = p->next;
            free(nd);
            return;
        }
        p = p->next;
    }
}
/**
 * @brief     获取链表长度(不包含头节点)
 * @param[in] param1  链表头节点
 * @return    长度
 */
int list_length(struct __list_node *head){
    if(!head){
        return -1;
    }

    struct __list_head *p = head->nd.next;
    int len = 0;
    while(p != &head->nd){
        len++;
        p = p->next;
    }
    return len;
}
/**
 * @brief     反转链表(不包含头节点)
 * @param[in] param1  链表头节点
 * @return    长度
 */
/**
 * @brief     打印节点
 * @param[in] param1  链表头节点
 * @return    无
 * @note      无
 */
void list_print_nd(struct __list_node *head){
    if(!head){
        return;
    }

    struct __list_head *p = &head->nd;
    do{
        struct __list_node *nd = GET_LIST_NODE(p);
        printf("%s -> " ,nd->name);
        p = p->next;
    }while(p != &head->nd);
    printf("head\n");
}