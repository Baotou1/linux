#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "list.h"

DECLARE_LIST_HEAD("head" ,list);
int main(int argc ,char* argv[])
{
    printf("len = %d\n" ,list_length(&list));

    list_add_nd(&list ,"pf8");
    list_add_nd(&list ,"pf9");
    list_add_nd(&list ,"pf10");
    list_add_nd(&list ,"pf11");
    list_print_nd(&list);
    printf("len = %d\n" ,list_length(&list));

    struct __list_node* newnd = list_find_nd(&list ,"pf10");
    printf("newnd->name = %s\n" ,newnd->name);
    newnd->name = "pa10";
    list_print_nd(&list);

    list_delete_nd(&list ,"pf11");
    list_print_nd(&list);
    printf("len = %d\n" ,list_length(&list));

    list_free(&list);
    list_print_nd(&list);
    printf("len = %d\n" ,list_length(&list));
    return 0;
}