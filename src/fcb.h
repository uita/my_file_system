#ifndef FCB_H
#define FCB_H

/* operations for single file */

#include "stdint.h"

#define MAX_NAME_LEN 256
#define MAX_DIRECT_LEN 11

struct fcb {
        uint32_t id;
        uint8_t name[MAX_NAME_LEN];
        uint32_t size;             // data size
        uint32_t block_size;
        uint32_t block_count;      // include blocks for indexes
	uint32_t tag;
	uint32_t direct[MAX_DIRECT_LEN];
	uint32_t single_indirect;
	uint32_t double_indirect;
        /* for directory */
        uint32_t parent_id;
	uint32_t child_id;
	uint32_t brother_id;
};

bool read(uint8_t* ptr, uint32_t addr, uint32_t size, struct fcb* f);
/* variable=true, means the size of file wouble be change to (addr+size) */
bool write(uint8_t* ptr, uint32_t addr, uint32_t size, struct fcb* f, 
                bool variable);
/* empty the blocks of file but don't delete file */
bool clear(struct fcb* f);
bool resize(uint32_t size);

uint32_t max_data_size(struct fcb *f);
uint32_t total_block_count(struct fcb *f);

#endif
