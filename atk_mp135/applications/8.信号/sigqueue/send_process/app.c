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

    if(argc != 3)
        exit(-1);

    __sig_t *__psig = _sig_init();
    if(__psig == NULL)
        exit(-1);
    
    __psig->__val.sival_int = 10;
    __psig->__pid = atoi(argv[1]);
    __psig->__num = atoi(argv[2]);
    if(_sig_sigqueue(__psig->__pid ,__psig->__num ,__psig->__val) == -1)
    {
        SIG_EXIT(__psig);
    }
    
    printf("signal send successed,send val %d\n",__psig->__val.sival_int);
    _sig_free(&__psig);
    exit(0);
}
