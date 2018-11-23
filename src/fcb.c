#include "fcb.h"
#include "fsb_alloc.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#define next_id(block,size) (*((uint32_t*)(((uint8_t*)block)+size-4)))
#define set_next_id(block,size,id) \
        (*((uint32_t*)(((uint8_t*)block)+size-4)))=id

struct visit_arg {
        //uint32_t begin;
        //uint32_t end;
        uint32_t now;
        uint32_t now_block_id;
        struct fcb *f;
        void *other_arg;
};


static bool visit_blocks(uint32_t root, uint32_t max_degree, 
                uint32_t begin, uint32_t count, visit_arg *args,
                bool (*visit)(struct visit_arg *),
                struct fcb *f)
{
        uint32_t bs, step, last_step, now_step, offset, i;
        uint8_t* buffer = NULL;
        bool error;
        if (!f || !visit || count == 0) {
                return false;
        }
        bs = f->block_size;
        step = bs / 4;
        last_step = step - 1;
        buffer = (uint8_t*)malloc(f->block_size);
        if (!buffer)
                return false;
        now_step = pow(step, max_degree - 1) * last_step;
        while (max_degree--) {
                error = !read_fsb(buffer, root);
                if (error)
                        return false;
                if (!max_degree) {
                        now_step = 1;
                } else {
                        now_step /= last_step;
                }
                offset = begin / now_step;
                root = *(((uint32_t*)buffer) + offset);
                begin -= offset * now_step;
        }
        /* visit the data indexed by the last index */
        i = begin;
        while (true) {
                for (; i < last_step && i < count; ++i) {
                        args->now_block_id = *(((uint32_t*)buffer) + i);
                        error = !visit(args);
                        if (error)
                                return false;
                        ++(args->now);
                }
                i = 0;
                if (count < last_step) {
                        count = 0;
                } else {
                        count -= last_step - begin;
                }
                if (count == 0)   // end
                        break;
                root = *(((uint32_t*)buffer) + last_step); // get next index block id
                read_fsb(buffer, root);
        }
        return true;
}

/* This function provides a recursive method to visit blocks in multistage indexes,
 * such as 'single_indirect', 'double_indirect' and so on. */
//static bool visit_blocks(uint32_t root, uint32_t degree, uint32_t max_degree, 
//                uint32_t begin, uint32_t count, visit_arg *args,
//                bool (*visit)(struct visit_arg *),
//                struct fcb *f)
//{
//        uint32_t step_size, i, steps, leftover, b, c;
//        uint8_t *buffer = NULL;
//        uint32_t *p;
//        if (!f || !visit)
//                return false;
//        buffer = (uint8_t*)malloc(f->block_size);
//        if (!buffer) {
//                return false;
//        } else {
//                read_fsb(buffer, root);
//        }
//        if (degree > max_degree) {  // arrive leaf node
//                args->now_block_id = root;
//                error = !visit(args);
//                if (error)
//                        return false;
//                ++(args->now);
//                return true;
//        }
//        steps = f->block_size / 4;
//        step_size = pow(steps, max_degree - degree);
//        /* visit the first step */
//        ///////////////////////////////// begin is wrong!!!!!!!!!!!!1
//        p = (uint32_t*)(buffer + s * 4);
//        c = step_size - begin % step_size;
//        if (count <= c)
//                c = count;
//        error = !visit_blocks(*p, degree+1, max_degree, begin, c, args, visit, f);
//        if (error)
//                return false;
//        if (count == c)     // it means there is only one step need to be visited in this degree
//                return true;
//        leftover = count - c;
//        b = begin + c;
//        p = p + 1;
//        /* middle steps */
//        while(leftover > step_size) {  // here may be wrong
//                error = !visit_blocks(*p, degree+1, max_degree, b, step_size, args, visit, f);
//                if (error)
//                        return false;
//                leftover = leftover - step_size;
//                p = p + 1;
//                b = b + step_size;
//        }
//        /* last steps */
//        error = !visit_blocks(*p, degree+1, max_degree, b, leftover, args, visit, f);
//        if (error)
//                return false;
//        free(buffer);
//        return true;
//}

static bool visit_direct(uint32_t begin, uint32_t count, void* args,
                bool (*visit)(struct visit_arg *),
                struct fcb *f)
{
        uint32_t i, end;
        bool error;
        if (begin + count > MAX_DIRECT_LEN || !f || !visit) {
                return false;
        }
        end = begin + count;
        for (i = begin; i < end; ++i) {
                args->now_block_id = f->direct[i];
                error = !visit(args);
                if (error) {
                        return false;
                }
                ++(args->now);
        }
        return true;
}

/* level == 0 means visiting direct
 * level == 1 means visiting single indirect
 * level == 2 means visiting double indirect
 * ... */
static bool visit_index(uint32_t level, uint32_t begin, uint32_t count, void* args,
                bool (*visit)(struct visit_arg *),
                struct fcb *f)
{
        uint32_t end, i, bc, root_id;
        bc = f->block_size / 4;
        bc = pow(bc, level);
        if (level == 0) {
                return visit_direct(begin, count, args, visit, f);
        }
        if (begin + count > bc || !f || !visit) {
                return false;
        }
        if (level == 1) {
                root_id = f->single_indirect;
        } else if (level == 2) {
                root_id = f->double_indirect;
        }
        return visit_blocks(root_id, level, begin, count, args, visit, f);
}

static bool visit_file_blocks(uint32_t begin, uint32_t count, void* args,
                bool (*visit)(struct visit_arg *),
                struct fcb *f)
{
        uint32_t i, bc, m[3], max, c;
        if (!f || !visit)
                return false;
        bc = f->block_size / 4;
        m[0] = MAX_DIRECT_LEN;
        m[1] = bc;
        m[2] = bc * bc;
        max = m[0] + m[1] + m[2];
        if (begin >= max || begin + count < begin || begin + count >= max)
                return false;
        ///////////////////////////
        //  ssdsafadfadsfasdfsdanfandnfandf
        ///////////
        //
        for (i = 0; i < 3; ++i) {
                if (begin + count <= m[i]) {
                        visit_index(i, begin, count, args, visit, f);
                        break;
                } else {
                        visit_index(i, begin, m[i], args, visit, f);
                        begin += m[i];
                        count -= m[i];
                }
        }
}

//static bool get_block_id(uint32_t *block_id, uint32_t n, struct fcb *f)
//{
//        uint8_t *buffer = NULL;
//        uint32_t index_size;
//        uint32_t next;
//        uint32_t step_size;
//        uint32_t offset;
//        if (!f)
//                return false;
//        index_size = f->block_size / 4;
//        if (n < MAX_DIRECT_LEN) {                   // direct
//                *block_id = f->direct[n];
//        } else {
//                buffer = (uint8_t*)malloc(sizeof(uint8_t) * block_size);
//                if (!buffer)
//                        return false;
//                n = n - MAX_DIRECT_LEN;
//                if (n < index_size) {               // single_indirect
//                        next = f->single_indirect;
//                        step_size = 1;
//                }
//                n = n - index_size;
//                if (n < index_size * index_size) {  // double_indirect
//                        next = f->double_indirect;
//                        step_size = index_size;
//                } else {
//                        return false;
//                }
//                while (degree--) {
//                        read_fsb(buffer, next);
//                        offset = n / step_size;
//                        next = *(((uint32_t*)buffer) + offset);
//                        n = n - offset * step_size;
//                        step_size = step_size / index_size;
//                }
//                *block_id = next;
//                if (buffer)
//                        free(buffer);
//        }
//        return true;
//}

//static bool visit_blocks(uint32_t first_block, uint32_t count, struct fcb* f, bool (*visit)(uint32_t, struct fcb*)) {
//        uint32_t first_block, last_block, i, bs, block_id;
//        uint8_t* buffer;
//        bs = f->block_size;
//        first_block = addr / (bs - 4); 
//        last_block = (addr + size - 1) / (bs - 4);
//        for (i = first_block; i <= last_block; ++i) {
//                error = !get_block_id(&block_id, i, f);
//                if (error) {
//                        return false;
//                }
//                if (i != last_block) {
//                        error = !visit(addr, size, f, block_id, false);
//                } else {
//                        error = !visit(addr, size, f, block_id, true);
//                }
//                if (error) {
//                        return false;
//                }
//        }
//}
//
//static bool visit_for_read(uint32_t addr, uint32_t size, struct fcb* f, uint32_t block_id, bool more_block)
//{
//        if (i == first_block || i == last_block) {
//                error = !get_block_id(&block_id, i, f);
//                if (error) {
//                        return false;
//                }
//                read_fsb(buffer, block_id);
//                if (i == first_block) {
//                        cpy_size = bs - addr % bs;
//                        to = ptr;
//                        from = buffer + bs - cpy_size;
//                } else {
//                        cpy_size = (addr + size) % bs;
//                        to = ptr + size - cpy_size;
//                        from = buffer;
//                }
//                memcpy(to, from, cpy_size);
//        } else {
//                error = !get_block_id(&block_id, i, f);
//                if (error)
//                        return false;
//                to = ptr - addr % bs + (i - first_block) * bs;
//                read_fsb(to , i);
//        }
//
//}
//

#define next_id(block,size) (*((uint32_t*)(((uint8_t*)block)+size-4)))
#define set_next_id(block,size,id) \
        (*((uint32_t*)(((uint8_t*)block)+size-4)))=id

bool read(uint8_t* ptr, uint32_t addr, uint32_t size, struct fcb* f)
{
        uint32_t block_id, n, bs;
        int count, i;
        uint8_t* buffer = NULL, to = NULL;
        bool error;
        if (!f || !ptr || f->size < addr + size) {
                return false;
        }
        bs = f->block_size;
        buffer = (uint8_t*)malloc(block_size);
        if (!buffer) {
                return false;
        }
        /* get the first block id */
        error = !get_block_id(&block_id, addr / bs, f);
        if (error) {
                return false;
        }
        read_fsb(buffer, block_id);
        count = (addr + size - 1) / bs - addr / bs - 1;
        /* 'count' = total blocks - 2 */
        if (count == -1) {
                memcpy((void*)ptr, (void*)(buffer + addr % bs), size);
        } else {
                memcpy((void*)ptr, (void*)(buffer + addr % bs), bs - addr % bs);
        }
        /* deal with blocks between the first and last block */
        to = ptr + bs - addr % bs;
        block_id = next_id(buffer, bs);
        for (i = 0; i < count; ++i) {
                error = !read_fsb(to, block_id);
                if (error) {
                        return false;
                }
                block_id = next_id(to, bs);
                to = to + bs;
        }
        /* deal with last block */
        if (count >= 0) {
                error = !read_fsb(buffer, block_id);
                if (error) {
                        return false;
                }
                memcpy(to, buffer, (addr + size) % bs);
        }
        return true;
}

/* variable=true, means the size of file wouble be change to (addr+size) */
bool write(uint8_t* ptr, uint32_t addr, uint32_t size, struct fcb* f, 
                bool variable)
{
        uint32_t block_id, n, bs;
        int count, i;
        uint8_t* buffer = NULL, to = NULL;
        bool error;
        if (!f || !ptr || f->size < addr + size) {
                return false;
        }
        if (variable) {
                change_len(addr, size, f);
        }
        bs = f->block_size;
        buffer = (uint8_t*)malloc(block_size);
        if (!buffer) {
                return false;
        }
        /* get the first block id */
        error = !get_block_id(&block_id, addr / bs, f);
        if (error) {
                return false;
        }
        read_fsb(buffer, block_id);
        count = (addr + size - 1) / bs - addr / bs - 1;
        /* 'count' = total blocks - 2 */
        if (count == -1) {
                memcpy((void*)ptr, (void*)(buffer + addr % bs), size);
        } else {
                memcpy((void*)ptr, (void*)(buffer + addr % bs), bs - addr % bs);
        }
        /* deal with blocks between the first and last block */
        to = ptr + bs - addr % bs;
        block_id = next_id(buffer, bs);
        for (i = 0; i < count; ++i) {
                error = !read_fsb(to, block_id);
                if (error) {
                        return false;
                }
                block_id = next_id(to, bs);
                to = to + bs;
        }
        /* deal with last block */
        if (count >= 0) {
                error = !read_fsb(buffer, block_id);
                if (error) {
                        return false;
                }
                memcpy(to, buffer, (addr + size) % bs);
        }
        return true;
}

bool clear(struct fcb* f);
bool resize(uint32_t size);

uint32_t max_data_size(struct fcb *f)
{
        uint32_t index_size = f->block_size/4;
        return (MAX_DIRECT_LEN+index_size+index_size*index_size)*(f->block_size-4);
}
uint32_t total_block_count(struct fcb *f)
