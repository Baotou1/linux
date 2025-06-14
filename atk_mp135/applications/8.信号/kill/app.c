#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

/**
 * @name    main
 * @brief   
 */
int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    if(argc < 2)
        exit(-1);

    int pid = atoi(argv[1]);
    printf("pid: %d\n" ,pid);
    
    if(_sig_kill(pid ,SIGINT) == -1)
        exit(-1);

    exit(0);
}
