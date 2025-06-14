objects = app.o file.o
objects += process.o

main: $(objects)
	gcc -o $@ $^

%.o: %.c
	gcc -c $<

clean:
	rm -rf *.o main