#include "super_block.h"
#include "balloc.h"
#include "bbuf.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

static __u32 *_bids = NULL;
static __32  _sp = -1;
static __u32 _first_block;
static __u32 _max_block_num;
static __u32 _block_size;
static __u32 _max_stack_size;

int build_bindex()
{
	__u32 i;
        _sp = -1;
	for (i = 0; i < _max_block_num; ++i) {
                if (!reclaim_block(i))
                        return false;
        }
        return true;
}

void balloc_init(struct super_block *sb)
{
        int i;
	_max_block_num = sb->max_block_num;
	_block_size = sb->block_size;
        _max_stack_size = _block_size / sizeof(__u32);
        _sp = sb->block_sp;
        _bids = (__u32 *)malloc(_max_stack_size * _block_size);
        for (i = 0; i < _max_stack_size; ++i) {
                _bids[i] = sb->bids[i];
        }
}

void balloc_uninit(struct super_block *sb)
{
        int i;
	sb->max_block_num = _max_block_num;
	sb->block_size = _block_size;
        sb->block_sp = _sp;
        for (i = 0; i < _max_stack_size; ++i) {
                sb->bids[i] = _bids[i];
        }
        if (_bids)
                free(_bids);
	_bids = NULL;
	_sp = -1;
}

int allocate_block(__u32* bid)
{
	int error;
	__u32 bn;
	if (_sp == -1)
		return false;
	bn = _bids[_sp];
	/* if stack will be empty after allocation and the top of stack 
	 * is not the first block, it should be replaced by a stack from disk 
	 * according to the last block id */
	if (_sp == 0 && _bids[_sp] != _first_block) {
		error = !bbuf_read(_bids, bn);
		if (error) {
			printf("failed to read disk while allocating\n");
			return false;
		}
		_sp = _max_stack_size - 1;
	} else {
		_sp = _sp - 1;
	}
	*bid = bn;
	return true;
}

int reclaim_block(__u32 bid)
{
	int error;
	if (bid >= _max_block_num) {
		return false;
	}
	if (_sp == -1) { /* mark the first block */
		_first_block = bid;
	}
	if (_sp == _max_stack_size - 1) { /* stack is full, should move the stack to disk */
		error = !bbuf_write(_bids, bid);
		if (error) {
			printf("failed to write a block while reclaiming block\n");
			return false;
		}
		_sp = 0;
	} else {
		_sp = _sp + 1;
	}
	_bids[_sp] = bid;
	return true;
}
