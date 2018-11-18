#ifndef DISK_H
#define DISK_H

#include "stdint.h"

#define BLOCK_SIZE 4096

struct block {
	uint8_t data[BLOCK_SIZE];
};

bool read_block(struct block *bk, int block_num);
bool write_block(struct block *bk, int block_num);

#endif
