#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

void sig_handle(int sig)
{
    printf("11111\n");
}
/**
 * @name    main
 * @brief   
 */
int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    __sig_t *__psig = _sig_init();
    if(__psig == NULL)
        exit(-1);

    __psig->__num = SIGQUIT;
    __psig->__act->sa_handler = sig_handle;
    __psig->__oact = NULL;
    
    if(_sig_sigaction(__psig) == -1)
        exit(-1);

    while(1){
        sleep(5);
        if(_sig_raise(__psig->__num) == -1)
            break;
    }
    
    _sig_free(&__psig);
    exit(0);
}
