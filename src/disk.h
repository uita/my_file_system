#ifndef DISK_H
#define DISK_H

#include "stdint.h"
#include "stdbool.h"

/* Byte */
int read_disk(void* ptr, uint32_t disk_addr, uint32_t size);
int write_disk(void* ptr, uint32_t disk_addr, uint32_t size);

#endif
