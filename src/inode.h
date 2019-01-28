#ifndef FCB_H
#define FCB_H

#include "stdint.h"
#include "type.h"
#include "super_block.h"

#define MAX_NAME_LEN 256
#define MAX_DIRECT_LEN 10
#define INDEX_SIZE 13

struct inode {
        __u32 size;             // data size
        __u32 block_count;      // not include blocks for indexes
	__u32 index[INDEX_SIZE];
        __u32 parent;
};

int ino_init(struct super_block *sb);
void ino_uninit();

int ino_read(void *block, __u32 n, struct inode *ino);
int ino_write(void *block, __u32 n, struct inode *ino);

int ino_alloc(__u32 *id);
int ino_free(__u32 id);

int ino_cs(__u32 size, struct inode *ino);

#endif
