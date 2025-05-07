#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

struct _mystruct
{
    int b;
    int c;
};
typedef struct _mystruct mystruct;
mystruct mys_a;

int main(int argc ,char* argv[])
{
    int a = 10;
    char *b = (char*)(&a);

    printf("%p\n" ,(void*)(b));
    printf("%p\n" ,(void*)(&a));

    printf("%p\n" ,(void*)(&mys_a));
    printf("%p\n" ,(void*)(&mys_a.b));
    printf("%p\n" ,(void*)(&mys_a.c));
    printf("%p\n" ,(void*)offsetof(mystruct, c));

    printf("%p\n" ,(void*)\
                    ((char*)(&mys_a.c) - offsetof(mystruct, c)));
    return 0;
}