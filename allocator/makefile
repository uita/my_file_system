a.out:main.o allocator.o disk.o
	gcc -g main.o allocator.o disk.o
main.o:main.c allocator.h
	gcc -g -c main.c
allocator.o:allocator.c disk.h
	gcc -g -c allocator.c
disk.o:disk.c
	gcc -g -c disk.c

clean:
	rm *.o
	rm a.out
