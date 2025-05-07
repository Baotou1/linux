#include <stdio.h>
#include <stdlib.h>
/* 测试俩个无符号数相减  */
int main(char argc ,char* argv[])
{
	unsigned int a,b;
	a=0x01;
	b=0x10;
	printf("a - b = %x\n",(a-b));
	return 0;
}
