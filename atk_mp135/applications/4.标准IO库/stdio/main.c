#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"

void test1(void);
void test2(void);
void test3(void);
void test4(void);
void test5(void);
void test(void);
int main(int argc ,char *argv[])
{
    //test1();
    //test2();
    //test3();
    //test4();
    test5();
}


/*标准输出 printf()的行缓冲模式测试*/
void test1(void)
{
    printf("hello world!\n");
    printf("hello world");
    for(;;)
        sleep(1);

    exit(0);
}

/*标准输出 printf()的无缓冲模式测试*/
void test2(void)
{
    int ret = setvbuf(stdout ,NULL ,_IONBF ,0);
    if(ret != 0){
        exit(-1);
    }

    printf("hello world!\n");
    printf("hello world");
    for(;;)
        sleep(1);
        
    exit(0);
}

/*标准输出 printf()的全缓冲模式测试*/
void test3(void)
{
    char *buf = (char*)calloc(1024 ,1);
    if(buf == NULL)
        exit(-1);
    
    int ret = setvbuf(stdout ,buf ,_IOFBF ,1024);
    if(ret != 0){
        exit(-1);
    }

    printf("hello world!\n");
    printf("hello world");
    for(;;)
        sleep(1);
    
    free(buf);
    exit(0);
}

/*关闭文件时刷新stdio缓冲区*/
void test4(void)
{
    printf("hello world!\n");
    printf("hello world");
    fclose(stdout);
    for(;;)
        sleep(1);
   
    exit(0);    
}

/*程序退出刷新stdio缓冲区*/
void test5(void)
{
    printf("hello world!\n");
    printf("hello world");
    _exit(0);
}

void test(void)
{
    char buf2[1024];
    char *buf1 = (char*)calloc(1024 ,1);
    if(buf1 == NULL)
        exit(-1);
    
    int *buf3 = (int*)calloc(1024 ,sizeof(int));
    if(buf3 == NULL)
        exit(-1);
    printf("buf1 = %ld\n",sizeof(buf1));
    printf("buf2 = %ld\n",sizeof(buf2));
    printf("buf3 = %ld\n",sizeof(buf3));
    printf("%ld\n",sizeof(int));
    free(buf1);
    free(buf3);
    exit(0);
}