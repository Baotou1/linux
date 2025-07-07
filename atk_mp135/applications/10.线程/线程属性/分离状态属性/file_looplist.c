/**
 * @file    file_looplist.c
 * @brief   文件循环链表管理实现
 *
 * 本文件实现基于单向循环链表的数据结构，用于管理文件资源节点的增删查找和释放。
 * 主要功能包括：
 *  - 初始化空链表节点 (__file_list_init)
 *  - 向链表尾部添加文件节点 (__file_list_add_nd)
 *  - 查找指定路径的文件节点 (__file_list_find_nd)
 *  - 删除指定路径的文件节点 (__file_list_delete_nd)
 *  - 释放整个文件链表及所有节点资源 (__file_list_free)
 *
 * 链表节点采用单向循环链表结构，头节点通过 __index == LIST_HEAD 标识。
 * 节点内含文件指针，删除节点时会自动关闭文件资源，防止资源泄露。
 *
 * 设计要点：
 *  - 支持自动查找真实链表头节点，保证链表操作的正确性。
 *  - 所有操作均保持链表循环结构的完整。
 *  - 释放操作确保链表内所有节点及文件资源均被正确释放，防止内存泄漏。
 *
 * 适用场景：
 *  - 需要动态管理一组文件资源，支持高效插入、删除及查找操作。
 *  - 文件资源管理、缓存机制等。
 *
 * @author  
 * @date    
 */
#include "file_looplist.h"

/**
 * @func   __file_list_init
 * @brief  初始化一个空的文件循环链表节点
 *
 * @retval __flist_t* 返回初始化后的循环链表节点指针，失败返回 NULL
 *
 * @details
 *  创建一个新的文件循环链表节点，节点中保存的文件指针初始化为 NULL。
 *  节点的链表头 __list_h 的 __next 指针指向自身，形成单节点的循环链表结构。
 *  该节点可用于构建文件管理的循环链表。
 */
__flist_t *__file_list_init(void)
{
    /* 分配内存空间，清零 */
    __flist_t *__pl = (__flist_t *)calloc(1 ,sizeof(__flist_t));
    if(__pl == NULL)
        return NULL;

    /* 设置节点索引，标识为链表头 */
    __pl->__index = LIST_HEAD;

    /* 初始化链表头的 __next 指针指向自身，形成单节点循环链表 */
    __pl->__list_h.__next = &__pl->__list_h; 

    /* 返回新创建的链表节点指针 */
    return __pl;
}

/**
 * @func   __file_list_free
 * @brief  释放文件循环链表所有节点内存
 *
 * @param[in,out] __pl 指向链表头指针的地址（__flist_t **）
 *
 * @details
 *  遍历并释放链表中所有节点，包括头节点，释放完后将 *__pl 设置为 NULL。
 */
void __file_list_free(__flist_t **__pl)
{
    /* 参数校验，指针为空则直接返回 */
    if(__pl == NULL || (*__pl) == NULL)
        return;

    __flist_t *__head = (*__pl);
    _list_h *__h = &__head->__list_h;
    /* 若当前节点不是头节点，查找真正的头节点 */
    if(__head->__index != LIST_HEAD)
    {
        FLIST_FIND_HEAD(__h, __head);
    }

    _list_h *__p = __h->__next;

    /* 遍历直到回到头节点，释放所有非头节点 */
    while(__p != __h)
    {
        /* 保存当前节点的下一个节点指针，防止释放后丢失 */
        _list_h *__next = __p->__next;
        __flist_t *__nd = GET_FLIST_NODE(__p);

        /* 关闭节点内的文件资源 */
        _file_close(__nd->__pf);
        free(__nd);

        /* 移动到下一个节点 */
        __p = __next;
    }

    /* 释放头节点持有的文件资源 */
    _file_close(__head->__pf);
    free(__head);

    /* 置空外部指针，避免悬空 */
    (*__pl) = NULL;
}


/**
 * @func   __file_list_add_nd
 * @brief  在文件循环链表尾部添加一个新节点
 *
 * @param[in] __pl  指向文件循环链表头节点的指针 (__flist_t *)
 * @param[in] __pf  指向要添加的文件对象 (_file_t *)
 *
 * @retval 0    添加成功
 * @retval -1   参数非法或内存分配失败
 *
 * @details
 *  该函数在已有的文件循环链表中创建一个新的节点，并将该节点插入到链表尾部，
 *  保持链表的循环结构不变。
 *
 *  操作步骤：
 *    1. 检查输入参数有效性。
 *    2. 查找真正的链表头（如果当前节点不是头节点）。
 *    3. 分配新节点内存，并初始化。
 *    4. 将新节点插入到尾部，使链表保持循环。
 *
 * @note
 *  - 假设传入的链表是单向循环链表。
 *  - 新节点分配失败时不修改链表。
 */
int __file_list_add_nd(__flist_t *__pl, _file_t *__pf)
{
    if (__pl == NULL || __pf == NULL)
        return -1;

    /* 获取链表头的链表结构指针 */
    _list_h *__h = &(__pl->__list_h);
    __flist_t *__nd = __pl;

    /* 如果当前节点不是头节点，找到真正的头节点 */
    if (__pl->__index != LIST_HEAD)
    {
        FLIST_FIND_HEAD(__h, __nd);
    }

    /* 分配新节点 */
    __flist_t *__new_nd = (__flist_t *)calloc(1, sizeof(__flist_t));
    if (__new_nd == NULL)
        return -1;

    /* 绑定文件指针 */
    __new_nd->__pf = __pf;
    /* 新节点的 next 指向头节点，实现循环 */
    __new_nd->__list_h.__next = __h;

    /* 找到尾节点：尾节点的 next 指向头节点 */
    _list_h *__p = __h;
    while (__p->__next != __h)
    {
        __p = __p->__next;
    }

    /* 将尾节点的 next 指向新节点，完成插入 */
    __p->__next = &__new_nd->__list_h;

    return 0;
}

/**
 * @func   __file_list_find_nd
 * @brief  在文件循环链表中查找指定路径的文件节点
 *
 * @param[in,out] __pl       双重指针，传入链表头，若找到则修改为目标节点指针
 * @param[in]     __pathname 要查找的目标路径字符串
 *
 * @retval 0   找到匹配节点，*__pl 被更新为对应节点
 * @retval -1  未找到匹配节点或参数无效
 *
 * @note
 *  - 该函数搜索循环链表中第一个路径匹配的节点。
 *  - 查找成功后更新 *__pl 指向该节点。
 *  - 未找到时，*__pl 保持不变。
 */
int __file_list_find_nd(__flist_t **__pl ,const char *__pathname)
{
    if(__pl == NULL || (*__pl) == NULL || __pathname == NULL)
        return -1;

    __flist_t *__head = (*__pl);
    _list_h *__h = &__head->__list_h;
    /* 若当前节点不是头节点，查找真正的头节点 */
    if(__head->__index != LIST_HEAD)
    {
        FLIST_FIND_HEAD(__h, __head);
    }

    _list_h *__p = __h;
    do
    {
        __flist_t *__nd = GET_FLIST_NODE(__p);
        if(__nd->__pf && __nd->__pf->__pathname 
                                    && strcmp(__nd->__pf->__pathname ,__pathname) == 0)
        {
            (*__pl) = __nd;
            return 0;
        }
        __p = __p->__next;
    }while(__p != __h);

    return -1;
}

/**
 * @func   __file_list_delete_nd
 * @brief  从文件循环链表中删除指定路径的节点
 *
 * @param[in,out] __pl        指向链表头节点的指针地址 (__flist_t **)
 * @param[in]      __pathname 要删除的目标路径字符串
 *
 * @retval  0   删除成功
 * @retval -1   删除失败（参数无效或路径未匹配）
 *
 * @details
 *  本函数用于从文件循环链表中移除指定路径对应的节点。支持以下功能：
 *   - 自动查找真实的头节点（__index == LIST_HEAD）
 *   - 删除链表中任意位置的节点（头节点或中间节点）
 *   - 删除头节点时自动更新新的头节点
 *   - 删除节点时调用 _file_close() 关闭文件资源
 *   - 删除后释放节点内存，避免内存泄漏
 *
 * 处理逻辑包括：
 *   1. 校验参数合法性；
 *   2. 查找真正的头节点；
 *   3. 判断是否为单节点链表；
 *   4. 若为头节点匹配则删除并更新；
 *   5. 否则遍历链表查找并删除匹配节点；
 */
int __file_list_delete_nd(__flist_t **__pl ,const char *__pathname)
{
    if(__pl == NULL || (*__pl) == NULL || __pathname == NULL)
        return -1;

    __flist_t *__head = (*__pl);
    _list_h *__h = &__head->__list_h;

    /* 若当前节点不是头节点，查找真正的头节点 */
    if(__head->__index != LIST_HEAD)
    {
        FLIST_FIND_HEAD(__h, __head);
    }
    
    _list_h *__p = __h->__next;

    /* 情况1：链表仅有一个节点（自环） */
    if(__p == __h) {
        if(__head->__pf && __head->__pf->__pathname &&
                                            strcmp(__head->__pf->__pathname, __pathname) == 0)
        {
            _file_close(__head->__pf);  /* 关闭文件描述符 */
            free(__head);               /* 释放节点内存 */
            (*__pl) = NULL;             /* 清空头指针 */
            return 0;
        }
        return -1;                      /* 单节点不匹配路径，删除失败 */
    }

    /* 情况2：头节点匹配路径 */
    if(__head->__pf && __head->__pf->__pathname &&
                                            strcmp(__head->__pf->__pathname, __pathname) == 0)
    {
        /* 查找尾节点 */
        while(__p->__next != __h)
        {
            __p = __p->__next;
        }

        _list_h *__tail = __p;
        __tail->__next = __h->__next;                /* 跳过原头节点 */

        _file_close(__head->__pf);
        free(__head);

        (*__pl) = GET_FLIST_NODE(__tail->__next); /* 设置新头 */
        (*__pl)->__index = LIST_HEAD;                 /* 标记为头节点 */
        return 0;
    }
    
    /* 情况3：删除中间节点 */
    _list_h *__prev = __h;                           /* 标记__p节点的上一个节点 */
    while(__p != __h) 
    {
        __flist_t *__nd = GET_FLIST_NODE(__p);
        if(__nd->__pf && __nd->__pf->__pathname &&
                                            strcmp(__nd->__pf->__pathname, __pathname) == 0)
        {
            __prev->__next = __p->__next;           /* 断链 */
            _file_close(__nd->__pf);
            free(__nd);
            return 0;
        }
        __prev = __p;
        __p = __p->__next;
    }

    /* 情况4：未找到节点 */
    return -1;
}