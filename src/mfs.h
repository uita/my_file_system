#ifndef MFS_H
#define MFS_H

#include "super_block.h"
#include "file.h"

#define illegal_char '#'

int mfs_init(struct super_block *sb);
void mfs_uninit(struct super_block *sb);

void mfs_pwd();
void mfs_ls();
int mfs_cd(char *path);
int mfs_cp(char *src, char *dst);
int mfs_mv(char *src, char *dst);
int mfs_mkdir(char *name);
int mfs_touch(char *name);
int mfs_rm(char *path);
struct file *mfs_open(char *path);
int mfs_close(struct file *f);
int mfs_read(void *data, __u32 size, struct file *f);
int mfs_write(void *data, __u32 size, struct file *f);

#endif
