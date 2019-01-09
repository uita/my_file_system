#ifndef DMAP_H
#define DMAP_H

#include "type.h"

struct dmap {
        __u32 row_count;
        __u32 row_size;
        __u32 *tag;
        void *dirty;
        void **rows;
        (int)(*read_row_from_disk)(void*, __u32);
        (int)(*write_row_to_disk)(void*, __u32);
};

struct dmap *dmap_init(__u32 rc, __u32 rs, 
                (int)(*rfd)(void *, __u32), (int)(*wtd)(void *, __u32));
void dmap_uninit(struct dmap *dm);

/* the functions below don't check validity of 'id' */

void *read_row(__u32 id, struct dmap *dm);
/* not really write to disk but buffer */
int write_row(void* r, __u32 id, struct dmap *dm);

// tag match
int tmatch(__u32 id, struct dmap *dm);
int sync(__u32 index, struct dmap *dm);

#endif
