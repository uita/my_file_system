#include "super_block.h"
#include "rw.h"
#include "ibuffer.h"

static struct dmap *_dm = NULL;

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
        _dm = dmap_init(sb->ibuf_rc, sb->ibuf_rs, rfd, wtd);
        if (!_dm)
                return false;
        return true;
}

void ibuf_uninit()
{
        if (_dm)
                dmap_uninit(_dm);
}

void *ibuf_read(void *inode, __u32 iid)
{
        return read_row(iid, _dm);
}

int ibuf_write(void *inode, __u32 iid)
{
        return write_row(inode, iid, _dm);
}
