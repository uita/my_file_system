#ifndef FILE_H
#define FILE_H

#include "type.h"
#include "inode.h"

struct file {
        __u32 ptr;
        __u32 ino_id;
        __u32 info;
        struct inode *ino;
        void *block;
};

int f_init(struct super_block *sb);
void f_uninit();

int f_read(void *data, __u32 size, struct file *f);
int f_write(void *data, __u32 size, struct file *f);

int f_cs(__u32 size, struct file *f);
int f_sp(__u32 ptr, struct file *f);

struct file *f_open(__u32 iid);
void f_close(struct file *f);

int f_create(__u32 *iid, __u32 parent, __u32 type);
int f_delete(__u32 iid);

int f_type(struct file *f);
time_t f_time(struct file *f);

#endif
