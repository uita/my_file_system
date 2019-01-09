#include "bbuf.h"
#include "dmap.h"
#include "rw.h"

struct dmap *_dm;

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
        _dm = dmap_init(sb->bbuf_rc, sb->bbuf_rs, rfd, wtd);
        if (!_dm)
                return false;
        return true;
}

void bbuf_uninit()
{
        if (_dm)
                dmap_uninit(_dm);
}

void *bbuf_read(__u32 bid)
{
        return read_row(bid, _dm);
}

int bbuf_write(void *block, __u32 bid)
{
        return write_row(block, bid, _dm);
}
