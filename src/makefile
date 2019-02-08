a.out:main.o ialloc.o balloc.o bbuf.o ibuf.o two_que.o rw.o disk.o inode.o file.o
	gcc *.o
main.o:main.c ialloc.h balloc.h rw.h super_block.h inode.h ibuf.h bbuf.h file.h
	gcc -g -c main.c
file.o:file.c file.h inode.h super_block.h list.h
	gcc -g -c file.c
inode.o:inode.c inode.h super_block.h balloc.h ialloc.h bbuf.h ibuf.h
	gcc -g -c inode.c
ialloc.o:ialloc.c super_block.h ialloc.h
	gcc -g -c ialloc.c
balloc.o:balloc.c super_block.h balloc.h bbuf.h
	gcc -g -c balloc.c
bbuf.o:bbuf.c bbuf.h two_que.h rw.h
	gcc -g -c bbuf.c
ibuf.o:ibuf.c rw.h two_que.h super_block.h
	gcc -g -c ibuf.c
two_que.o:two_que.c two_que.h list.h
	gcc -g -c two_que.c
rw.o:rw.c super_block.h rw.h disk.h
	gcc -g -c rw.c
disk.o:disk.c disk.h
	gcc -g -c disk.c

clean:
	rm *.o
	rm *.out
	rm *.gch
