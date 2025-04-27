#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int main(int argc ,char* agrv[])
{
    _list *list = list_init_head(5);
    if(list == NULL)
        printf("list init errpr\n");
    list_add_nd(list ,6);
    list_add_nd(list ,8);
    list_add_nd(list ,9);
    _list *list_a = list_init_head(10);
    list = list_add_list(list ,list_a);
    list_print(list);

    _list *f_nd = list_find_nd(list ,6);
    printf("f_nd = %d\n" ,f_nd->data);

    list = list_delete_nd(list ,5);
    list_print(list);
    list = list_delete_nd(list ,9);
    list_print(list);
    list_free(list);
    return 0;
}