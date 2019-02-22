#ifndef RW_H
#define RW_H

#include "type.h"
#include "super_block.h"

/* this two functions can be called before init */
int read_super_block(struct super_block *sb);
int write_super_block(struct super_block *sb);

void rw_init(struct super_block *sb);
void rw_uninit(struct super_block *sb);

int read_inode(void *inode, __u32 iid);
int write_inode(void *inode, __u32 iid);

int read_block(void *block, __u32 bid);
int write_block(void *block, __u32 bid);

#endif
