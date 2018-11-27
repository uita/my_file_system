#include "fcb.h"
#include "fsb_alloc.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

struct visit_arg {
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
        while (true) {
                error = !read_fsb(buffer, root);
                if (error)
                        return false;
                now_step /= last_step;
                if (!(--max_degree)) {
                        break;
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
        free(buffer);
        return true;
}

/* if n < MAX_DIRECT_LEN , this function will fail. */
static uint32_t get_parent_of_leaf(uint32_t n, struct fcb* f) {
        n -= MAX_DIRECT_LEN;
        //////////////////
}

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
        m[1] = bc - 1;
        m[2] = m[1] * bc;
        max = m[0] + m[1] + m[2];
        if (begin >= max || begin + count < begin || begin + count >= max)
                return false;
        for (i = 0; i < 3; ++i) {
                if (begin + count <= m[i]) {
                        visit_index(i, begin, count, args, visit, f);
                        break;
                } else {
                        visit_index(i, begin, m[i], args, visit, f);
                        begin = 0;
                        count -= m[i];
                }
        }
}

struct rw_arg {
        uint32_t count;  // remaining bytes
        uint32_t offset;
        uint8_t *ptr;
}

static bool visit_for_read(struct visit_arg * arg)
{
        struct rw_arg* og = (struct rw_arg*)(arg->other_arg);
        uint32_t bs = arg->f->block_size;
        uint32_t len;
        uint8_t *buffer = NULL;
        /* If can not read a block to ptr immediately, 
         * it need a buffer to store a block for fear of out_of_range */
        if (og->offset != 0 || count < bs) {
                buffer = (uint8_t*)malloc(bs);
                if (!buffer)
                        return false;
                read_fsb(buffer, arg->now_block_id);
                len = og->offset + count <= bs ? count : bs - og->offset;
                memcpy((void*)(og->ptr), (void*)(buffer + og->offset), len);
                free(buffer);
        } else {
                read_fsb(og->ptr, arg->now_block_id);
                len = bs;
        }
        og->count -= len;
        og->offset = 0;
        og->ptr += len;
        return true;
}

bool read(uint8_t* ptr, uint32_t addr, uint32_t size, struct fcb* f)
{
        uint32_t begin, count;
        struct visit_arg arg;
        struct rw_arg og;
        if (!ptr || addr + size > max_data_size(f)) {
                return false;
        }
        begin = addr / f->block_size;
        count = (addr + size) / f->block_size - begin + 1;
        arg.now = 0;
        arg.now_block_id = 0;
        arg.f = f;
        arg.other_arg = (void*)(&og);
        og.count = size;
        og.offset = addr % f->block_size;
        og.ptr = ptr;
        return visit_file_blocks(begin , count, arg, visit_for_read, f);
}

//struct visit_arg {
//        uint32_t now;
//        uint32_t now_block_id;
//        struct fcb *f;
//        void *other_arg;
//};
//static bool visit_file_blocks(uint32_t begin, uint32_t count, void* args,
//                bool (*visit)(struct visit_arg *),
//                struct fcb *f)
//struct rw_arg {
//        uint32_t count;  // remaining bytes
//        uint32_t offset;
//        uint8_t *ptr;
//}

bool visit_for_write(struct visit_arg *arg)
{
        struct rw_arg* og = (struct rw_arg*)(arg->other_arg);
        uint32_t bs = arg->f->block_size;
        uint32_t len;
        uint8_t *buffer = NULL;
        /* If can not write a block to extenal storage immediately, 
         * it need a buffer to store a block for fear of out_of_range,
         * and nead to read the block before write a part of it. */
        if (og->offset != 0 || count < bs) {
                buffer = (uint8_t*)malloc(bs);
                if (!buffer)
                        return false;
                read_fsb(buffer, arg->now_block_id);
                len = og->offset + count <= bs ? count : bs - og->offset;
                memcpy((void*)(buffer + og->offset), (void*)(og->ptr), len);
                write_fsb(buffer, arg->now_block_id);
                free(buffer);
        } else {
                write_fsb(og->ptr, arg->now_block_id);
                len = bs;
        }
        og->count -= len;
        og->offset = 0;
        og->ptr += len;
        return true;
}

/* variable=true, means the size of file wouble be adjust to (addr+size) */
bool write(uint8_t* ptr, uint32_t addr, uint32_t size, struct fcb* f, 
                bool variable)
{
        uint32_t begin, count;
        struct visit_arg arg;
        struct rw_arg og;
        if (!ptr || (!variable && addr + size > max_data_size(f))) {
                return false;
        }
        if (variable) {
                error = !adjust_file_size(addr + size, f);
                if (error)
                        return false;
        }
        begin = addr / f->block_size;
        count = (addr + size) / f->block_size - begin + 1;
        arg.now = 0;
        arg.now_block_id = 0;
        arg.f = f;
        arg.other_arg = (void*)(&og);
        og.count = size;
        og.offset = addr % f->block_size;
        og.ptr = ptr;
        return visit_file_blocks(begin , count, arg, visit_for_write, f);
}



static bool adjust_file_size(uint32_t size, struct fcb* f)
{

}

static bool add_blocks(uint32_t num, struct fcb* f)
{
        if (f->size / f->block_size)
}

/* remove blocks from tail to head */
static bool remove_blocks(uint32_t num, struct fcb* f)
{

}

bool clear(struct fcb* f);
bool resize(uint32_t size);

uint32_t max_data_size(struct fcb *f)
{
        if (!f)
                return 0;
        return max_data_block(f) * f->block_size;
}

static uint32_t max_data_block(struct fcb *f)
{
        uint32_t is = f->block_size/4;
        return (MAX_DIRECT_LEN+(is+1)*(is-1));
}

/* include blocks for indexes */
uint32_t fcb_block_count(struct fcb *f)
{

}
