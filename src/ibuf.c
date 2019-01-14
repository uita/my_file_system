#include "super_block.h"
#include "rw.h"
#include "ibuffer.h"
#include "two_que.h"

static struct two_que *_tq = NULL;

int rfd(void *inode, __u32 id)
{
        return read_inode(inode, id);
}

int wtd(void *inode, __u32 id)
{
        return write_inode(inode, id);
}

int ibuf_init(struct super_block *sb)
{
        ///////
        _tq = tq_init(sb->ibuf_bs, sb->ibuf_ml, sb->ibuf_ml, rfd, wtd);
        if (!_tq)
                return 0;
        return 1;
}

void ibuf_uninit()
{
        tq_uninit(_tq);
}

void *ibuf_read(void *inode, __u32 iid)
{
        return tq_read(iid, _tq);
}

int ibuf_write(void *inode, __u32 iid)
{
        return tq_write(inode, iid, _tq);
}
