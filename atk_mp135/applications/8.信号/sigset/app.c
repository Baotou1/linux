#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

void sig_handle(int sig)
{
    printf("11111.\n");
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

    __psig->__act->sa_handler = sig_handle;
    __psig->__num = SIGINT;
    
    if(_sig_sigaction(__psig) == -1)
    {
        SIG_EXIT(__psig);
    }
    
    if(_sig_sigemptyset(__psig->__sig_set) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigaddset(__psig->__sig_set ,SIGINT) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_sigprocmask(SIG_BLOCK ,__psig->__sig_set ,NULL) == -1)
    {
        SIG_EXIT(__psig);
    }

    if(_sig_raise(SIGINT) == -1)
    {
        SIG_EXIT(__psig);
    }
     
    printf("sleep.\n");
    sleep(5);

    if(_sig_sigprocmask(SIG_UNBLOCK ,__psig->__sig_set ,NULL) == -1)
    {
        SIG_EXIT(__psig);
    }

    printf("222\n");
    _sig_pause();
 
    _sig_free(&__psig);
    exit(0);
}
