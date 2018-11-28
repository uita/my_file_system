#include "index.h"
#include "stdint.h"

struct index_iterator *create(struct fcb *f)
{
        struct index_iterator *ii = NULL;
        if (!f)
                return NULL;
        ii = (struct index_iterator*)malloc(sizeof(struct index_iterator));
        if (!ii)
                return NULL;
        memset(ii, 0, sizeof(struct index_iterator));
        ii->f = f;
        return ii;
}

void destroy(struct index_iterator *ii)
{
        if (!ii)
                return;
        if (ii->buffer)
                free(ii->buffer);
        free(ii);
}

bool set_pos(uint32_t pos, struct index_iterator *ii) 
{
        if (!ii || !(ii->f))
                return false;
        if (need_updata_buffer(pos, ii)) {
                if (!updata_buffer(pos, ii))
                        return false;
        }
        ii->pos = pos;
        ii->begin = get_leaf_begin();
        return true;
}

uint32_t get_data(struct index_iterator *ii);
bool add_leaves(uint32_t n, struct fcb *f)
{
        if (f->block_count )
}
bool remove_leaves(uint32_t n, struct fcb *f);

static bool need_updata_buffer(uint32_t pos, struct index_iterator *ii)
{
}
static bool updata_buffer(uint32_t pos, struct index_iterator *ii)
{
        bool re;
        if (is_next_block(ii->pos, pos)) {
                re = updata_from_tail(ii);
        } else {
                re = updata_from_root(pos, ii);
        }
        return re;
}
static bool updata_from_tail(struct index_iterator *ii)
{}
static bool updata_from_root(uint32_t pos, struct index_iterator *ii)
{}

static uint32_t get_indirect(uint32_t pos, uint32_t len)
{
        uint32_t re = 0;
        if (pos < MAX_DIRECT_LEN)
                return re;
        pos -= MAX_DIRECT_LEN;
        --len;
        while (pos >= len) {
                pos -= len;
                len *= (len + 1);
                ++re;
        }
        return re;
}

static uint32_t get_inner_pos(uint32_t pos, uint32_t len)
{
        if (pos < MAX_DIRECT_LEN)
                return pos;
        pos -= MAX_DIRECT_LEN;
        --len;
        while (pos >= len) {
                pos -= len;
                len *= (len + 1);
        }
        return pos;
}

static uint32_t get_root(uint32_t pos, uint32_t len, struct fcb* f)
{
        if (pos < MAX_DIRECT_LEN)
                return f->direct[pos];
        uint32_t idt = get_indirect(pos, len);
        if (idt == 1)
                return f->single_indirect;
        if (idt == 2)
                return f->double_indirect;
        return 0;
}

static bool is_neighbour(uint32_t pos1, uint32_t pos2, uint32_t len)
{

}

static bool read_leaves(uint32_t n, uint32_t root, uint32_t leaf_degree,
                void* buffer, struct fcb *f)
{
        if (!f || !buffer)
                return false;
        uint32_t len = f->block_size / 4;
        uint32_t step_size = pow(len, leaf_degree - 2) * (len - 1);
        bool error = false;
        while (leaf_degree--) {
                error = !read_fsb(root, buffer);
                if (error)
                        return false;
                if (leaf_degree == 0)
                        break;
                root = *(((uint32_t*)buffer)+n/step_size);
                step_size /= len;
                n -= n / step_size * step_size;
        }
        return true;
}

static bool add(uint32_t root, uint32_t *leftover, uint32_t isize;
                uint32_t degree, uint32_t max_degree, struct fcb *f)
{
        uint32_t i, len, step_size, tail, bid;
        bool error = false;
        uint32_t *buffer = NULL;
        len = f->block_size / 4;
        if (degree != max_degree) {
                step_size = pow(len, max_degree - degree - 1) * (len - 1);
        } else {
                step_size = 1;
        }
        tail = isize / step_size;
        if (isize % step_size == 0)
                tail--;
        buffer = (uint32_t*)malloc(f->block_size);
        if (!buffer)
                return false;
        error = !read_fsb(root, buffer);
        if (error) 
                goto exit;
        if (degree == max_degree) { // leaf node
                --len;
                for (i = tail + 1; i < len; ++i) {
                        if ((*leftover) != 0) {
                                error = !allocate_fsb(&bid);
                                if (error)
                                        goto exit;
                                buffer[i] = bid;
                                --(*leftover);
                        }
                }
        } else {  // not leaf node
                for (i = tail, i < len; ++i) {
                        error = !add(buffer[i], leftover, isize-i*step_size, 
                                        degree+1, max_degree, f);
                        if (error || *leftover == 0)
                                goto exit;
                        error = !allocate_fsb(&bid);
                        if (error)
                                goto exit;
                        buffer[i+1] = bid;
                }

        }
        error = !write_fsb(root, buffer);
exit:
        free(buffer);
        return !error;
}
