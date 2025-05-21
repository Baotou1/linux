#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

//_exit() _EXIT() exit()
int main(void) {
    int fd;
    fd = open("./test_file" ,O_RDONLY);
    if(fd == -1){
        perror("errno");
        return -1;
    }
    close(fd);
    _exit(0); //等价于return 0
}  