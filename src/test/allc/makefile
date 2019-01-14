a.out:main.o ialloc.o balloc.o rw.o disk.o
	gcc *.o
main.o:main.c ialloc.h balloc.h rw.h super_block.h inode.h
	gcc -g -c main.c
ialloc.o:ialloc.c super_block.h ialloc.h
	gcc -g -c ialloc.c
balloc.o:balloc.c super_block.h balloc.h rw.h
	gcc -g -c balloc.c
rw.o:rw.c super_block.h rw.h disk.h
	gcc -g -c rw.c
disk.o:disk.c disk.h
	gcc -g -c disk.c

clean:
	rm *.o
	rm *.out
	rm *.gch
