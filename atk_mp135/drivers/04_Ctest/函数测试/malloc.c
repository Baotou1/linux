#include <stdio.h>
#include <stdlib.h>
typedef struct 
{
	int _a;
	int _b;
}mystruct;
mystruct* a;
mystruct* b;

int main(char argc ,char* argv[])
{
	int* ptr = NULL;
	if(!ptr)
	{
		printf("111\n");
	}
	if(ptr)
	{
		printf("222\n");
	}
	a = (mystruct*)malloc(sizeof(*a));
	b = (mystruct*)malloc(sizeof(*b));
	if((a != NULL) && (b != NULL))
	{
		printf("int = %ld\n",sizeof(int));
		printf("a = %ld\n",sizeof(a));
		printf("b = %ld\n",sizeof(b));
	}
	return 0;
}
