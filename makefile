a.out:main.o ialloc.o balloc.o bbuf.o ibuf.o two_que.o rw.o disk.o inode.o file.o mfs.o
	gcc *.o
main.o:main.c ialloc.h balloc.h rw.h super_block.h inode.h ibuf.h bbuf.h file.h mfs.h
	gcc -c main.c
mfs.o:mfs.c mfs.h file.h list.h type.h
	gcc -c mfs.c
file.o:file.c file.h inode.h super_block.h list.h
	gcc -c file.c
inode.o:inode.c inode.h super_block.h balloc.h ialloc.h bbuf.h ibuf.h
	gcc -c inode.c
ialloc.o:ialloc.c super_block.h ialloc.h
	gcc -c ialloc.c
balloc.o:balloc.c super_block.h balloc.h bbuf.h
	gcc -c balloc.c
bbuf.o:bbuf.c bbuf.h two_que.h rw.h
	gcc -c bbuf.c
ibuf.o:ibuf.c rw.h two_que.h super_block.h
	gcc -c ibuf.c
two_que.o:two_que.c two_que.h list.h
	gcc -c two_que.c
rw.o:rw.c super_block.h rw.h disk.h
	gcc -c rw.c
disk.o:disk.c disk.h
	gcc -c disk.c

clean:
	rm *.o
	rm *.out
	rm *.gch
