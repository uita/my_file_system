#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "disk.h"
#include "stdbool.h"

/* block_num allocator */

#define MAX_STACK_SIZE BLOCK_SIZE/sizeof(uint32_t)

void empty_block_stack();
bool allocate_block(uint32_t* block_num);
bool reclaim_block(uint32_t block_num);

struct block_stack {
	uint32_t block_nums[MAX_STACK_SIZE];
	int32_t sp;
	uint32_t size;
	uint32_t first_block;
};

/* file_num allocator */

#define COL 64
#define ROW MAX_FILE_NUM/COL
#define 

void empty_file_bitmap();
bool allocate_file(uint32_t* file_num);
bool reclaim_block(uint32_t file_num);

struct file_bitmap {
	uint64_t file_nums[ROW][COL];
	uint32_t last_length;
};



#endif
