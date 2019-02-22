#include "file.h"
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

int is_equal(char *l1, char *l2, unsigned size) {
        for (unsigned i = 0; i < size; ++i)
                if (l1[i] != l2[i])
                        return 0;
        return 1;
}
void print(char *l1, char *l2, unsigned size) {
        printf("============================\n");
        for (unsigned i = 0; i < size; ++i)
                printf("%c", l1[i]);
        printf("\n");
        for (unsigned i = 0; i < size; ++i)
                printf("%c", l2[i]);
        printf("\n");
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
        f_init(&sb);

        /* test file */
        char wbuf[588];
        char rbuf[588];
        int l = 588, e;
        for (int i = 0; i < 588; ++i)
                wbuf[i] = (char)(i%10+48);
        __u32 iid = 0;
        f_create(&iid, 0, 0);
        struct file *f = NULL;
        for (int t = 0; t < 10000; t++) {
                l = rand() % 588;
                f = f_open(iid);
                e = f_cs(l, f);
                e = f_write(wbuf, l, f);
                e = f_sp(0, f);
                e = f_read(rbuf, l, f);
                f_close(f);
                f = NULL;
                if (!is_equal(wbuf, rbuf, l)) {
                        printf("error %d\n", l);
                        print(wbuf, rbuf, l);
                        break;
                }
                memset(rbuf, 0, 588);
        }
        return 0;
}
