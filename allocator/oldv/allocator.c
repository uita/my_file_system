#include "allocator.h"
#include "string.h"
#include "stdlib.h"

#define MAX_STACK_SIZE BLOCK_SIZE/sizeof(uint32_t)

struct list_list stack;

bool allocate_block(uint32_t* block_num) 
{
	struct alloc_node* top;
	struct block next_block;
	bool error;
	uint32_t i;
	uint8_t *ptr;
	uint32_t bn;
	struct alloc_node *node;
	if (stack.size == 0)
		return false;
	remove_list_head(top, stack, struct alloc_node, ld);
	*block_num = top->block_num;
	free(top);
	top = NULL;
	/* if stack if empty after allocation, should move a stack from disk according to the last block number */
	if (stack.size == 0) { 
		error = !read_block(&next_block, top->block_num);
		if (error) {
			printf("failed to read disk while allocating\n");
			return false;
		}
		ptr = next_block.data;
		for (i = 0; i < MAX_STACK_SIZE; ++i) {
			bn = 0 | (uint32_t)*ptr | 
			         (((uint32_t)*(ptr+1))<<8) |
				 (((uint32_t)*(ptr+2))<<16) |
				 (((uint32_t)*(ptr+3))<<24);
			node = (struct alloc_node*)malloc(sizeof(struct alloc_node));
			node->block_num = bn;
			add_to_list_head(node, stack, ld);
			ptr = ptr + 4;
		}
	}
	return true;
}

bool reclaim_block(uint32_t block_num) {
	int i;
	uint8_t *ptr;
	uint32_t bn;
	struct block bk;
	struct alloc_node *node;
	bool error;
	if (stack.size == MAX_STACK_SIZE) { /* stack is full, should move the stack to disk */
		ptr = bk.data + (MAX_STACK_SIZE-1) * 4;
		for (i = 0; i < MAX_STACK_SIZE; ++i) {
			remove_list_head(node, stack, struct alloc_node, ld);
			bn = node->block_num;
			free(node);
			*(ptr)   = bn;
			*(ptr+1) = bn >> 8;
			*(ptr+2) = bn >> 16;
			*(ptr+1) = bn >> 24;
			ptr = ptr - 4;
		}
		error = !write_block(&bk, block_num);
		if (error) {
			printf("failed to write a block while reclaiming block\n");
			return false;
		}
	}
	node = (struct alloc_node*)malloc(sizeof(struct alloc_node));
	node->block_num = block_num;
	add_to_list_head(node, stack, ld);
	return true;
}
