#ifndef BBUF_H
#define BBUF_H

int bbuf_init(struct super_block *sb);
void bbuf_uninit();

int bbuf_read(void *block, __u32 bid);
int bbuf_write(void *block, __u32 bid);

#endif
