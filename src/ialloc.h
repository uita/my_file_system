#ifndef FILE_ALLOCATOR_H
#define FILE_ALLOCATOR_H

#include "type.h"
#include "super_block.h"

void ialloc_init(struct super_block *sb);
void ialloc_uninit(struct super_block *sb);

int allocate_inode(__u32* iid);
int reclaim_inode(__u32 iid);

#endif
