#ifndef TWO_QUE_H
#define TWO_QUE_H

#include "type.h"
#include "list.h"

struct two_que {
        __u32 max_len1, max_len2;
        __u32 buf_size;
        struct list *ls1, *ls2;
        int (*read_from_disk)(void*, __u32);
        int (*write_to_disk)(void*, __u32);
};

struct tq_node {
        __u32 id;
        __u32 info;  // containing information of dirty-bit and read-count
        void *buf;
        struct list_node var;
};

#define is_dirty(node) (node->info & 0x8000000)
#define get_rcount(node) (node->info & 0x7fffffff)

struct two_que *tq_create(__u32 bs, __u32 ml1, __u32 ml2,
                int (*rfd)(void *, __u32), int (*wtd)(void *, __u32));

void tq_destroy(struct two_que *tq);

int tq_read(void *data, __u32 id, struct two_que *tq);
/* not really write to disk but write to buffer */
int tq_write(void *data, __u32 id, struct two_que *tq);

/* Path = id + offset + offset + ... 
 * Len = the number of 'offsets'
 * For indexes
 * In terms of blocks */
int tq_read_wp(void *data, __u32 *path, int len, struct two_que *tq);
int tq_write_wp(void *data, __u32 *path, int len, struct two_que *tq);

#endif
