#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

void test1(void);
void test2(void);
int main(int argc ,char *argv[])
{
    test2();
}

void test1(void)
{
    char str1[] = "abc";
    char str2[] = "abc";
    char *str3 = "abc";
    char *str4 = "abc";
    const char str5[] = "abc";
    const char str6[] = "abc";
    const char *str7 = "abc";
    const char *str8 = "abc";

    printf("str1 = %p\n",str1);
    printf("str2 = %p\n",str2);
    printf("str3 = %p\n",str3);
    printf("str4 = %p\n",str4);
    printf("str5 = %p\n",str5);
    printf("str6 = %p\n",str6);
    printf("str7 = %p\n",str7);
    printf("str8 = %p\n",str8);
    exit(0);
}

void test2(void)
{
    printf("hello");
    write(STDOUT_FILENO ,"write\n" ,sizeof("write\n"));
    exit(0);
}