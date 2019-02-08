#ifndef FCB_H
#define FCB_H

#include "super_block.h"
#include "type.h"

#include "time.h"
#include "stdint.h"

#define MAX_NAME_LEN 256
#define MAX_DIRECT_LEN 10
#define INDEX_SIZE 13
#define FILE_TYPE 0
#define DIR_TYPE 1

struct inode {
        __u32 size;             // data size
        __u32 block_count;      // not include blocks for indexes
        __u32 type;
        time_t ctime;
	__u32 index[INDEX_SIZE];
        __u32 parent;
};

int ino_init(struct super_block *sb);
void ino_uninit();

int ino_read_data(void *block, __u32 n, struct inode *ino);
int ino_write_data(void *block, __u32 n, struct inode *ino);

int ino_cs(__u32 size, struct inode *ino);

__u32 ino_alloc(struct inode *ino, __u32 parent, __u32 type);
int ino_recla(struct inode *ino, __u32 id);
int ino_free(struct inode *ino);

int ino_read(struct inode *ino, __u32 id);
int ino_write(struct inode *ino, __u32 id);

//int ino_set_type(int type, struct inode *ino);
//int ino_type(struct inode *ino);

//int ino_set_time(time_t time, struct inode *ino);
//time_t ino_time(struct inode *ino);

__u32 ino_max_data_size();

#endif
