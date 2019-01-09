#include "disk.h"
#include "string.h"

/* for test */
static uint8_t disk[102400];

int read_disk(void* ptr, uint32_t disk_addr, uint32_t size) {
	memcpy(ptr, (void*)(disk+disk_addr) , size);
        return size;
}
int write_disk(void* ptr, uint32_t disk_addr, uint32_t size) {
	memcpy((void*)(disk+disk_addr), ptr, size);
        return size;
}
