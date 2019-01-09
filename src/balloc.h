#ifndef BALLOC_H
#define BALLOC_H

#include "super_block.h"
#include "type.h"

void balloc_init(struct super_block *sb);
void balloc_uninit(struct super_block *sb);
int build_bindex();

int allocate_block(__u32* bid);
int reclaim_block(__u32 bid);

#endif
