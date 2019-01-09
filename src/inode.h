#ifndef FCB_H
#define FCB_H

/* operations for single file */

#include "stdint.h"

#define MAX_NAME_LEN 256
#define MAX_DIRECT_LEN 11

struct inode {
        __u32 size;             // data size
        __u32 block_count;      // not include blocks for indexes
	__u32 direct[MAX_DIRECT_LEN];
	__u32 single_indirect;
	__u32 double_indirect;
        __u32 parent;
};

int read(void* ptr, __u32 addr, __u32 size, struct inode* ino);

/* var=true, means the size of file wouble be change to (addr+size) */
int write(void* ptr, __u32 addr, __u32 size, struct inode* ino, int var);

/* empty the blocks of file but don't delete file */
int clear(struct inode* f);

int resize(__u32 size);

__u32 max_data_size(struct inode *f);

/* include blocks for indexes */
__u32 inode_block_count(struct inode *f);


#endif
