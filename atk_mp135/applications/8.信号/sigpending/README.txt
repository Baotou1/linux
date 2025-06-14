sigpending
switch(_sig_sigismember(&_wait_mask, SIGINT))
    {
        case -1:
            SIG_EXIT(__psig);
        case 0:
            printf("SIGINT 信号未处于等待状态\n");
            break;
        case 1:
            printf("SIGINT 信号处于等待状态\n");
            break;
    }