#include "lru.h"
#include "type.h"

#define set_dirty(lb) lb->info |= 0x80000000;
#define clear_dirty(lb) lb->info &= 0x7fffffff;

struct lru_que *lru_init(__u32 ml, __u32 bs, 
                (int)(*rfd)(void *, __u32), (int)(*wtd)(void *, __u32))
{
        struct lru_que *lq = NULL;
        if (!rfd || !wtd)
                goto malloc_failed;
        lq = (struct lru_que *)malloc(sizeof(struct lru_que));
        if (!lq)
                goto malloc_failed;
        create_list(ls, struct lru_buf);
        if (!ls)
                goto malloc_failed;
        lq->max_len = ml;
        lq->buf_size = bs;
        read_from_disk = rfd;
        write_to_disk = wtd;
        return lq;
malloc_failed:
        lru_uninit(lq);
        return NULL;
}

void lru_uninit(struct lru_que *lq)
{
        struct lru_buf *b = NULL;
        if (!lq)
                return;
        if (lq->ls) {
                while (list_size(lq->ls)) {
                        remove_head(b, lq->ls, struct lru_buf, var);
                        free(b);
                }
                destroy_list(ls);
        }
        free(lq);
}

void *lru_read(__u32 id, struct lru_que *lq)
{
        struct lru_buf *lb;
#define is_equal(b) (b->id == id)
        remove_container(is_equal, lb, lq->ls, struct lru_buf, var);
        if (lb) {  // found and removed
                add_rcount(lb);
                add_buf_to_tail(b, struct lru_que *lq)
                return lb->buf;
        } else {
                lb = create_lru_buf(lq);
                if (read_from_disk(lb->buf, id)) {
                        lb->id = id;
                        add_buf_to_tail(lb, struct lru_que *lq)
                        return lb->buf;
                } else {
                        destroy_lru_buf(lb);
                }
        }
        return NULL;
}

/* not really write to disk but buffer */
int lru_write(void* b, __u32 id, struct lru_que *lq)
{
        struct lru_buf *lb = NULL;
#define is_equal(b) (b->id == id)
        remove_container(is_equal, lb, lq->ls, struct lru_buf, var);
        if (lb) {  // found
                memcpy(lb->buf, b, lq->buf_size);
                add_rcount(lb);
                add_buf_to_tail(lb, struct lru_que *lq)
                set_dirty(lb)
        } else {  // not found
                lb = create_lru_buf(lq);
                if (write_to_disk(b, id)) {
                        memcpy(lb, b, lq->buf_size);
                        lb->id = id;
                        add_buf_to_tail(lb, struct lru_que *lq)
                } else {
                        destroy_lru_buf(lb);
                        return 0;
                }
        }
        return 1;
}

static struct lru_buf *create_lru_buf(struct lru_que *lq)
{
        struct lru_buf *b = (struct lru_buf *)malloc(sizeof(struct lru_buf));
        if (!b)
                return NULL;
        memset(b, 0, sizeof(struct lru_buf));
        b->buf = malloc(lq->buf_size);
        if (!b->buf) {
                free(b);
                return NULL;
        }
        return b;
}

static void destroy_lru_buf(struct lru_buf *b)
{
        if (!b)
                return;
        if (b->buf)
                free(b->buf);
        free(b);
}

/* the highest bit of 'info' is the dirty bit */
static void add_rcount(struct lru_buf* lb)
{
        if (lb->info << 1 == 0xfffffffe)
                return;
        lb->info += 1;
}

static void add_buf_to_tail(struct lru_buf *lb, struct lru_que *lq)
{
        struct lru_buf *tmp;
        if (lq->ls->size == lq->max_len) {
                remove_head(tmp, lq->ls, struct lru_buf, var);
        }
        add_tail(lb, lq->ls, struct lru_buf, var);
        destroy_lru_buf(tmp);
}
