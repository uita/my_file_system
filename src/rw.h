#ifndef RW_H
#define RW_H


#include "super_block.h"
#include "stdint.h"
#include "stdbool.h"

/* this two functions can be called before init */
bool read_super_block(struct super_block *sb);
bool write_super_block(struct super_block *sb);

void rw_init(struct super_block *sb);
void rw_uninit(struct super_block *sb);

bool read_fcb(uint8_t *fcb, uint32_t fcb_id);
bool write_fcb(uint8_t *fcb, uint32_t fcb_id);

bool read_fsb(uint8_t *fsb, uint32_t fsb_id);
bool write_fsb(uint8_t *fsb, uint32_t fsb_id);

#endif
