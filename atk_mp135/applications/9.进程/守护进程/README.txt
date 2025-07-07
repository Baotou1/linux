hello
进程测试
hello
hello
wwww
wwww
wwww
aaaa
aaaa
aaaa

    __proc->__pfl = __file_list_init();
    if(__proc->__pfl == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }

    __proc->__pfl->__pf = _file_init("./README.txt");
    if(__proc->__pfl->__pf == NULL)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    if(_file_open(__proc->__pfl->__pf ,O_RDWR ,0) == -FILE_ERROR)
    {
        PROCESS_EXIT_FLUSH(&__proc ,-1);
    }
    char __str[] = "aaaa\n";

    PROCESS_EXIT_FLUSH(&__proc ,0);
    