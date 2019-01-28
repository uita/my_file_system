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
        if (pos >= MAX_DIRECT_LEN) {
                if (!(ii->buffer))
                        buffer = malloc(ii->f->block_size);
                if (!buffer)
                        return false;
        }
        if (need_update_buffer(pos, ii)) {
                if (!update_buffer(pos, ii))
                        return false;
                ii->lid = leaf_id(pos, ii->f->block_size / 4);
        }
        ii->pos = pos;
        if (pos < MAX_DIRECT_LEN)
                ii->lid = 0;
        return true;
}

uint32_t get_data(struct index_iterator *ii)
{
        uint32_t pos = ii->pos;
        if (pos < MAX_DIRECT_LEN)
                return ii->f->direct[ii->pos];
        pos -= MAX_DIRECT_LEN;
        return *(((uint32_t*)(ii->buffer)) + pos % (ii->f->block_size / 4 - 1));
}

bool add_leaves(uint32_t n, struct fcb *f)
{}
bool remove_leaves(uint32_t n, struct fcb *f)
{}

/*
static bool need_update_buffer(uint32_t pos, struct index_iterator *ii)
static bool update_buffer(uint32_t pos, struct index_iterator *ii)
static bool update_from_tail(struct index_iterator *ii)
static bool update_from_root(uint32_t pos, struct index_iterator *ii)

static bool read_leaf_from_tree(uint32_t ipos, uint32_t root, uint32_t leaf_depth,
static bool is_next_leaf(uint32_t pos, struct index_iterator *ii)
static uint32_t root(uint32_t pos, uint32_t len, struct fcb* f)
static uint32_t inner_pos(uint32_t pos, uint32_t len)
static uint32_t leaf_id(uint32_t pos, uint32_t len)
static uint32_t tree_depth(uint32_t pos, uint32_t len)
*/

static bool need_update_buffer(uint32_t pos, struct index_iterator *ii)
{
        uint32_t len;
        if (pos < MAX_DIRECT_LEN)
                return false;
        len = ii->f->block_size / 4;
        if (ii->lid == leaf_id(pos, len))
                return false;
        return true;
}

static bool update_buffer(uint32_t pos, struct index_iterator *ii)
{
        bool re;
        if (is_next_leaf(pos, ii)) {
                re = update_from_tail(ii);
        } else {
                re = update_from_root(pos, ii);
        }
        return re;
}

static uint32_t leaf_id(uint32_t pos, uint32_t len)
{
        if (pos < MAX_DIRECT_LEN)
                return 0;
        return (pos - MAX_DIRECT_LEN) / len + 1;
}

static bool update_from_tail(struct index_iterator *ii)
{
        uint32_t len = ii->f->block_size / 4;
        if (ii->pos < MAX_DIRECT_LEN)
                return read_fsb(ii->f->single_indirect, ii->buffer);
        else
                return read_fsb(*(((uint32_t*)(ii->buffer))+len-1), ii->buffer);
}

static bool update_from_root(uint32_t pos, struct index_iterator *ii)
{
        uint32_t len = ii->f->block_size / 4;
        return read_leaf_from_tree(inner_pos(pos), root(pos, len, ii->f),
                        tree_depth(pos, len), ii->buffer, ii->f);	// 'root' will never be 0
}

static uint32_t tree_depth(uint32_t pos, uint32_t len)
{
        uint32_t re = 1;
        if (pos < MAX_DIRECT_LEN)
                return 0;
        pos -= MAX_DIRECT_LEN;
        --len;
        while (pos >= len) {
                pos -= len;
                len *= (len + 1);
                ++re;
        }
        return re;
}

static uint32_t inner_pos(uint32_t pos, uint32_t len)
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

static uint32_t root(uint32_t pos, uint32_t len, struct fcb* f)
{
        uint32_t idt = tree_depth(pos, len);
        if (idt == 1)
                return f->single_indirect;
        if (idt == 2)
                return f->double_indirect;
        return 0;
}

static bool is_next_leaf(uint32_t pos, struct index_iterator *ii)
{
        if (ii->lid + 1 == leaf_id(pos, len))
                return true;
        return false;
}

static bool read_leaf_from_tree(uint32_t ipos, uint32_t root, uint32_t leaf_depth,
                void* buffer, struct fcb *f)
{
        if (!f || !buffer)
                return false;
        uint32_t len = f->block_size / 4;
	uint32_t step_size = 1;
	uint32_t offset;
        bool error = false;
	if (leaf_depth > 1) {
		step_size = len - 1;
		if (leaf_depth > 2)
			step_size *= pow(len, leaf_depth - 2);
	}
        while (true) {
                error = !read_fsb(root, buffer);
                if (error)
                        return false;
                if (--leaf_depth == 0)
                        break;
		offset = ipos / step_size;
                root = *(((uint32_t*)buffer) + offset);
                ipos -= offset * step_size;
                step_size /= len;
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
