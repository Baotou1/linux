#include "file.h"
#include "process.h"
#include "log.h"
#include "signal.h"

void sig_handle(int __sig)
{
    printf("aaabbb\n");
}
/**
 * @name    main
 * @brief   
 */
int main(int argc ,char *argv[])
{
    if(_log_init() == -1)
        exit(-1);

    __sig_t *__sig = (__sig_t *)calloc(1 ,sizeof(__sig_t));
    if(__sig == NULL)
        exit(-1);

    __sig->__act = (struct sigaction *)calloc(1 ,sizeof(struct sigaction));    
    if(__sig->__act == NULL){
        free(__sig);
        __sig = NULL;
        exit(-1);
    }

    __sig->__num = SIGINT;
    __sig->__act->sa_handler = sig_handle;
    __sig->__oact = NULL;

    if(_sig_sigaction(__sig) == -1){
        _log_write("sign" ,"error: init default.\n");
        exit(-1);
    }
    _log_write("sign" ,"init successed.\n");

    while(1);

    free(__sig->__act);
    free(__sig);
    __sig = NULL;
    exit(0);
}
