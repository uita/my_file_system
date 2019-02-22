#ifndef IBUF_H
#define IBUF_H

int ibuf_init(struct super_block *sb);
void ibuf_uninit();

int ibuf_read(void *inode, __u32 iid);
int ibuf_write(void *inode, __u32 iid);

#endif
