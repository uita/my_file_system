#include "super_block.h"
#include "ialloc.h"
#include "string.h"
#include "stdlib.h"

#define row_t __u32

#define _bitmap_col (sizeof(row_t)*8)
#define _bitmap_row (_max_inode_num/_bitmap_col+1)
#define _last_row_len (_max_inode_num%_bitmap_col)
#define _total_ids_bytes (sizeof(row_t)*_bitmap_row)

static row_t *_iids;
static __u32 _max_inode_num;
static __u32 _spare_inode;


void ialloc_init(struct super_block *sb)
{
	int i;
	_max_inode_num = sb->max_inode_num;
        _spare_inode = sb->spare_inode;
	_iids = (row_t*)malloc(sizeof(row_t)*_bitmap_row);
	for (i = 0; i < _bitmap_row; ++i)
		_iids[i] = sb->iids[i];
}

void ialloc_uninit(struct super_block *sb)
{
        int i;
        for (i = 0; i < _bitmap_row; ++i)
                sb->iids[i] = _iids[i];
	free(_iids);
	_iids = NULL;
	_max_inode_num = 0;
        sb->spare_inode = _spare_inode;
}

int allocate_inode(__u32* iid)
{
	int i, j;
	__u32 n, col;
	row_t full;
	for (i = 0; i < _bitmap_row; ++i) {
		if (i != _bitmap_row-1) {
			col = _bitmap_col;
			full = 0xffffffff;
		} else { 			// last column
			col = _last_row_len;
			full = ((row_t)1<<_last_row_len) - 1;
		}
		n = _iids[i];
		for (j = 0; j < col; ++j) {
			if ((n & ((row_t)1<<j)) == 0) {
				_iids[i] |= ((row_t)1<<j);
				*iid = _bitmap_col * i + j;
				return true;
			}
		}
	}
        _spare_inode -= 1;
	return false;
}

int reclaim_inode(__u32 iid)
{
	if (iid >= _max_inode_num)
		return false;
	__u32 row = iid / _bitmap_col;
	__u32 col = iid % _bitmap_col;
	if ((_iids[row] & ((row_t)1<<col)) == 0)
		return false;
	_iids[row] &= 0xffffffff - ((row_t)1<<col);
        _spare_inode += 1;
	return true;
}

__u32 spare_inode()
{
        return _spare_inode;
}
