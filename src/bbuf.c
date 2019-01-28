#include "super_block.h"
#include "bbuf.h"
#include "two_que.h"
#include "rw.h"

struct two_que *_tq;

int bbuf_rfd(void *block, __u32 id)
{
        return read_block(block, id);
}

int bbuf_wtd(void *block, __u32 id)
{
        return write_block(block, id);
}

int bbuf_init(struct super_block *sb)
{
        _tq = tq_create(sb->bbuf_bs, sb->bbuf_ml, sb->bbuf_ml, bbuf_rfd, bbuf_wtd);
        if (!_tq)
                return false;
        return true;
}

void bbuf_uninit()
{
        tq_destroy(_tq);
}

int bbuf_read(void *block, __u32 bid)
{
        return tq_read(block, bid, _tq);
}

int bbuf_write(void *block, __u32 bid)
{
        return tq_write(block, bid, _tq);
}

/* Path = id + offset + offset + offset + ... 
 * Len = the number of 'offsets'
 * For indexes 
 * In terms of blocks */
int bbuf_read_wp(void *block, __u32 *path, int len)
{
        return tq_read_wp(block, path, len, _tq);
}

int bbuf_write_wp(void *block, __u32 *path, int len)
{
        return tq_write_wp(block, path, len, _tq);
}
