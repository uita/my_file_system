#include "bbuf.h"
#include "tqap.h"
#include "rw.h"

struct two_que *_tq;

int rfd(void *block, __u32 id)
{
        return read_block(block, id);
}

int wtd(void *block, __u32 id)
{
        return write_block(block, id);
}

int bbuf_init(struct super_block *sb)
{
        _tq = tq_init(sb->bbuf_bs, sb->bbuf_ml, sb->bbuf_ml, rfd, wtd);
        if (!_tq)
                return false;
        return true;
}

void bbuf_uninit()
{
        tq_uninit(_tq);
}

void *bbuf_read(__u32 bid)
{
        return tq_read(bid, _tq);
}

int bbuf_write(void *block, __u32 bid)
{
        return tq_write(block, bid, _tq);
}
