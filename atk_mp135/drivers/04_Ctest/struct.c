#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include <sys/ioctl.h>

struct person{
	void (*speak)(struct person *);
	char *name;
};

void myspeak(struct person *p)
{
	printf("%s\r\n",p->name);
}
int main(char argc ,char* argv[])
{
	struct person p;
	p.name = (char*)malloc(20);
	if(!p.name){
		return 1;
	}
	strcpy(p.name ,"Baotou");
	p.speak = myspeak;
	p.speak(&p);
	free(p.name);
	return 0;
}
