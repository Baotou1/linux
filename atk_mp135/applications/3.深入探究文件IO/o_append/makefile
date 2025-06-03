main: test_app.o file.o
	gcc -o main test_app.o file.o

test_app.o: test_app.c
	gcc -c test_app.c

file.o: file.c
	gcc -c file.c

make clean:
	rm -rf main *.o hole_file.c