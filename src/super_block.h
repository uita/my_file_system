#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

#include "stdint.h"

struct super_block {
        /* needed for install */
        uint32_t disk_capacity;   /* bytes */ 

        uint32_t super_block_addr;
        uint32_t fcb_zone_addr;
        uint32_t fsb_zone_addr;
        uint32_t fcb_size;
        uint32_t fsb_size;      /* block size */
        uint32_t max_fcb_num;
        uint32_t max_fsb_num;
        uint32_t fcb_ids[5];
        uint32_t fsb_ids[128];
        int32_t fsb_sp;
};

#endif
