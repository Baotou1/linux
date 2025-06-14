objects = app.o file.o
objects += process.o
objects += log.o

main: $(objects)
	gcc -o $@ $^

%.o: %.c
	gcc -c $<

clean:
	rm -rf *.o main