#ifndef FSB_ALLOC_H
#define FSB_ALLOC_H

#include "super_block.h"
#include "stdbool.h"
#include "stdint.h"

void fsb_alloc_init(struct super_block *sb);
void fsb_alloc_uninit(struct super_block *sb);
bool fsb_build_index();

bool allocate_fsb(uint32_t* fsb_id);
bool reclaim_fsb(uint32_t fsb_id);

#endif
