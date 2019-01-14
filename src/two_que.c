#include "two_que.h"

#define set_dirty(node) node->info |= 0x80000000;
#define clear_dirty(node) node->info &= 0x7fffffff;

#define add_rcount(node) \
        if (node->info << 1 != 0xfffffffe) { \
                node->info += 1; \
        }

#define getn_from_two_list(node, visit, tq) \
        remove_container(visit, node, tq->ls1, struct tq_node, var); \
        if (!node) \
                remove_container(visit, node, tq->ls2, struct tq_node, var);

struct two_que *tq_init(__u32 bs, __u32 ml1, __u32 ml2
                (int)(*rfd)(void *, __u32), (int)(*wtd)(void *, __u32))
{
        struct two_que *tq = (struct two_que *)malloc(sizeof(struct two_que));
        if (!tq)
                return NULL;
        create_list(tq->ls1);
        create_list(tq->ls2);
        if (!tq->ls1 || !tq->ls2)
                tq_uninit(tq);
        tq->buf_size = bs;
        tq->max_len1 = ml1;
        tq->max_len2 = ml2;
        tq->read_from_disk = rfd;
        tq->write_to_disk = wtd;
        return tq;
}

void tq_uninit(struct two_que *tq)
{
        if (!tq)
                return;
        destroy_list(tq->ls1);
        destroy_list(tq->ls2);
        free(tq);
}

void *tq_read(__u32 id, struct tq_que *tq)
{
        if (!tq)
                return NULL;
        struct tq_node *node = read_node(id, tq);
        if (!node)
                return NULL;
        add_node(node, tq);
        return node->buf;
}

//#define getn_from_two_list(node, visit, tq)

/* not really write to disk but write to buffer */
int tq_write(void* data, __u32 id, struct tq_que *tq)
{
        if (!tq)
                return 0;
        struct tq_node *node = read_node(id, tq);
        if (!node)
                return 0;
        add_node(node, tq);
        set_dirty(node);
        memcpy(node->buf, data, tq->buf_size);
        return 1;
}

static struct tq_node *alloc_node(__u32 id, __u32 buf_size)
{
        struct tq_node *node = (struct tq_node *)malloc(sizeof(struct tq_node));
        if (!node)
                return NULL;
        node->buf = malloc(buf_size);
        if (!node->buf) {
                free(node);
                return NULL;
        }
        node->id = id;
        node->info = 1;  // the first visit
        return node;
}

static void free_node(struct tq_node *node)
{
        if (!node)
                return;
        if (node->buf)
                free(node->buf);
        free(node);
}

static struct tq_node *read_node(__u32 id, struct two_que *tq)
{
        struct tq_node *node = NULL;
#define id_equal(node) (node->id == id);
        getn_from_two_list(node, id_equal, tq);
        if (node) {
                add_rcount(node);
        } else {
                node = alloc_node(id, tq->buf_size);
                if (!tq->read_from_disk(node->buf, id))
                        free_node(node);
        }
        return node;
}

static void add_node(struct tq_node *node, struct two_que *tq)
{
        struct list *l = get_rcount(node) > 1 ? tq->ls2 : tq->ls1;
        __u32 max_len = l == tq->ls1 ? max_len1 : max_len2;
        struct tq_node *head = NULL;
        if (list_size(l) >= max_len) {
                head = get_head(l, struct tq_node, var);
                tq->write_to_disk(head->buf, head->id);
        }
        add_tail(node, l, struct tq_node, var);
}
