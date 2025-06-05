utimensat
    __UMASK(0003);
    __CHMOD(PATHNAME ,0774);
    int ret = 0;
    __ACCESS_MODE(PATHNAME , F_OK ,ret);
    if(ret == -1){
        printf("Error: %s file does not exist!\n", PATHNAME);
        exit(-1);
    }
#if 1 /* 相对路径 */
    if(argc > 1){
        /* 同时设置为当前时间 */
        if(strcmp(argv[1] , "NULL") == 0){
            printf("\ntimes == NULL\n");
            if(utimensat(AT_FDCWD ,PATHNAME ,NULL ,AT_SYMLINK_NOFOLLOW) == -1)
                exit(-1);
        }
    }
#endif
#if 0 /* 绝对路径 */
    if(argc > 1){
        /* 同时设置为当前时间 */
        if(strcmp(argv[1] , "NULL") == 0){
            printf("\ntimes == NULL\n");
            if(utimensat(-1 ,PATHNAME ,NULL ,AT_SYMLINK_NOFOLLOW) == -1)
                exit(-1);
        }
        /* 设置访问时间为当前时间,内存修改时间不变 */
        else if(strcmp(argv[1] ,"timespec") == 0){
            printf("\ntimes == timespec.\n");
            struct timespec timsp_arr[2];
            memset(timsp_arr, 0, sizeof(timsp_arr));
            
            if(argc ==4 && strcmp(argv[2] ,"UTIME_NOW") == 0 && strcmp(argv[3] ,"UTIME_OMIT") == 0){
                printf("1-UTIME_NOW and 2-UTIME_OMIT.\n");
                timsp_arr[0].tv_nsec = UTIME_NOW;
                timsp_arr[1].tv_nsec = UTIME_OMIT;
                if(utimensat(-1 ,PATHNAME ,timsp_arr ,AT_SYMLINK_NOFOLLOW) == -1)
                    exit(-1);            
            }
        }
    }
#endif