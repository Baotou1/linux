#include <stdlib.h>
#include <stdio.h>
#include <signal.h>  

void mysighandler(int num)
{
    printf("SIGINT signal!\n");
    exit(0);
}

int main(int argc ,char* argv[])
{
    int signum = SIGINT;
    signal(signum ,mysighandler);
    while(1);
    return 0;
}