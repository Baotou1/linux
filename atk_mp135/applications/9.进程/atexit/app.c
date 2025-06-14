#include "file.h"
#include "process.h"

void test1(void)
{
    printf("222\n");
}

int main(int argc ,char *argv[])
{
    __proc_t __proc;

    __proc.__function = test1;
    if(_proc_atexit(__proc.__function) == -PROC_ERROR){
        fprintf(stderr, "atexit registration failed\n");
        exit(-1);
    }
        
    exit(0);
}