objects = app.o file.o
objects += process.o
objects += log.o
objects += signal.o
objects += file_looplist.o 

main: $(objects)
	gcc -o $@ $^

%.o: %.c
	gcc -c $<

clean:
	rm -rf *.o main