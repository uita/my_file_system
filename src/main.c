#include "mfs.h"
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

void sb_init(struct super_block *sb) {
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
        sb->root_dir = 0;
        sb->dir_max_degree = 100;
}

int is_equal(char *l1, const char *l2, unsigned size) {
        int i;
        for (i = 0; i < size && l1[i] != '\0' && l2[i] != '\0'; ++i)
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
        sb_init(&sb);
        rw_init(&sb);
        ibuf_init(&sb);
        bbuf_init(&sb);
        ialloc_init(&sb);
        balloc_init(&sb);
        ino_init(&sb);
        build_bindex();

        sb.root_dir = ino_alloc(0xffffffff, DIR_TYPE);

        ino_uninit();
        balloc_uninit(&sb);
        ialloc_uninit(&sb);
        bbuf_uninit();
        ibuf_uninit();
        rw_uninit(&sb);
        write_super_block(&sb);

        /* init */
        read_super_block(&sb);
        rw_init(&sb);
        bbuf_init(&sb);
        ibuf_init(&sb);
        balloc_init(&sb);
        ialloc_init(&sb);
        ino_init(&sb);
        f_init(&sb);
        mfs_init(&sb);

        /* run */
        char name[file_name_max_len];
        char path1[256];
        char path2[256];
        char ins[10];

        printf(">>");
        while (scanf("%s", ins)) {
                memset(name, illegal_char, file_name_max_len);
                if (is_equal(ins, "pwd", file_name_max_len))          {
                        mfs_pwd();
                } else if (is_equal(ins, "ls", file_name_max_len))    {
                        mfs_ls();
                } else if (is_equal(ins, "cd", file_name_max_len))    {
                        scanf("%s", path1);
                        mfs_cd(path1);
                } else if (is_equal(ins, "cp", file_name_max_len))    {
                        scanf("%s%s", path1, path2);
                        mfs_cp(path1, path2);
                } else if (is_equal(ins, "mv", file_name_max_len))    {
                        scanf("%s%s", path1, path2);
                        mfs_mv(path1, path2);
                } else if (is_equal(ins, "mkdir", file_name_max_len)) {
                        scanf("%s", name);
                        mfs_mkdir(name);
                } else if (is_equal(ins, "touch", file_name_max_len)) {
                        scanf("%s", name);
                        mfs_touch(name);
                } else if (is_equal(ins, "rm", file_name_max_len))    {
                        scanf("%s", path1);
                        mfs_rm(path1);
                } else if (is_equal(ins, "cat", file_name_max_len))   {
                        scanf("%s", path1);
                        mfs_cat(path1);
                } else if (is_equal(ins, "input", file_name_max_len)) { 
                        scanf("%s%s", path1, path2);
                        mfs_input(path1, path2);
                } else if (is_equal(ins, "output", file_name_max_len)) { 
                        scanf("%s%s", path1, path2);
                        mfs_output(path1, path2);
                } else if (is_equal(ins, "open", file_name_max_len))  {
                } else if (is_equal(ins, "close", file_name_max_len)) {
                } else if (is_equal(ins, "read", file_name_max_len))  {
                } else if (is_equal(ins, "write", file_name_max_len)) {
                } else if (is_equal(ins, "esc", file_name_max_len)) {
                        break;
                } else {
                        printf("unknown instruction\n");
                }
                printf(">>");
        }

        /* uninit */
        mfs_uninit(&sb);
        f_uninit();
        ino_uninit();
        ialloc_uninit(&sb);
        balloc_uninit(&sb);
        ibuf_uninit();
        bbuf_uninit();
        rw_uninit(&sb);
        write_super_block(&sb);
}
