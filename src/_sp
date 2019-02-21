#include "file.h"
#include "inode.h"
#include "super_block.h"
#include "list.h"
#include "stdlib.h"

#define dir_size(f) (f_size(f) >> 5)

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
                        memcpy(data, f->block + f->ptr % _block_size, cs);
                } else if (i == end) {
                        cs = (f->ptr + size) % _block_size;
                        if (cs == _block_size) {
                                ino_read_data(dp, i, f->ino);
                                ino_read_data(f->block, i + 1, f->ino); //for the new block after the end
                        } else {
                                ino_read_data(f->block, i, f->ino);
                                //cs = (f->ptr + size) % _block_size + 1;
                                cs = size;
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
        if (f->ptr == f_size(f))
                f_sp(0, f);
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
                                //cs = (f->ptr + size) % _block_size + 1;
                                cs = size;
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
        if (f->ptr == f_size(f))
                f_sp(0, f);
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
        if (!f->block)
                f->block = malloc(_block_size);
        if (!ino_read_data(f->block, ptr / _block_size, f->ino))
                return 0;
        f->ptr = ptr;
        return 1;
}

// not change parent
int f_ci(__u32 ino_id, struct file *f)
{
        if (!f)
                return 0;
        __u32 p = f_parent(f);
        ino_recla(f->ino, f->ino_id);
        ino_read(f->ino, ino_id);
        f->ino_id = ino_id;
        f_parent(f) = p;
        return 1;
}

void f_sync(struct file *f)
{
        if (!f)
                return;
        if (f->ino != NULL)
                ino_write(f->ino, f->ino_id);
        if (f->block != NULL)
                ino_write_data(f->block, f->ptr / _block_size, f->ino);
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

struct file *f_open(__u32 iid, const char *name) {
        struct file *f = f_create_empty_file();
        if (!f || !ino_read(f->ino, iid)) {
                f_destroy(f);
                return NULL;
        }
        f->ino_id = iid;
        if (name != NULL)
                memcpy(f->name, name, file_name_max_len);
        return f;
}

void f_close(struct file *f) {
        if (!f)
                return;
        ino_write(f->ino, f->ino_id);
        f_destroy(f);
}

int f_create(__u32 *iid, __u32 parent, __u32 type)
{
        *iid = ino_alloc(parent, type);
        if (*iid == 0)  // can not be root
                return 0;
        return 1;
}

int f_delete(__u32 iid)
{
        return ino_recla(NULL, iid);
}

int dir_read(int i, char *file_name, __u32 *ino_id, struct file *f)
{
        if (i > (f_size(f) - 1) >> 5)
                return 0;
        if (f_sp(i << 5, f)) {
                if (f_read(file_name, file_name_max_len, f)) {
                        if (f_read(ino_id, 4, f)) {
                                return 1;
                        }
                }
        }
        //if (f_sp(i << 5, f) &&
        //    f_read(file_name, file_name_max_len, f) &&
        //    f_read(ino_id, 4, f) ) {
        //        return 1;
        //}
        return 0;
}

int dir_write(int i, char *file_name, __u32 *ino_id, struct file *f)
{
        if (i > (f_size(f) - 1) >> 5)
                return 0;
        if (f_sp(i << 5, f) &&
            f_write(file_name, file_name_max_len, f) &&
            f_write(ino_id, 4, f)) {
                return 1;
        }
        return 0;
}

int dir_add(char *file_name, __u32 ino_id, struct file *f)
{
        if (f_cs(f_size(f) + 32, f)) {
                if (f_sp(f_size(f) - 32, f)) {
                        if (f_write(file_name, file_name_max_len, f)) {
                                if (f_write(&ino_id, 4, f)) {
                                        return 1;
                                }
                        }
                }
        }
        //if (f_cs(f_size(f) + 32, f) &&
        //    f_sp(f_size(f) - 32, f) &&
        //    f_write(file_name, file_name_max_len, f) &&
        //    f_write(&ino_id, 4, f)) {
        //        return 1;
        //}
        return 0;
}

static int is_equal(char *fn1, char *fn2, int len) {
        int i;
        for (i = 0; i < len; ++i)
                if (fn1[i] != fn2[i])
                        return 0;
        return 1;
}

int dir_remove(char *file_name, struct file *f)
{
        int i;
        char name[file_name_max_len];
        __u32 id;
        while (dir_read(i, name, &id, f) != 0) {
                if (is_equal(file_name, name, file_name_max_len)) {
                        if (dir_size(f) - 1 != i) {
                            dir_read(dir_size(f)-1, name, &id, f);
                            dir_write(i, name, &id, f);
                        }
                        f_cs(f_size(f) - 32, f);
                }
                ++i;
        }
        return 0;
}
