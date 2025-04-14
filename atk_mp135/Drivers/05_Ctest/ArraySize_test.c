#include "stdio.h"

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))


void uprint(int* buf)
{
	printf("数组个数为%ld\n",ARRAY_SIZE(buf));
	printf("sizeof(buf) = %ld\n" ,sizeof(buf));
	printf("sizeof(buf[0]) = %ld\n" ,sizeof(buf[0]));
}

int main(char argc ,char* argv[])
{
	int buf[10];
	for(unsigned char i = 0 ;i < 10 ;i++)
	{
		buf[i] = i;
	}
	printf("数组个数为%ld\n",ARRAY_SIZE(buf));
	uprint(buf);
	return 0;
}
