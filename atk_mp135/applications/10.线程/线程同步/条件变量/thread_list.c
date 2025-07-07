/**
 * @file    thread_list.c
 * @brief   线程双向循环链表管理实现
 *
 * 本文件实现基于双向循环链表的数据结构，用于管理线程节点的增删查操作。
 * 线程链表支持动态添加线程节点、根据线程名称查找节点、
 * 以及删除指定名称的线程节点。链表采用循环双向链表结构，
 * 头节点使用特殊标识 __index == LIST_HEAD。
 *
 * 主要功能函数：
 *  - __thd_list_init       : 初始化线程链表，返回头节点
 *  - __thd_list_free       : 释放整个线程链表及所有节点
 *  - __thd_list_add_nd     : 向链表尾部添加线程节点
 *  - __thd_list_find_nd    : 根据线程名称查找节点，返回节点指针
 *  - __thd_list_delete_nd  : 删除指定名称线程节点，更新头节点指针
 *
 * 数据结构说明：
 *  - __tlist_t             : 线程链表节点结构体，包含线程数据指针和链表指针
 *  - _dlist_h              : 双向链表节点头结构，维护前后指针
 *
 * 注意事项：
 *  - 调用删除函数后，若链表为空，头节点指针被设置为 NULL
 *  - 线程名称必须唯一且非空
 *  - 释放节点时需确保线程数据资源正确释放，避免内存泄漏
 *
 * 依赖：
 *  - 需包含 "thread_list.h"、链表辅助宏和相关线程结构体定义
 */
#include "thread_list.h"

/**
 * @func   __thd_list_init
 * @brief  初始化线程链表控制结构
 *
 * @retval __tlist_t*  成功返回链表控制结构体指针
 * @retval NULL        内存分配失败
 *
 * @details
 *  - 动态分配并初始化一个线程链表控制结构体 (__tlist_t)
 *  - 设置 __index 为 LIST_HEAD，表示该节点为链表头
 *  - 初始化双向循环链表头结点的前后指针指向自身，表示空链表
 */
__tlist_t *__thd_list_init(void)
{
    /* 分配链表控制结构体内存 */
    __tlist_t *__pl = (__tlist_t *)calloc(1 ,sizeof(__tlist_t));
    if(__pl == NULL)
        return NULL;

    /* 设置链表索引，标识为链表头 */
    __pl->__index = LIST_HEAD;

    /* 初始化链表头节点，使其指针循环指向自身，表示空链表 */
    __pl->__dlist_h.__next = &__pl->__dlist_h;
    __pl->__dlist_h.__prev = &__pl->__dlist_h;

    return __pl;
}

/**
 * @func   __thd_list_free
 * @brief  释放线程链表及其所有节点的内存
 *
 * @param[in,out] __pl  指向线程链表头指针的地址，释放后置为 NULL
 *
 * @details
 *  该函数用于释放整个线程循环链表的所有内存资源，包括：
 *    - 每个节点中的线程结构体（__pthd）通过 __thread_free 安全释放；
 *    - 节点本身通过 free 释放；
 *    - 最后将链表头指针置为 NULL，避免悬空引用。
 *
 *  支持以下场景：
 *    1. 若链表为空（*__pl 为 NULL），函数直接返回；
 *    2. 若链表仅有一个节点（自环），释放该节点及其线程资源；
 *    3. 若为非头节点或中间节点，使用 TLIST_FIND_HEAD 查找真正的头节点；
 *    4. 遍历并释放所有非头节点；
 *    5. 最后释放头节点并将 *__pl 设置为 NULL。
 */
void __thd_list_free(__tlist_t **__pl)
{
    /* 参数校验：链表指针或指向内容为空直接返回 */
    if(__pl == NULL || (*__pl) == NULL)
        return;

    _dlist_h *__h = &(*__pl)->__dlist_h;

    /* 如果链表只有一个节点（next == prev），直接释放头节点 */
    if(__h->__next == __h->__prev)
    {
        __thread_free(&(*__pl)->__pthd);
        free((*__pl));
        (*__pl) = NULL;
    }
    else
    {
        __tlist_t *__head = (*__pl);
        /* 如果当前节点不是链表头，查找真正的头节点 */
        if(__head->__index != LIST_HEAD)
        {
            TLIST_FIND_HEAD(__h ,__head);
        }

        /* 循环释放所有非头节点 */
        while(__h->__next != __h->__prev)
        {
            _dlist_h *__p = __h->__next;
            /* 取链表第一个节点 */
            __tlist_t *__nd = GET_TLIST_NODE(__p); 
            
            /* 从链表中摘除节点 */
            __p->__next->__prev = __h;
            __h->__next = __p->__next;

            /* 释放节点内存 */
            __thread_free(&__nd->__pthd);
            free(__nd);
        }

        /* 最后释放头节点 */
        __thread_free(&__head->__pthd);
        free(__head);
        (*__pl) = NULL;
    }
}

/**
 * @func    __thd_list_add_nd
 * @brief   向线程双向循环链表尾部添加一个新的线程节点
 *
 * @param[in,out] __pl    指向线程链表头节点（__tlist_t *）
 * @param[in]     __pthd  指向要添加的线程对象指针（__thd_t *）
 *
 * @retval 0   添加成功
 * @retval -1  添加失败（参数非法或内存分配失败）
 *
 * @details
 *  该函数将在已存在的线程双向循环链表中添加一个新节点，保持链表循环结构：
 *    - 自动查找链表头节点（通过 __index == LIST_HEAD 标识）
 *    - 在链表尾部插入新节点
 *    - 更新链表尾节点和头节点的指针连接
 *    - 新节点绑定传入的线程对象
 */
int __thd_list_add_nd(__tlist_t *__pl,__thd_t *__pthd)
{
    /* 参数合法性检查 */
    if(__pl == NULL || __pthd == NULL)
        return -1;

    /* 获取链表头节点的双向链表头结构 */
    __tlist_t *__head = __pl;
    _dlist_h *__h = &(__head->__dlist_h);

    /* 如果当前节点不是链表头，查找真正的头节点 */
    if(__head->__index != LIST_HEAD)
    {
        TLIST_FIND_HEAD(__h,__head);
    }

    /* 分配新节点内存 */
    __tlist_t *__new_nd = (__tlist_t *)calloc(1 ,sizeof(__tlist_t));
    if(__new_nd == NULL)
        return -1;

    /* 绑定线程数据 */
    __new_nd->__pthd = __pthd;
    /* 获取链表尾节点 */
    _dlist_h *__t = __h->__prev;

    /* 设置新节点的前驱和后继指针，插入到头节点之前 */
    __new_nd->__dlist_h.__next = __h;
    __new_nd->__dlist_h.__prev = __t;

    /* 修改尾节点和头节点的指针，保持链表循环结构 */
    __h->__prev = &(__new_nd->__dlist_h);
    __t->__next = &(__new_nd->__dlist_h);

    return 0;
}

/**
 * @func    __thd_list_find_nd
 * @brief   在线程双向循环链表中查找指定名称的线程节点
 *
 * @param[in,out] __pl    指向链表中任意节点指针的地址（__tlist_t **）
 *                        成功时会被更新为目标节点地址
 * @param[in]     __name  要查找的线程名称（不能为空）
 *
 * @retval 0   找到匹配的线程节点，*__pl 被更新为目标节点
 * @retval -1  参数无效或未找到匹配节点
 *
 * @details
 *  该函数在以 *__pl 所在链表为基础的循环链表中查找名称匹配的线程节点：
 *    - 若当前节点不是头节点，通过 TLIST_FIND_HEAD 宏查找真正的头节点；
 *    - 从头节点起，遍历链表查找线程名称字段 (__name)；
 *    - 如果找到，更新 *__pl 为该节点地址，并返回 0；
 *    - 如果遍历结束未找到匹配节点，返回 -1。
 *
 * @note
 *  - 传入的 __pl 不仅用于提供起始遍历位置，也用于输出匹配节点；
 *  - 不会修改链表结构，仅更新指针；
 *  - __name 必须非空，且链表节点中对应 __pthd->__name 也应合法。
 */
int __thd_list_find_nd(__tlist_t **__pl, const char *__name)
{
    if(__pl == NULL || (*__pl) == NULL || __name == NULL)
        return -1;

    __tlist_t *__head = (*__pl);
    _dlist_h *__h = &(__head->__dlist_h);

    /* 若当前节点不是头节点，查找真正的头节点 */
    if(__head->__index != LIST_HEAD)
    {
        TLIST_FIND_HEAD(__h, __head);
    }

    _dlist_h *__p = __h;
    do
    {
        __tlist_t *__nd = GET_TLIST_NODE(__p);
        if(strcmp(__nd->__pthd->__name ,__name) == 0)
        {
            (*__pl) = __nd;
            return 0;
        }
        __p = __p->__next;
    }while(__p != __h);

    return -1;
}

/**
 * @func    __thd_list_delete_nd
 * @brief   从线程循环链表中删除指定名称的线程节点
 *
 * @param[in,out] __pl    指向线程链表头节点指针的地址（__tlist_t **）
 *                        删除成功后，若链表为空，则将 *__pl 置为 NULL
 * @param[in]     __name  要删除的线程名称（不能为空）
 *
 * @retval 0   删除成功
 * @retval -1  删除失败（如参数无效、名称未匹配等）
 *
 * @details
 *  本函数在线程循环链表中查找名称为 __name 的节点并将其删除，逻辑包括：
 *    - 若传入参数无效，直接返回 -1；
 *    - 若当前节点不是头节点，通过 TLIST_FIND_HEAD 查找真正的头节点；
 *    - 若链表仅包含一个节点，判断名称是否匹配，匹配则释放节点并置 NULL；
 *    - 若头节点匹配，则将其后的节点设为新头节点，更新链表指针并释放原头节点；
 *    - 否则遍历查找中间节点，匹配名称后摘除节点并释放资源；
 *    - 若最终未找到匹配节点，返回 -1。
 *
 *  删除时将释放：
 *    - 节点内部的线程结构体（__pthd），使用 __thread_free；
 *    - 节点本身内存；
 *    - 若删除的是头节点，还将重设链表头指针（*__pl）。
 */
int __thd_list_delete_nd(__tlist_t **__pl, const char *__name)
{
    if(__pl == NULL || (*__pl) == NULL || __name == NULL)
        return -1;

    __tlist_t *__head = (*__pl);
    _dlist_h *__h = &(__head->__dlist_h);

    /* 若当前节点不是头节点，查找真正的头节点 */
    if(__head->__index != LIST_HEAD)
    {
        TLIST_FIND_HEAD(__h, __head);
    }

    /* 情况1：链表仅有一个节点（自环） */
    if(__head->__dlist_h.__next == &(__head->__dlist_h))
    {
        if(__head->__pthd && strcmp(__head->__pthd->__name, __name) == 0)
        {
            __thread_free(&__head->__pthd);
            free(__head);
            (*__pl) = NULL;
            return 0;
        }
        return -1;  /* 唯一节点但名字不匹配，返回失败 */
    }

    /* 情况2：头节点匹配路径 */
    if(__head->__pthd && strcmp(__head->__pthd->__name, __name) == 0)
    {
        /* 修改指针，断开当前节点 */
        __h->__prev->__next = __h->__next;
        __h->__next->__prev = __h->__prev;

        /* 设置下一个节点为新的头节点 */
        __h = __h->__next;
        __tlist_t *__new_head = GET_TLIST_NODE(__h);
        __new_head->__index = LIST_HEAD;

        __thread_free(&__head->__pthd);
        free(__head);
        /* 更新头指针 */
        (*__pl) = __new_head;
        return 0;
    }

    /* 情况3：删除中间节点 */
    _dlist_h *__p = __h;
    /* 遍历链表，查找匹配名称的普通节点 */
    do
    {
        __tlist_t *__nd = GET_TLIST_NODE(__p);
        if(__nd->__pthd && strcmp(__nd->__pthd->__name, __name) == 0)
        {
            /* 断开并释放该节点 */
            __p->__prev->__next = __p->__next;
            __p->__next->__prev = __p->__prev;

            __thread_free(&__nd->__pthd);
            free(__nd);
            return 0;
        }
        __p = __p->__next;
    }while(__p != __h);

    /* 情况4：未找到节点 */
    return -1;  
}

