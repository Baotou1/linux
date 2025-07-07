#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "dlist.h"

int main(int argc ,char* argv[])
{
    int i = 0;
    _dlist *dlist = dlist_init(0);//初始化一个哨兵头
    for(i = 1; i < 10 ;i++)
        dlist_add_nd(dlist ,i);
    dlist_print(dlist);

    _dlist *nd = dlist_find_nd(dlist ,5);
    if(nd)
        printf("nd == %d\n" ,nd->data);
    else
        printf("nd == NULL\n");
    for(i = 1; i < 10 ;i++)
        dlist_delete_nd(dlist ,i);
    dlist_print(dlist);
    dlist_free(dlist);
    return 0;
}