#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

#include "type.h"

struct super_block {
        /* needed for install */
        __u32 disk_capacity;   /* bytes */ 

        __u32 super_block_addr;
        __u32 inode_zone_addr;
        __u32 block_zone_addr;
        __u32 inode_size;
        __u32 block_size;      /* block size */
        __u32 max_inode_num;
        __u32 max_block_num;
        __u32 iids[5];
        __u32 bids[128];
        __32  block_sp;
        __u32 ibuf_rc;
        __u32 ibuf_rs;
        __u32 bbuf_rc;
        __u32 bbuf_rs;
};

#endif
