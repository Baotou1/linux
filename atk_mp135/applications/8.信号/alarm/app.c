#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

void sig_handle(int sig)
{
    printf("Alarm timeout\n");
    _sig_alarm(5);
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

    __psig->__num = SIGALRM;
    __psig->__act->sa_handler = sig_handle;
    __psig->__oact = NULL;
    
    if(_sig_sigaction(__psig) == -1)
        exit(-1);

    __psig->__seconds = 5;
    _sig_alarm(__psig->__seconds);
        
    while(1);
    
    _sig_free(&__psig);
    exit(0);
}
