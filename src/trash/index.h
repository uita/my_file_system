#ifndef INDEX_H
#define INDEX_H

#include "stdint.h"
#include "fcb.h"

struct index_iterator {
        struct fcb *f;
        void* buffer;
        uint32_t pos;
        uint32_t lid;
};

struct index_iterator *create(uint32_t pos, struct fcb *f);
void destroy(struct index_iterator *ii);
bool set_pos(uint32_t pos, struct index_iterator *ii);
uint32_t get_data(struct index_iterator *ii);
bool add_leaves(uint32_t n, struct fcb *f);
bool remove_leaves(uint32_t n, struct fcb *f);

#endif
