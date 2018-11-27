#include "index.h"
#include "stdint.h"

struct index_iterator *create(uint32_t pos, struct fcb *f)
{
        struct index_iterator *ii = NULL;
        if (!f || pos >= f->data_block_count)
                return NULL;
        ii = (struct index_iterator*)malloc(sizeof(struct index_iterator));
        if (!ii)
                return NULL;
        ii->buffer = malloc(f->block_size);
        if (!(ii->buffer) || !updata_buffer(ii)) {
                free(ii);
                ii = NULL;
        }
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

bool move_back(struct index_iterator *ii);
bool move_front(struct index_iterator *ii);
bool set_pos(uint32_t pos, struct index_iterator *ii);
uint32_t get_data(struct index_iterator *ii);
bool add_leaves(uint32_t n, struct fcb *f)
{
        if (f->block_count )
}
bool remove_leaves(uint32_t n, struct fcb *f);

/* search down */
static bool updata_buffer(struct index_iterator *ii);
static bool add(uint32_t root, uint32_t *leftover, uint32_t isize;
                uint32_t degree, uint32_t max_degree, struct fcb *f)
{
        uint32_t i, len, step_size, tail, bid;
        bool error = true;
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
        return error;
}
