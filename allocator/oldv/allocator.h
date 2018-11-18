#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "disk.h"
#include "list.h"

#define BLOCK_NUM 1000

bool allocate_block(uint32_t* block_num);
bool reclaim_block(uint32_t* block_num);

struct alloc_node {
	uint32_t block_num;
	list_node ld;
}

#endif
