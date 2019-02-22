#ifndef BBUF_H
#define BBUF_H

int bbuf_init(struct super_block *sb);
void bbuf_uninit();

int bbuf_read(void *block, __u32 bid);
int bbuf_write(void *block, __u32 bid);


/* Path = id + offset + offset + ... 
 * Len = the number of 'offsets'
 * For indexes
 * In terms of blocks */
int bbuf_read_wp(void *block, __u32 *path, int len);
int bbuf_write_wp(void *block, __u32 *path, int len);

#endif
