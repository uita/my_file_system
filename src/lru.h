#ifndef LRU_H
#define LRU_H

#include "list.h"
#include "type.h"

struct lru_que {
        __u32 max_len;
        __u32 buf_size;
        struct list *ls;
        (int)(*read_from_disk)(void*, __u32);
        (int)(*write_to_disk)(void*, __u32);
};

struct lru_buf {
        __u32 id;
        __u32 info;   // containing information of dirty-bit and read-count
        void *buf;
        struct list_node var;
};

#define is_dirty(lb) (lb->info & 0x80000000)
#define get_rcount(lb) (lb->info & 0x7fffffff)

struct lru_que *lru_init(__u32 ml, __u32 bs, 
                (int)(*rfd)(void *, __u32), (int)(*wtd)(void *, __u32));

void lru_uninit(struct lru_que *lq);

void *lru_read(__u32 id, struct lru_que *lq);
/* not really write to disk but buffer */
int lru_write(void* b, __u32 id, struct lru_que *lq);

__u32 lru_rcount(lru_buf *lb);
//int lru_ismatch(__u32 id, struct lru_que *lq);

//int sync(struct lru_que *lq);

#endif
