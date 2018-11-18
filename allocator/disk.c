#include "disk.h"
bool read_block(struct block *bk, int block_num) {
	if (block_num < 20 && block_num > -1)
		memcpy(bk, blocks+block_num, BLOCK_SIZE);
	return true;
}
bool write_block(struct block *bk, int block_num) {
	if (block_num < 20 && block_num > -1)
		memcpy(blocks+block_num, bk, BLOCK_SIZE);
	return true;
}
