#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

void sig_sigaction(int __sig, siginfo_t *__info, void *__context)
{
    printf("Signal %d received.\n", __sig);

    if (__info) {
        printf("  From PID: %d.\n", __info->si_pid);
        printf("  From UID: %d.\n", __info->si_uid);
        printf("  Signal code: %d.\n", __info->si_code);
        printf("  Received value = %d.\n" ,__info->si_value.sival_int);
    }
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

    __psig->__act->sa_flags = SA_SIGINFO;
    __psig->__act->sa_sigaction = sig_sigaction;
    __psig->__oact = NULL;
    __psig->__num = SIGINT;
    if(_sig_sigaction(__psig) == -1)
    {
        SIG_EXIT(__psig);
    }

    printf("wait receive.\n");
    while(1);

    _sig_free(&__psig);
    exit(0);
}
