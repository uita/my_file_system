#include "super_block.h"
#include "fsb_alloc.h"
#include "rw.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

static uint32_t *_fsb_ids = NULL;
static int32_t _sp = -1;
static uint32_t _first_fsb;
static uint32_t _max_fsb_num;
static uint32_t _fsb_size;
static uint32_t _max_stack_size;

/* needed init rw */
bool fsb_build_index()
{
	uint32_t i;
        _sp = -1;
	for (i = 0; i < _max_fsb_num; ++i) {
                if (!reclaim_fsb(i))
                        return false;
        }
        return true;
}

void fsb_alloc_init(struct super_block *sb)
{
        int i;
	_max_fsb_num = sb->max_fsb_num;
	_fsb_size = sb->fsb_size;
        _max_stack_size = _fsb_size / sizeof(uint32_t);
	_fsb_ids = (uint32_t*)malloc(sizeof(uint32_t)*_max_stack_size);
        _sp = sb->fsb_sp;
        for (i = 0; i < _max_stack_size; ++i) {
                _fsb_ids[i] = sb->fsb_ids[i];
        }
}

void fsb_alloc_uninit(struct super_block *sb)
{
        int i;
	sb->max_fsb_num = _max_fsb_num;
	sb->fsb_size = _fsb_size;
        sb->fsb_sp = _sp;
        for (i = 0; i < _max_stack_size; ++i) {
                sb->fsb_ids[i] = _fsb_ids[i];
        }
	if (_fsb_ids != NULL)
		free(_fsb_ids);
	_fsb_ids = NULL;
	_sp = -1;
}

bool allocate_fsb(uint32_t* fsb_id)
{
	bool error;
	uint32_t bn;
	if (_sp == -1)
		return false;
	bn = _fsb_ids[_sp];
	/* if stack will be empty after allocation and the top of stack 
	 * is not the first fsb, it should move a stack from disk 
	 * according to the last fsb id */
	if (_sp == 0 && _fsb_ids[_sp] != _first_fsb) {
		error = !read_fsb((uint8_t*)_fsb_ids, bn);
		if (error) {
			printf("failed to read disk while allocating\n");
			return false;
		}
		_sp = _max_stack_size - 1;
	} else {
		_sp = _sp - 1;
	}
	*fsb_id = bn;
	return true;
}

bool reclaim_fsb(uint32_t fsb_id)
{
	bool error;
	if (fsb_id >= _max_fsb_num) {
		return false;
	}
	if (_sp == -1) { /* mark the first fsb */
		_first_fsb = fsb_id;
	}
	if (_sp == _max_stack_size - 1) { /* stack is full, should move the stack to disk */
		error = !write_fsb((uint8_t*)_fsb_ids, fsb_id);
		if (error) {
			printf("failed to write a fsb while reclaiming fsb\n");
			return false;
		}
		_sp = 0;
	} else {
		_sp = _sp + 1;
	}
	_fsb_ids[_sp] = fsb_id;
	return true;
}
