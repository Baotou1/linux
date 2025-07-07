#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "looplist.h"

int main(int argc ,char* argv[])
{
    int i = 0;
    _list *list = list_init(1);
    
    for(i = 2; i < 10 ;i++)
        list_add_nd(list ,i);
    list_print(list);

    _list *f_nd = list_find_nd(list ,5);
    printf("f_nd = %d\n" ,f_nd->data);
    
    list = list_delete_nd(list ,6);
    list_print(list);
	printf("%p\n",list);
	printf("%p\n" ,&(list->data));
	printf("%p\n",&list);
    list_free(list);
    return 0;
}
