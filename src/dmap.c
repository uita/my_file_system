#include "dmap.h"
#include "stdlib.h"

#define set(dirty, index)    dirty[index>>3] |= ((__u8)128)>>(index%8);
#define clear(dirty, index)  dirty[index>>3] &= !(((__u8)128)>>(index%8));
#define is_set(dirty, index) dirty[index>>3] & ((__u8)128)>>(index%8);

struct dmap *dmap_init(__u32 rc, __u32 rs, 
                (int)(*rfd)(__u32), (int)(*wtd)(void *, __u32))
{
        if (!rfd || !wtd)
                goto malloc_failed;
        struct dmap *dm = (struct dmap*)malloc(sizeof(struct dmap));
        if (!dm)
                goto malloc_failed;
        dm->row_count = rc;
        dm->row_size = rs;
        dm->read_row_from_disk = rfd;
        dm->write_row_to_disk = wtd;
        dm->tag = (__u32*)malloc(sizeof(__u32) * rc);
        if (!(dm->tag))
                goto malloc_failed;
        dm->dirty = malloc(rc / 8);
        if (!(dm->dirty))
                goto malloc_failed;
        dm->rows = malloc(sizeof(void*) * rc);
        if (!(dm->rows))
                goto malloc_failed;
        int i;
        void *p = NULL;
        for (i = 0; i < rc; ++i) {
                p = malloc(row_size);
                if (!p)
                        goto malloc_failed;
                (dm->rows)[i] = p;
        }
        return dm;
malloc_failed:
        dmap_uninit(dm);
        return NULL;
}

int dmap_uninit(struct dmap *dm)
{
        int i;
        if (!dm)
                return;
        if (dm->tag)
                free(dm->tag);
        if (dm->dirty)
                free(dm->dirty);
        if (dm->rows) {
                for (i = 0; i < dm->row_count; ++i) {
                        if ((dm->rows)[i])
                                free((dm->rows)[i])
                }
        }
        return;
}

void *read_row(__u32 id, struct dmap *dm)
{
        __u32 index = id % dm->row_count;
        __u32 tag = id / dm->row_count;
        void *p = dm->rows[index];
        if (tag != dm->tag[index]) {  // not match
                if (!(dm->read_row_from_disk(rows[index], id)))
                        return NULL;
                dm->tag[index] = tag;
                clear(dm->dirty, index);
        }
        return p;
}

/* not really write to disk but buffer */
int write_row(void* r, __u32 id, struct dmap *dm)
{
        __u32 index = id % dm->row_count;
        __u32 tag = id / dm->row_count;
        if (tag == dm->tag[index]) {  // match
                set(dm->dirty, index);
        } else {
                if (is_set(dm->dirty, index)) {  // is dirty, need to sync before covering
                        if (!write_row_to_disk(dm->rows[index], id))
                                return false;
                }
                dm->tag[index] = tag;
                clear(dm->dirty, index);
        }
        memcpy(dm->rows[index], r, dm->row_size);
        return true;
}

int tmatch(__u32 id, struct dmap *dm)
{
        if (id / dm->row_count == dm->tag[id % dm->row_count])
                return true;
        return false;
}

int sync(__u32 index, struct dmap *dm)
{
        __u32 id = dm->tag[index] + index;
        if (!write_row_to_disk(dm->rows[index], id))
                return false;
        return true;
}
