#include "ialloc.h"
#include "balloc.h"
#include "rw.h"
#include "super_block.h"
#include "inode.h"

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

void test_init(struct super_block *sb) {
        sb->disk_capacity = 102400;
        sb->super_block_addr = 0;
        sb->inode_zone_addr = sizeof(struct super_block);
        sb->block_zone_addr = sb->inode_zone_addr + 10240;
        sb->inode_size = sizeof(struct inode);
        sb->block_size = 512;
        sb->max_inode_num = (sb->block_zone_addr - sb->inode_zone_addr) / sb->inode_size;
        sb->max_block_num = (sb->disk_capacity - sb->block_zone_addr) / sb->block_size;
        memset((void*)(sb->iids), 0, 20);
        memset((void*)(sb->bids), 0, 512);
        sb->block_sp = 0;
        ibuf_bs = sb->inode_size;
        ibuf_ml = 10;
        bbuf_bs = sb->block_size;
        bbuf_ml = 5;
}

int main() {
        /* install */
        struct super_block sb;
        test_init(&sb);
        rw_init(&sb);
        balloc_init(&sb);
        build_bindex();
        balloc_uninit(&sb);
        rw_uninit(&sb);
        write_super_block(&sb);

        /* run */
        read_super_block(&sb);
        rw_init(&sb);
        balloc_init(&sb);
        ialloc_init(&sb);

        /* test inode */
        uint32_t inode_id = 0;
        uint32_t i;
        for (i = 0; i < 200; ++i) { // max = 160
                if (allocate_inode(&inode_id))
                        printf("%u | ", inode_id);
                else
                        printf("alloc inode %u failed\n", i);
        }
        printf("\n");
        for (i = 20; i < 160; ++i) {
                if (reclaim_inode(i))
                        printf("%u * ", i);
                else
                        printf("recla inode %u failed\n", i);
        }
        printf("\n");
        for (i = 0; i < 200; ++i) { // max = 160
                if (allocate_inode(&inode_id))
                        printf("%u | ", inode_id);
                else
                        printf("alloc inode %u failed\n", i);
        }
        printf("\n");
        for (i = 0; i < 40; ++i) {
                if (reclaim_inode(i))
                        printf("%u * ", i);
                else
                        printf("recla inode %u failed\n", i);
        }
        printf("\n");
        printf("============================\n");
        uint32_t block_id;
        for (i = 0; i < sb.max_block_num+20; ++i) {
                if (allocate_block(&block_id)) {
                        printf("alloc block %u success\n", block_id);
                } else {
                        printf("alloc block i=%u failed\n", i);
                }
        }
        for (i = 0; i < sb.max_block_num+20; ++i) {
                if (reclaim_block(i)) {
                        printf("recla block %u success\n", i);
                } else {
                        printf("recla block i=%u failed\n", i);
                }
        }
}
