#include "inode.h"
#include "super_block.h"
#include "balloc.h"
#include "ialloc.h"
#include "bbuf.h"
#include "ibuf.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "time.h"

#define _bc (_block_size/4)
#define _max_single _bc
#define _max_double _bc*_bc
#define _max_triple _bc*_bc*_bc
#define _max_inode_block_count (_max_direct+_max_single+_max_double+_max_triple)
#define _max_inode_size (_max_inode_block_count*_block_size)
#define _max_direct MAX_DIRECT_LEN
#define _max_depth(i) (i < MAX_DIRECT_LEN ? 0 : i - MAX_DIRECT_LEN + 1)
static __u32 _block_size;
static __u32 _max_inode_num;
static __u32 _max_block_num;

int ino_init(struct super_block *sb)
{
        _block_size = sb->block_size;
        _max_inode_num = sb->max_inode_num;
        _max_block_num = sb->max_block_num;
}

void ino_uninit()
{

}

/* return path len */
static int get_path(__u32 *path, __u32 n, struct inode *ino)
{
        __u32 a = _bc;
        __u32 b = a, i;
        if (n < MAX_DIRECT_LEN) {
                path[0] = ino->index[n];
                return 0;
        }
        n -= MAX_DIRECT_LEN;
        for (i = 0; n >= b; ++i) {
                n -= b;
                b *= a;
        }
        path[0] = ino->index[MAX_DIRECT_LEN + i];
        // get offset
        b /= a;  // b == step size
        for (i = 1; b != 0; ++i) {
                path[i] = n / b;
                n -= b * path[i];
                b /= a;
        }
        return i - 1;
}

int ino_read_data(void *block, __u32 n, struct inode *ino)
{
        if (n >= ino->block_count)
                return 0;
        __u32 *path = (__u32 *)malloc(sizeof(__u32) * 4);
        int len = get_path(path, n, ino);
        if (bbuf_read_wp(block, path, len)) {
                free(path);
                return 1;
        } else {
                free(path);
                return 0;
        }
}

int ino_write_data(void *block, __u32 n, struct inode *ino)
{
        if (n >= ino->block_count)
                return 0;
        __u32 *path = (__u32 *)malloc(sizeof(__u32) * 4);
        int len = get_path(path, n, ino);
        if (bbuf_write_wp(block, path, len)) {
                free(path);
                return 1;
        } else {
                free(path);
                return 0;
        }
}

static int get_path_fi(int n, __u32 *path, struct inode *ino)
{       
        __u32 a = _bc;
        __u32 b = a, i;
        if (n < MAX_DIRECT_LEN) {
                path[0] = n;
                return 1;
        }
        n -= MAX_DIRECT_LEN;
        for (i = 0; n >= b; ++i) {
                n -= b;
                b *= a;
        }
        path[0] = MAX_DIRECT_LEN + i;
        // get offset
        b /= a;  // b == step size
        for (i = 1; b != 0; ++i) {
                path[i] = n / b;
                n -= b * path[i];
                b /= a;
        }
        return i;
}

static int ino_add(__u32 id, int depth, int max_depth,
                __u32 *leftover, struct inode *ino)
{
        int i, end = _bc, re = 1;
        if (depth > max_depth || *leftover == 0)
                return 1;
        __u32 *block = (__u32 *)malloc(_block_size);
        if (!block)
                return 0;
        for (i = 0; i < end; ++i) {
                if (*leftover == 0)
                        break;
                allocate_block(block + i);
                if (depth == max_depth)
                        *leftover -= 1;
                if (!ino_add(block[i], depth+1, max_depth, leftover, ino)) {
                        free(block);
                        return 0;
                }
        }
        if (!bbuf_write(block, id))
                re = 0;
        free(block);
        return re;
}

static int ino_sub(__u32 id, int depth, int max_depth,
                __u32 *leftover, int *is_done, struct inode *ino)
{
        int i;
        if (depth > max_depth || *leftover == 0) {
                *is_done = 1;
                return 1;
        }
        __u32 *block = (__u32 *)malloc(_block_size);
        if (!block)
                return 0;
        if (!bbuf_read(block, id)) {
                free(block);
                return 0;
        }
        for (i = _bc - 1; i >= 0; --i) {
                if (*leftover == 0)
                        break;
                if (!ino_sub(block[i], depth+1, max_depth, leftover, is_done, ino)) {
                        free(block);
                        return 0;
                }
                if (*leftover != 0 || *is_done != 0) {
                        reclaim_block(block[i]);
                        if (depth == max_depth)
                                *leftover -= 1;
                        if (i == 0) {
                                *is_done = 1;
                        } else {
                                *is_done = 0;
                        }
                }
        }
        free(block);
        return 1;
}

// depth = 0
static int ino_add_block(__u32 *block, __u32 *path, int len, int depth, __u32 *md,
                __u32 *leftover, struct inode *ino)
{
        __u32 i, end, *next_block = NULL;
        if (len - 1 != depth) { // not leaf
                next_block = (__u32 *)malloc(_block_size);
                if (!next_block ||
                                !bbuf_read(next_block, block[path[depth]]) ||
                                !ino_add_block(next_block, path, len, depth+1, md, leftover, ino)) {
                        goto failed;
                }
        }
        end = depth == 0 ? INDEX_SIZE : _bc;
        for (i = path[depth]+1; i < end; ++i) {
                if (*leftover == 0)
                        break;
                allocate_block(block + i);
                if (depth == 0) { //solve max depth
                        *md = _max_depth(i);
                }
                if (depth == *md) { // leaf
                        *leftover -= 1;
                        ino->block_count += 1;
                }
                if (!ino_add(block[i], depth+1, *md, leftover, ino))
                        goto failed;
        }
        if (next_block) {
                if (!bbuf_write(next_block, block[path[depth]]))
                        goto failed;
                free(next_block);
        }
        return 1;
failed:
        if (next_block)
                free(next_block);
        return 0;
}

static void free_blocks_on_path(__u32 **blocks, int len) {
        if (!blocks)
                return;
        int i;
        for (i = 1; i < len; ++i) {
                if (blocks[i])
                        free(blocks[i]);
        }
        free(blocks);
}

static __u32 **ino_read_blocks_on_path(__u32 *path, int len, struct inode *ino)
{
        __u32 i, id;
        __u32 **blocks = NULL;
        blocks = (__u32 **)malloc(sizeof(__u32 *) * len);
        if (!blocks)
                return NULL;
        blocks[0] = ino->index;
        for (i = 1; i < len; ++i) {
                id = blocks[i-1][path[i-1]];
                blocks[i] = (__u32 *)malloc(_block_size);
                if (!blocks[i] || !bbuf_read(blocks[i], id)) {
                        free_blocks_on_path(blocks, len);
                        return NULL;
                }
        }
        return blocks;
}

//static int ino_sub(__u32 id, int depth, int max_depth,
//                __u32 *leftover, int *is_done, struct inode *ino)
static int ino_sub_block(__u32 *path, int len, __u32 *leftover, struct inode *ino)
{
        int i, j, md, is_done = 1;
        __u32 **blocks = ino_read_blocks_on_path(path, len, ino);
        if (!blocks)
                return 0;
        md = _max_depth(path[0]);
        for (i = len - 1; i >= 0; --i) {
                if (*leftover == 0 && is_done == 0)
                        break;
                for (j = path[i]; j >= 0; --j) {
                        if (*leftover == 0 && is_done == 0)
                                break;
                        if (i == 0)
                                md = _max_depth(j);
                        if (j != path[i]) {
                                if (!ino_sub(blocks[i][j], i + 1, md, leftover, &is_done, ino)) {
                                        free_blocks_on_path(blocks, len);
                                        return 0;
                                }
                        }
                        if (is_done != 0) {
                                reclaim_block(blocks[i][j]);
                                if (i == md)  //leaf
                                        *leftover -= 1;
                                if (j == 0)
                                        is_done = 1;
                                else
                                        is_done = 0;
                        }
                }
        }
        free_blocks_on_path(blocks, len);
        return 1;
}

// n > 0
static int ino_addb(__u32 n, struct inode *ino)
{
        if (n > spare_block() || ino->block_count + n > _max_inode_block_count)
                return 0;
        if (ino->block_count == 0) {  // add first block
                allocate_block(ino->index);
                n--;
                ino->block_count += 1;
        }
        __u32 path[4];
        int len = get_path_fi(ino->block_count - 1, path, ino);
        __u32 md = _max_depth(path[0]); // init max depth
        return ino_add_block(ino->index, path, len, 0, &md, &n, ino);
}

//static int ino_sub_block(__u32 *path, int len, __u32 *leftover, struct inode *ino)
// n > 0
static int ino_subb(__u32 n, struct inode *ino)
{
        if (n + spare_block() > _max_block_num || n > ino->block_count)
                return 0;
        __u32 path[4];
        int len = get_path_fi(ino->block_count - 1, path, ino);
        __u32 md = _max_depth(path[0]);
        return ino_sub_block(path, len, &n, ino);
}

int ino_cs(__u32 size, struct inode *ino)
{
        if (size > _max_inode_size)
                return 0;
        __u32 newb = size == 0 ? 0 : (size - 1) / _block_size + 1;
        __u32 b = ino->block_count;
        if (newb > b) {
                if (!ino_addb(newb - b, ino))
                        return 0;
        } else if (b > newb) {
                if (!ino_subb(b - newb, ino))
                        return 0;
        } else {

        }
        ino->size = size;
        ino->block_count = newb;
        return 1;
}

__u32 ino_alloc(__u32 parent, __u32 type)
{
        __u32 id;
        struct inode *ino = NULL;
        if (allocate_inode(&id) == 0)
                return 0;
        ino = (struct inode *)malloc(sizeof(struct inode));
        if (!(ino)) {
                reclaim_inode(id);
                return 0;
        }
        memset(ino, 0, sizeof(struct inode));
        ino->parent = parent;
        ino->type = type;
        ino->ctime = time(NULL);
        ibuf_write(ino, id);
        free(ino);
        return id;
}

int ino_recla(struct inode *ino, __u32 id)
{
        if (!ino) {
                ino = (struct inode *)malloc(sizeof(struct inode));
                if (!ino)
                        return 0;
                if (!ibuf_read(ino, id)) {
                        free(ino);
                        return 0;
                }
                ino_cs(0, ino);
                reclaim_inode(id);
                ino_free(ino);
        }
        ino_cs(0, ino);
        reclaim_inode(id);
        return 1;
}

int ino_free(struct inode *ino)
{
        if (ino)
                free(ino);
}

int ino_read(struct inode *ino, __u32 id)
{
        return ibuf_read(ino, id);
}

int ino_write(struct inode *ino, __u32 id)
{
        return ibuf_write(ino, id);
}

//int ino_set_type(int type, struct inode *ino)
//{
//        if (type < 0 || type > 1)
//                return 0;
//        ino->type = type;
//        return 1;
//}

int ino_type(struct inode *ino)
{
        return ino->type;
}

//int ino_set_time(time_t t, struct inode *ino)
//{
//        ino->ctime = t;
//}

time_t ino_time(struct inode *ino)
{
        return ino->ctime;
}

__u32 ino_max_data_size()
{
        return _max_inode_size;
}
