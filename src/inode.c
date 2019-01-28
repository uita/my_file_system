#include "inode.h"
#include "super_block.h"
#include "balloc.h"
#include "ialloc.h"
#include "bbuf.h"
#include "ibuf.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"




#include "rw.h"

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

int ino_init(struct super_block *sb)
{
        _block_size = sb->block_size;
        _max_inode_num = sb->max_inode_num;
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

int ino_read(void *block, __u32 n, struct inode *ino)
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

int ino_write(void *block, __u32 n, struct inode *ino)
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

int ino_alloc(__u32 *id)
{
        return allocate_inode(id);
}

int ino_free(__u32 id)
{
        return reclaim_inode(id);
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
        //if (!write_block(block, id))
        //        re =0;
        if (!bbuf_write(block, id))
                re = 0;
        free(block);
        return re;
}

// depth = 0
static int ino_add_block(__u32 *block, __u32 *path, int len, int depth,
                __u32 *leftover, struct inode *ino)
{
        __u32 i, end, *next_block = NULL;
        if (len - 1 != depth) { // not leaf
                next_block = (__u32 *)malloc(_block_size);
                if (!next_block ||
                                !bbuf_read(next_block, block[path[depth]]) ||
                                !ino_add_block(next_block, path, len, depth+1, leftover, ino)) {
                        goto failed;
                }
        }
        end = depth == 0 ? INDEX_SIZE : _bc;
        for (i = path[depth]+1; i < end; ++i) {
                if (*leftover == 0)
                        break;
                allocate_block(block + i);
                if (i < MAX_DIRECT_LEN) { // leaf
                        *leftover -= 1;
                        ino->block_count += 1;
                }
                if (!ino_add(block[i], depth+1, _max_depth(i), leftover, ino))
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

static int ino_addb(int n, struct inode *ino)
{
        if (ino->block_count == 0) {  // add first block
                allocate_block(ino->index);
                n--;
                ino->block_count += 1;
        }
        __u32 path[4];
        int len = get_path_fi(ino->block_count - 1, path, ino);
        return ino_add_block(ino->index, path, len, 0, &n, ino);
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
//                if (!ino_sub_block(b - newb, ino))
//                        return 0;
        } else {
                return 1;
        }
        ino->size = size;
        ino->block_count = newb;
        return 1;
}

