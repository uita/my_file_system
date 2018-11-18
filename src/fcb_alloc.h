#ifndef FILE_ALLOCATOR_H
#define FILE_ALLOCATOR_H

#include "super_block.h"
#include "stdint.h"
#include "stdbool.h"

void fcb_alloc_init(struct super_block *sb);
void fcb_alloc_uninit(struct super_block *sb);

bool allocate_fcb(uint32_t* fcb_id);
bool reclaim_fcb(uint32_t fcb_id);

#endif
