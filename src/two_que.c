#include "two_que.h"
#include "list.h"
#include "type.h"
#include "stdio.h"
#include "stdlib.h"

#define set_dirty(node) node->info |= 0x80000000;
#define clear_dirty(node) node->info &= 0x7fffffff;

#define add_rcount(node) \
        if (node->info << 1 != 0xfffffffe) { \
                node->info += 1; \
        }

#define getn_from_two_list(node, visit, tq) \
        remove_container(visit, node, tq->ls2, struct tq_node, var); \
        if (!node) \
                remove_container(visit, node, tq->ls1, struct tq_node, var);

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

static void print_tq(struct two_que *tq) {
        struct tq_node *node = NULL;
        printf("[ls1] : ");
        printf("%u : ", tq->ls1->size);
        for_each(node, tq->ls1, struct tq_node, var) {
                if (node)
                        printf("%u ", node->id);
        }
        printf("\n");
        printf("[ls2] : ");
        printf("%u : ", tq->ls2->size);
        for_each(node, tq->ls2, struct tq_node, var) {
                if (node)
                        printf("%u ", node->id);
        }
        printf("\n");
}

static struct tq_node *get_node(__u32 id, struct two_que *tq)
{
        struct tq_node *node = NULL;
#define id_equal(n) ((n == NULL ? 0 : ((struct tq_node*)n)->id == id))
        getn_from_two_list(node, id_equal, tq);
        if (node) {
                add_rcount(node);
        } else {
                node = alloc_node(id, tq->buf_size);
                if (!tq->read_from_disk(node->buf, id)) {
                        free_node(node);
                        node = NULL;
                }
        }
        return node;
}

static void add_node(struct tq_node *node, struct two_que *tq)
{
        struct list *l = get_rcount(node) > (__u32)1 ? tq->ls2 : tq->ls1;
        __u32 max_len = l == tq->ls1 ? tq->max_len1 : tq->max_len2;
        struct tq_node *head = NULL;
        if (list_size(l) >= max_len) {
                //print_tq(tq);
                remove_head(head, l, struct tq_node, var);
                //print_tq(tq);
                tq->write_to_disk(head->buf, head->id);
                free_node(head);
        }
                //print_tq(tq);
        add_tail(node, l, struct tq_node, var);
                //print_tq(tq);
}

static struct tq_node *read_node(__u32 id, struct two_que *tq)
{
        struct tq_node *node = get_node(id, tq);
        if (!node)
                return NULL;
        add_node(node, tq);
        return node;
}

void tq_destroy(struct two_que *tq)
{
        if (!tq)
                return;
        struct tq_node *node = NULL;
        while (true) {
#define alltrue(con) (1)
                getn_from_two_list(node, alltrue, tq);
                if (!node)
                        break;
                tq->write_to_disk(node->buf, node->id);
                free_node(node);
        }
        destroy_list(tq->ls1);
        destroy_list(tq->ls2);
        free(tq);
}

struct two_que *tq_create(__u32 bs, __u32 ml1, __u32 ml2,
                int (*rfd)(void *, __u32), int (*wtd)(void *, __u32))
{
        struct two_que *tq = (struct two_que *)malloc(sizeof(struct two_que));
        if (!tq)
                return NULL;
        create_list(tq->ls1, struct tq_node);
        create_list(tq->ls2, struct tq_node);
        if (!tq->ls1 || !tq->ls2)
                tq_destroy(tq);
        tq->buf_size = bs;
        tq->max_len1 = ml1;
        tq->max_len2 = ml2;
        tq->read_from_disk = rfd;
        tq->write_to_disk = wtd;
        return tq;
}

int tq_read(void* data, __u32 id, struct two_que *tq)
{
        if (!tq)
                return 0;
        struct tq_node *node = read_node(id, tq);
        if (!node)
                return 0;
        memcpy(data, node->buf, tq->buf_size);
        return 1;
}

/* not really write to disk but write to buffer */
int tq_write(void* data, __u32 id, struct two_que *tq)
{
        if (!tq)
                return 0;
        struct tq_node *node = read_node(id, tq);
        if (!node)
                return 0;
        set_dirty(node);
        memcpy(node->buf, data, tq->buf_size);
        return 1;
}

/* Path = id + offset + offset + ... 
 * Len = the number of 'offsets'
 * For indexes */

static struct tq_node *parse_path(__u32 *path, int len, struct two_que *tq)
{
        struct tq_node *node = NULL;
        int i;
        node = read_node(path[0], tq);
        if (node) {
                for (i = 1; i <= len; ++i) {
                        node = read_node(*(((__u32 *)(node->buf))+path[i]), tq);
                        if (!node)
                                break;
                }
        }
        return node;
}

int tq_read_wp(void *data, __u32 *path, int len, struct two_que *tq)
{
        struct tq_node *node = parse_path(path, len, tq);
        if (!node)
                return 0;
        memcpy(data, node->buf, tq->buf_size);
        return 1;
}

int tq_write_wp(void *data, __u32 *path, int len, struct two_que *tq)
{
        struct tq_node *node = parse_path(path, len, tq);
        if (!node)
                return 0;
        memcpy(node->buf, data, tq->buf_size);
        return 1;
}
