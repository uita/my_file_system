#ifndef FILE_H
#define FILE_H

#include "type.h"
#include "inode.h"

#define file_name_max_len 28
#define FILE_TYPE _FILE_TYPE
#define DIR_TYPE _DIR_TYPE

struct file {
        char name[file_name_max_len];
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
int f_ci(__u32 ino_id, struct file *f);

void f_sync(struct file *f);

struct file *f_open(__u32 iid, const char *name);
void f_close(struct file *f);

int f_create(__u32 *iid, __u32 parent, __u32 type);
int f_delete(__u32 iid);

#define f_type(f) (f->ino->type)
#define f_time(f) (f->ino->ctime)
#define f_size(f) (f->ino->size)
#define f_block_count(f) (f->ino->block_count)
#define f_parent(f) (f->ino->parent)

#define f_set_type(value, f) f_type(f) = value;
#define f_set_time(value, f) f_time(f) = value;
#define f_set_size(value, f) f_size(f) = value;
#define f_set_block_count(value, f) f_block_count(f) = value;
#define f_set_parent(value, f) f_parent(f) = value;

/* for directories */
int dir_read(int i, char *file_name, __u32 *ino_id, struct file *f);
int dir_add(char *file_name, __u32 ino_id, struct file *f);
int dir_remove(char *file_name, struct file *f);

#endif
