#include "fcb_alloc.h"
#include "fsb_alloc.h"
#include "rw.h"
#include "super_block.h"
#include "fcb.h"

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

void test_init(struct super_block *sb) {
        sb->disk_capacity = 102400;
        sb->super_block_addr = 0;
        sb->fcb_zone_addr = sizeof(struct super_block);
        sb->fsb_zone_addr = sb->fcb_zone_addr + 10240;
        sb->fcb_size = sizeof(struct fcb);
        sb->fsb_size = 512;
        sb->max_fcb_num = (sb->fsb_zone_addr - sb->fcb_zone_addr) / sb->fcb_size;
        sb->max_fsb_num = (sb->disk_capacity - sb->fsb_zone_addr) / sb->fsb_size;
        memset((void*)(sb->fcb_ids), 0, 20);
        memset((void*)(sb->fsb_ids), 0, 512);
}

int main() {
        /* install */
        struct super_block sb;
        test_init(&sb);
        rw_init(&sb);
        fsb_alloc_init(&sb);
        fsb_build_index();
        fsb_alloc_uninit(&sb);
        write_super_block(&sb);

        /* run */
        read_super_block(&sb);
        rw_init(&sb);
        fsb_alloc_init(&sb);
        fcb_alloc_init(&sb);

        /* test fcb */
        uint32_t fcb_id = 0;
        uint32_t i;
        for (i = 0; i < 200; ++i) { // max = 160
                if (allocate_fcb(&fcb_id))
                        printf("%u | ", fcb_id);
                else
                        printf("alloc fcb %u failed\n", i);
        }
        printf("\n");
        for (i = 20; i < 160; ++i) {
                if (reclaim_fcb(i))
                        printf("%u * ", i);
                else
                        printf("recla fcb %u failed\n", i);
        }
        printf("\n");
        for (i = 0; i < 200; ++i) { // max = 160
                if (allocate_fcb(&fcb_id))
                        printf("%u | ", fcb_id);
                else
                        printf("alloc fcb %u failed\n", i);
        }
        printf("\n");
        for (i = 0; i < 40; ++i) {
                if (reclaim_fcb(i))
                        printf("%u * ", i);
                else
                        printf("recla fcb %u failed\n", i);
        }
        printf("\n");
        printf("============================\n");
        uint32_t fsb_id;
        for (i = 0; i < sb.max_fsb_num+20; ++i) {
                if (allocate_fsb(&fsb_id)) {
                        printf("alloc fsb %u success\n", fsb_id);
                } else {
                        printf("alloc fsb i=%u failed\n", i);
                }
        }
        for (i = 0; i < sb.max_fsb_num+20; ++i) {
                if (reclaim_fsb(i)) {
                        printf("recla fsb %u success\n", i);
                } else {
                        printf("recla fsb i=%u failed\n", i);
                }
        }
}
