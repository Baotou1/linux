#include "file_looplist.h"

/**
 * @func   __file_list_init
 * @brief  初始化一个空的文件循环链表节点
 *
 * @retval __flist_t* 返回初始化后的循环链表节点指针，失败返回 NULL
 *
 * @details
 *  创建一个新的文件循环链表节点，节点中保存的文件指针初始化为 NULL。
 *  节点的链表头 __list_h 的 next 指针指向自身，形成单节点的循环链表结构。
 *  该节点可用于构建文件管理的循环链表。
 */
__flist_t *__file_list_init(void)
{
    __flist_t *__pfl = (__flist_t *)calloc(1 ,sizeof(__flist_t));
    if(__pfl == NULL)
        return NULL;

    __pfl->__pf = NULL;
    __pfl->__list_h.next = &__pfl->__list_h; 
    __pfl->__index = LIST_HEAD;
    return __pfl;
}

/**
 * @func   __file_list_free
 * @brief  释放文件循环链表所有节点内存
 *
 * @param[in,out] __pfl 指向链表头指针的地址（__flist_t **）
 *
 * @details
 *  遍历并释放链表中所有节点，包括头节点，释放完后将 *__pfl 设置为 NULL。
 */
void __file_list_free(__flist_t **__pfl)
{
    if(__pfl == NULL || (*__pfl) == NULL)
        return;

    _list_h *__h = &(*__pfl)->__list_h;
    _list_h *__p = __h->next;
    while(__p != __h)
    {
        _list_h *__next = __p->next;
        __flist_t *__nd = GET_FILE_LIST_NODE(__p);

        /* 释放文件的结构体 */
        _file_close(__nd->__pf);
        free(__nd);
        __p = __next;
    }

    _file_close((*__pfl)->__pf);
    free((*__pfl));
    (*__pfl) = NULL;
}

/**
 * @func   __file_list_add_nd
 * @brief  在文件循环链表尾部添加一个新节点
 *
 * @param[in] __pfl  指向文件循环链表头节点的指针 (__flist_t *)
 * @param[in] __pf   指向要添加的文件对象 (_file_t *)
 *
 * @retval int  返回操作结果，成功返回 0，失败返回 -1
 *
 * @details
 *  该函数在已有的文件循环链表中创建一个新的节点，并将该节点插入到链表尾部，
 *  保持链表的循环结构不变。
 *  
 *  操作步骤：
 *    1. 检查输入参数有效性，若头指针或文件指针为空，则返回失败。
 *    2. 为新节点分配内存并初始化，将文件指针赋值给新节点。
 *    3. 将新节点的链表头的 next 指针指向头节点的链表头，实现循环。
 *    4. 遍历链表找到最后一个节点（即其 next 指向头节点的节点）。
 *    5. 将最后一个节点的 next 指向新节点，实现尾部插入。
 *
 * @note
 *  - 新节点分配失败时函数直接返回失败，不做任何修改。
 *  - 函数假设传入的 __pfl 已经是合法的循环链表头。
 */
int __file_list_add_nd(__flist_t *__pfl ,_file_t *__pf)
{
    if(__pfl == NULL || __pf == NULL)
        return -1;

    __flist_t *__newpfl = (__flist_t *)calloc(1 ,sizeof(__flist_t));
    if(__newpfl == NULL)
        return -1;
    
    __newpfl->__pf = __pf;
    __newpfl->__list_h.next = &__pfl->__list_h;

    _list_h *__h = &__pfl->__list_h;
    _list_h *__p = __h->next;
    while(__p->next != __h)
    {
        __p = __p->next;
    }
    __p->next = &__newpfl->__list_h; 
    return 0;
}

/**
 * @func   __file_list_find_nd
 * @brief  在文件循环链表中查找指定路径的文件节点
 *
 * @param[in,out] __pfl      双重指针，传入链表头，若找到则指向目标节点
 * @param[in]     __pathname 要查找的目标路径字符串
 *
 * @retval  0 找到匹配的节点，*__pfl 被修改为对应节点
 * @retval -1 未找到匹配节点或参数无效
 *
 * @note
 *  该函数在循环链表中搜索指定路径对应的文件节点，
 *  若匹配成功，则通过双重指针将调用者的 __pfl 更新为目标节点。
 *  未找到时，__pfl 保持原值不变。
 */
int __file_list_find_nd(__flist_t **__pfl ,const char *__pathname)
{
    if(__pfl == NULL || (*__pfl) == NULL || __pathname == NULL)
        return -1;

    _list_h *__h = &(*__pfl)->__list_h;
    _list_h *__p = __h->next;
    while(__p != __h)
    {
        __flist_t *__nd = GET_FILE_LIST_NODE(__p);
        if(__nd->__pf && __nd->__pf->__pathname 
                                    && strcmp(__nd->__pf->__pathname ,__pathname) == 0)
        {
            (*__pfl) = __nd;
            return 0;
        }
        __p = __p->next;
    }

    return -1;
}

/**
 * @func   __file_list_delete_nd
 * @brief  从文件循环链表中删除指定路径对应的节点
 *
 * @param[in,out] __pfl       指向链表头节点指针的地址（__flist_t **）
 * @param[in]      __pathname 需要删除的文件路径字符串
 *
 * @retval 0  成功删除对应节点
 * @retval -1 删除失败（参数为空、找不到路径或其他异常）
 *
 * @details
 *  该函数用于从文件循环链表中删除一个指定路径的文件节点，支持：
 *    - 删除链表中任意位置的节点（包括头节点）
 *    - 自动识别头节点（根据 __index == LIST_HEAD 标记）
 *    - 自动更新链表头指针（如删除的是当前头节点）
 *    - 释放对应节点内存并关闭文件描述
 *  
 *  函数执行流程如下：
 *    1. 校验传入参数是否有效。
 *    2. 遍历链表找出头节点（__index == LIST_HEAD），更新 *__pfl 指向它。
 *    3. 判断是否为单节点链表，若匹配路径则直接删除并置 NULL。
 *    4. 判断是否为头节点匹配路径，若是则更新尾节点指向新的头节点，并删除原头。
 *    5. 否则，从链表中部遍历寻找目标路径，若匹配则将其从链表中断开并释放。
 *
 * @note
 *  - 若链表中只有一个节点，匹配失败将返回 -1，不会删除。
 *  - 删除头节点时，会自动更新新的头节点（即原来的 next 节点）。
 *  - 每个节点的 __pf->__pathname 不可为 NULL，需预先赋值。
 *  - 删除前会调用 _file_close() 关闭文件，防止资源泄漏。
 */
int __file_list_delete_nd(__flist_t **__pfl ,const char *__pathname)
{
    if(__pfl == NULL || (*__pfl) == NULL || __pathname == NULL)
        return -1;

    /* 找到头节点 */
    _list_h *__h = &(*__pfl)->__list_h;
    while(1)
    {
        __flist_t *__h_nd = GET_FILE_LIST_NODE(__h);
        if(__h_nd->__index == LIST_HEAD)
        {
            (*__pfl) = __h_nd;
            break;
        }
        __h = __h->next;

        /* 防止错误修改__index的值导致死循环（找不到头节点） */
        if (__h == &(*__pfl)->__list_h) 
            return -1;
    }

    _list_h *__p = __h->next;

    /* 只有一个节点 */
    if(__p == __h)
    {
        if((*__pfl)->__pf && (*__pfl)->__pf->__pathname &&
                                    strcmp((*__pfl)->__pf->__pathname, __pathname) == 0)
        {
            _file_close((*__pfl)->__pf);
            free((*__pfl));
            (*__pfl) = NULL;
            return 0;
        }

        /* 唯一节点不匹配路径，删除失败 */
        return -1; 
    }
    else
    {
        /* 是否删除头节点 */
        if((*__pfl)->__pf && (*__pfl)->__pf->__pathname &&
                                    strcmp((*__pfl)->__pf->__pathname, __pathname) == 0)
        {
            /* 找到尾节点 */
            while(__p->next != __h)
            {
                __p = __p->next;
            }

            _list_h *__tail = __p;
            __tail->next = __h->next;

            __flist_t *__head_nd = GET_FILE_LIST_NODE(__h);
            _file_close(__head_nd->__pf);
            free(__head_nd);

            (*__pfl) = GET_FILE_LIST_NODE(__tail->next);
            /* 重新标记新的头节点 */
            (*__pfl)->__index = LIST_HEAD; 
            return 0;
        }
        else
        {
            /* 标记__p节点的上一个节点 */
            _list_h *__prev = __h;
            while(__p != __h)
            {
                __flist_t *__nd = GET_FILE_LIST_NODE(__p);
                if(__nd->__pf && __nd->__pf->__pathname 
                                            && strcmp(__nd->__pf->__pathname ,__pathname) == 0)
                {
                    __prev->next = __p->next;
                    _file_close(__nd->__pf);
                    free(__nd);
                    return 0;
                }

                __prev = __p;
                __p = __p->next;
            }
        }
    }
    /* 未找到节点 */
    return -1;
}