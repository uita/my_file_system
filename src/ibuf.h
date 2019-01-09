#ifndef IBUF_H
#define IBUF_H

int ibuf_init(struct super_block *sb);
void ibuf_uninit();

void *ibuf_read(__u32 iid);
int ibuf_write(void *inode, __u32 iid);

#endif
