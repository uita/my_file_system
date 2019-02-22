#include "ialloc.h"
#include "balloc.h"
#include "rw.h"
#include "super_block.h"
#include "inode.h"
#include "ibuf.h"
#include "bbuf.h"
#include "type.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

void test_init(struct super_block *sb) {
        memset(sb, 0, sizeof(struct super_block));
        sb->disk_capacity = 102400;
        sb->super_block_addr = 0;
        sb->inode_zone_addr = sizeof(struct super_block);
        sb->block_zone_addr = sb->inode_zone_addr + 10240;
        sb->inode_size = sizeof(struct inode);
        //sb->block_size = 512;
        sb->block_size = 12;
        sb->max_inode_num = (sb->block_zone_addr - sb->inode_zone_addr) / sb->inode_size;
        sb->max_block_num = (sb->disk_capacity - sb->block_zone_addr) / sb->block_size;
        sb->spare_inode = sb->max_inode_num;
        sb->spare_block = 0;
        memset((void*)(sb->iids), 0, 20);
        memset((void*)(sb->bids), 0, 512);
        sb->block_sp = 0;
        sb->ibuf_bs = sb->inode_size;
        sb->ibuf_ml = 10;
        sb->bbuf_bs = sb->block_size;
        sb->bbuf_ml = 5;
}

int main() {
        /* install */
        struct super_block sb;
        test_init(&sb);
        rw_init(&sb);
        bbuf_init(&sb);
        balloc_init(&sb);
        build_bindex();
        balloc_uninit(&sb);
        bbuf_uninit();
        rw_uninit(&sb);
        write_super_block(&sb);

        /* run */
        read_super_block(&sb);
        rw_init(&sb);
        bbuf_init(&sb);
        ibuf_init(&sb);
        balloc_init(&sb);
        ialloc_init(&sb);
        ino_init(&sb);

        /* test inode */
        // init one inode
        __u32 root = 0;
        ino_alloc(&root);
        struct inode ino;
        ino.size = 0;
        ino.block_count = 0;
        // 10 + 3 + 9 + 27 = 49
        // 49 * 12 = 588
        char wbuf[588];
        for (int i = 0; i < 588; ++i)
                wbuf[i] = (char)(i%10+48);
        char rbuf[588];

        int re;
        for (int i = 0; i < 10000; ++i) {
                unsigned r = rand() % 588;
                re = ino_cs(r, &ino);
                if (re == 0) {
                        printf("error!!!!!!!!!!!!!!!!!!!!!!! %u\n", r);
                        break;
                } else {
                        printf("not error %u\n", r);
                }
        }
        ino_cs(588, &ino);
        printf("-----> %u", spare_block());



        //for (int i = 1; i <= 49 * 12; ++i)
        //        re = ino_cs(i, &ino);
        //for (int i = 49 * 12; i >= 0; --i)
        //        re = ino_cs(i, &ino);
        //for (int i = 1; i <= 49 * 12; ++i)
        //        re = ino_cs(i, &ino);




        //re = ino_cs(49 * 12, &ino);
        //re = ino_cs(49 * 10, &ino);
        //re = ino_cs(49 * 8, &ino);

        //re = ino_cs(103, &ino);
        //re = ino_cs(29, &ino);



        //char bb[12];
        //for (int i = 7620; i >= 7567; i--) {
        //        bbuf_read(bb, i);
        //        //read_block(bb, i);
        //        memset(bb, 0, 12);
        //}
        //for (int i = 0; i < 10; i++) {
        //        bbuf_read(bb, 7576);
        //        memset(bb, 0, 12);
        //}



        //re = ino_write(wbuf, 48, &ino);
        //re = ino_write(wbuf, 48, &ino);
        //re = ino_read(rbuf, 48, &ino);


        for (int i = 0; i < 49; ++i)
                if (!ino_write(wbuf + i*12, i, &ino))
                        break;
        printf("\n");
        for (int i = 0; i < 49; ++i)
                if (!ino_read(rbuf + i*12, i, &ino))
                        break;
        printf("====================================\n");
        for (int i = 0; i < 588; ++i) {
                printf("%c", wbuf[i]);
        }
        printf("\n------------------------------------\n");
        for (int i = 0; i < 588; ++i) {
                printf("%c", rbuf[i]);
        }
        printf("\n");
        return 0;
}
