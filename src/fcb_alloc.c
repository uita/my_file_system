#include "super_block.h"
#include "fcb_alloc.h"
#include "string.h"
#include "stdlib.h"

#define row_t uint32_t

#define _bitmap_col (sizeof(row_t)*8)
#define _bitmap_row (_max_fcb_num/_bitmap_col+1)
#define _last_row_len (_max_fcb_num%_bitmap_col)
#define _total_ids_bytes (sizeof(row_t)*_bitmap_row)

static row_t *_fcb_ids;
static uint32_t _max_fcb_num;


void fcb_alloc_init(struct super_block *sb)
{
	int i;
	_max_fcb_num = sb->max_fcb_num;
	_fcb_ids = (row_t*)malloc(sizeof(row_t)*_bitmap_row);
	for (i = 0; i < _bitmap_row; ++i)
		_fcb_ids[i] = sb->fcb_ids[i];
}

void fcb_alloc_uninit(struct super_block *sb)
{
	free(_fcb_ids);
	_fcb_ids = NULL;
	_max_fcb_num = 0;
}

bool allocate_fcb(uint32_t* fcb_id)
{
	int i, j;
	uint32_t n, col;
	row_t full;
	for (i = 0; i < _bitmap_row; ++i) {
		if (i != _bitmap_row-1) {
			col = _bitmap_col;
			full = 0xffffffff;
		} else { 			// last column
			col = _last_row_len;
			full = ((row_t)1<<_last_row_len) - 1;
		}
		n = _fcb_ids[i];
		for (j = 0; j < col; ++j) {
			if ((n & ((row_t)1<<j)) == 0) {
				_fcb_ids[i] |= ((row_t)1<<j);
				*fcb_id = _bitmap_col * i + j;
				return true;
			}
		}
	}
	return false;
}

bool reclaim_fcb(uint32_t fcb_id)
{
	if (fcb_id >= _max_fcb_num)
		return false;
	uint32_t row = fcb_id / _bitmap_col;
	uint32_t col = fcb_id % _bitmap_col;
	if ((_fcb_ids[row] & ((row_t)1<<col)) == 0)
		return false;
	_fcb_ids[row] &= 0xffffffff - ((row_t)1<<col);
	return true;
}
