main: app.o file.o
	gcc -o main app.o file.o

app.o: app.c
	gcc -c app.c

file.o: file.c
	gcc -c file.c

clean:
	rm -rf *.o main