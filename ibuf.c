#include "super_block.h"
#include "rw.h"
#include "ibuf.h"
#include "two_que.h"

static struct two_que *_tq = NULL;

int ibuf_rfd(void *inode, __u32 id)
{
        return read_inode(inode, id);
}

int ibuf_wtd(void *inode, __u32 id)
{
        return write_inode(inode, id);
}

int ibuf_init(struct super_block *sb)
{
        _tq = tq_create(sb->ibuf_bs, sb->ibuf_ml, sb->ibuf_ml, ibuf_rfd, ibuf_wtd);
        if (!_tq)
                return 0;
        return 1;
}

void ibuf_uninit()
{
        tq_destroy(_tq);
}

int ibuf_read(void *inode, __u32 iid)
{
        return tq_read(inode, iid, _tq);
}

int ibuf_write(void *inode, __u32 iid)
{
        return tq_write(inode, iid, _tq);
}
