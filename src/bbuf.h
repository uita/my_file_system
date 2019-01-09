#ifndef BBUF_H
#define BBUF_H

int bbuf_init(struct super_block *sb);
void bbuf_uninit();

void *bbuf_read(__u32 bid);
int bbuf_write(void *block, __u32 bid);

#endif
