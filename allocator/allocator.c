#include "allocator.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"

struct block_stack stack;

void empty_block_stack() {
	stack.sp = -1;
	stack.size = 0;
}

bool allocate_block(uint32_t* block_num) {
	bool error;
	uint32_t bn;
	struct block *next_block;
	if (stack.size == 0)
		return false;
	bn = stack.block_nums[stack.sp];
	/* if stack will be empty after allocation and the top of stack 
	 * is not the first block, it should move a stack from disk 
	 * according to the last block number */
	if (stack.size == 1 && stack.block_nums[stack.sp] != stack.first_block) {
		next_block = (struct block*)malloc(sizeof(struct block));
		error = !read_block(next_block, bn);
		if (error) {
			printf("failed to read disk while allocating\n");
			return false;
		}
		memcpy(stack.block_nums, next_block->data, BLOCK_SIZE);
		stack.sp = MAX_STACK_SIZE - 1;
		stack.size = MAX_STACK_SIZE;
		free(next_block);
	} else {
		stack.sp = stack.sp - 1;
		stack.size = stack.size - 1;
	}
	*block_num = bn;
	return true;
}

bool reclaim_block(uint32_t block_num) {
	bool error;
	struct block *next_block;
	if (stack.size == 0) { /* mark the first block */
		stack.first_block = block_num;
	}
	if (stack.size == MAX_STACK_SIZE) { /* stack is full, should move the stack to disk */
		next_block = (struct block*)malloc(sizeof(struct block));
		memcpy(next_block->data, stack.block_nums, BLOCK_SIZE);
		error = !write_block(next_block, block_num);
		if (error) {
			printf("failed to write a block while reclaiming block\n");
			return false;
		}
		stack.sp = 0;
		stack.size = 1;
	} else {
		stack.sp = stack.sp + 1;
		stack.size = stack.size + 1;
	}
	stack.block_nums[stack.sp] = block_num;
	return true;
}
