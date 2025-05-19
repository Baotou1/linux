# 编译目标文件
main: main.o file.o
	gcc -o main main.o file.o

# 编译 main.c
main.o: main.c
	gcc -c main.c

# 编译 file.c
file.o: file.c
	gcc -c file.c

# 清理目标文件
clean:
	rm -rf *.o main dest_file.c test_file.c
