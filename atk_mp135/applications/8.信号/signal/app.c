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

    if(_sig_signal(SIGINT ,sig_handle) == SIG_ERR){
        _log_write("sign" ,"error: init default.\n");
        exit(-1);
    }
    _log_write("sign" ,"init successed.\n");
    while(1);
    exit(0);
}