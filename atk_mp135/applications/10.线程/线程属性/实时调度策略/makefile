objects = main.o file.o
objects += process.o
objects += log.o
objects += signal.o
objects += file_looplist.o 
objects += thread.o 
objects += thread_list.o 
objects += applicate.o 
objects += init.o 
objects += tsync.o 

main: $(objects)
	gcc -o $@ $^ -pthread

%.o: %.c
	gcc -c $<

clean:
	rm -rf *.o main
