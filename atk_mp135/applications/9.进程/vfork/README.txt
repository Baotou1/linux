fork


    int __open_flags = (__ACCESS_MODE(LOGFILE ,F_OK) == 0) ? 
                                (O_RDWR | O_APPEND) : 
                                (O_RDWR | O_APPEND | O_CREAT | O_EXCL);