#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

void __sa_sigaction(int __num, siginfo_t *__info, void *__context)
{
    printf("1111.\n");
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
    
    __psig->__num = SIGABRT;
    __psig->__act->sa_sigaction = __sa_sigaction;
    if(_sig_sigaction(__psig) == -1)
    {
        SIG_EXIT(__psig ,-1);
    }
    
    sleep(2);
    __abort;
    while(1);

    SIG_EXIT(__psig ,0);
}
