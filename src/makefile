a.out:main.o fcb_alloc.o fsb_alloc.o rw.o disk.o
	gcc *.o
main.o:main.c fcb_alloc.h fsb_alloc.h rw.h super_block.h fcb.h
	gcc -g -c main.c
fcb_alloc.o:fcb_alloc.c super_block.h fcb_alloc.h
	gcc -g -c fcb_alloc.c
fsb_alloc.o:fsb_alloc.c super_block.h fsb_alloc.h rw.h
	gcc -g -c fsb_alloc.c
rw.o:rw.c super_block.h rw.h disk.h
	gcc -g -c rw.c
disk.o:disk.c disk.h
	gcc -g -c disk.c

clean:
	rm *.o
	rm *.out
	rm *.gch
