#ifndef DISK_H
#define DISK_H

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

#define BLOCK_SIZE 32
#define MAX_FILE_NUM 1000

struct block {
	uint8_t data[BLOCK_SIZE];
};

struct block blocks[20];

bool read_block(struct block *bk, int block_num);
bool write_block(struct block *bk, int block_num);

#endif
