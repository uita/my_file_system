#include "stdio.h"
#include "stdint.h"
#include "allocator.h"
#include "stdbool.h"

int main() {
	uint32_t n;
	int i;
	bool error;
	uint32_t tmp;
	n = 0;
	empty_allocator();
	for (i = 0; i < 1; ++i, ++n) {
		error = !reclaim_block(n);
		if (error) printf("reclaim_block return false\n");
	}
	for (i = 0; i < 5; ++i) {
		error = !allocate_block(&tmp);
		if (error) {
			printf("allocate_block return false\n");
		} else {
			printf("allocate block %u\n", tmp);
		}
	}
	n = 0;
	for (i = 0; i < 2; ++i, ++n) {
		error = !reclaim_block(n);
		if (error) printf("reclaim_block return false\n");
	}
	for (i = 0; i < 4; ++i) {
		error = !allocate_block(&tmp);
		if (error) {
			printf("allocate_block return false\n");
		} else {
			printf("allocate block %u\n", tmp);
		}
	}
	n = 0;
	for (i = 0; i < 20; ++i, ++n) {
		error = !reclaim_block(n);
		if (error) printf("reclaim_block return false\n");
	}
	for (i = 0; i < 23; ++i) {
		error = !allocate_block(&tmp);
		if (error) {
			printf("allocate_block return false\n");
		} else {
			printf("allocate block %u\n", tmp);
		}
	}
}
