一个"进程"内多次open打开同一个文件，那么会得到多个不同的文件描述符fd，同理在关闭文件的时候也需要调用close依次关闭各个文件描述符。 
打印多次打开同一个文件的文件描述法fd
