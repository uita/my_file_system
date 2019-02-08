#include "file.h"
#include "inode.h"
#include "super_block.h"
#include "list.h"
#include "stdlib.h"

static __u32 _block_size = 0;

int f_init(struct super_block *sb) {
        _block_size = sb->block_size;
        return 1;
}

void f_uninit() {
}

int f_read(void *data, __u32 size, struct file *f)
{
        __u32 beg, end, i, cs;
        void *dp = data;
        if (size == 0)
                return 1;
        if (!f->block) {
                f->block = malloc(_block_size);
                if (!f->block || !ino_read_data(f->block, 0, f->ino))
                        return 0;
        }
        if (f->ptr + size > f->ino->size)
                return 0;
        beg = f->ptr / _block_size;
        end = (f->ptr + size) / _block_size;
        for (i = beg; i <= end; ++i) {
                if (i == beg) {
                        cs = _block_size - f->ptr % _block_size;
                        cs = cs >= size ? size : cs;
                        memcpy(data, f->block + f->ptr, cs);
                } else if (i == end) {
                        cs = (f->ptr + size) % _block_size;
                        if (cs == _block_size) {
                                ino_read_data(dp, i, f->ino);
                                ino_read_data(f->block, i + 1, f->ino); //for the new block after the end
                        } else {
                                ino_read_data(f->block, i, f->ino);
                                cs = (f->ptr + size) % _block_size + 1;
                                memcpy(dp, f->block, cs);
                        }
                } else {
                        cs = _block_size;
                        ino_read_data(dp, i, f->ino);
                }
                f->ptr += cs;
                dp += cs;
                size -= cs;
        }
        return 1;
}

int f_write(void *data, __u32 size, struct file *f)
{
        __u32 beg, end, i, cs;
        void *dp = data;
        if (size == 0)
                return 1;
        if (!f->block) {
                f->block = malloc(_block_size);
                if (!f->block || !ino_read_data(f->block, 0, f->ino))
                        return 0;
        }
        if (f->ptr + size > f->ino->size)
                return 0;
        beg = f->ptr / _block_size;
        end = (f->ptr + size) / _block_size;
        for (i = beg; i <= end; ++i) {
                if (i == beg) {
                        cs = _block_size - f->ptr % _block_size;
                        cs = cs >= size ? size : cs;
                        memcpy(f->block + f->ptr % _block_size, data, cs);
                        ino_write_data(f->block, i, f->ino);
                } else if (i == end) {
                        cs = (f->ptr + size) % _block_size;
                        if (cs == _block_size) {
                                ino_write_data(dp, i, f->ino);
                                ino_read_data(f->block, i + 1, f->ino); //for the new block after the end
                        } else {
                                ino_read_data(f->block, i, f->ino);
                                cs = (f->ptr + size) % _block_size + 1;
                                memcpy(f->block, dp, cs);
                                ino_write_data(dp, i, f->ino);
                        }
                } else {
                        cs = _block_size;
                        ino_write_data(dp, i, f->ino);
                }
                f->ptr += cs;
                dp += cs;
                size -= cs;
        }
        return 1;

}

int f_cs(__u32 size, struct file *f)
{
        return ino_cs(size, f->ino);
}

int f_sp(__u32 ptr, struct file *f)
{
        if (ptr > f->ino->size)
                return 0;
        if (ptr / _block_size == f->ptr / _block_size) {
                f->ptr = ptr;
                return 1;
        }
        if (!ino_read_data(f->block, ptr / _block_size, f->ino))
                return 0;
        f->ptr = ptr;
        return 1;
}

static void f_destroy(struct file *f) {
        if (!f)
                return;
        if (f->ino)
                ino_free(f->ino);
        f->ino = NULL;
        if (f->block)
                free(f->block);
        f->block = NULL;
        free(f);
}

static struct file *f_create_empty_file() {
        struct file *f = (struct file *)malloc(sizeof(struct file));
        if (!f)
                return NULL;
        memset(f, 0, sizeof(struct file));
        f->ino = (struct inode *)malloc(sizeof(struct inode));
        if (!f->ino)
                f_destroy(f);
        memset(f->ino, 0, sizeof(struct inode));
        return f;
}

struct file *f_open(__u32 iid) {
        struct file *f = f_create_empty_file();
        if (!f || !ino_read(f->ino, iid)) {
                f_destroy(f);
                return NULL;
        }
        f->ino_id = iid;
        return f;
}

void f_close(struct file *f) {
        ino_write(f->ino, f->ino_id);
        f_destroy(f);
}

int f_create(__u32 *iid, __u32 parent, __u32 type)
{
        struct inode *ino = NULL;
        *iid = ino_alloc(ino, parent, type);
        if (!ino)
                return 0;
        return 1;
}

int f_delete(__u32 iid)
{
        struct inode *ino = NULL;
        return ino_recla(ino, iid);
}

int f_type(struct file *f)
{
        return f->ino->type;
}

time_t f_time(struct file *f)
{
        return f->ino->ctime;
}
