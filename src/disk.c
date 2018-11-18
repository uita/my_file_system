#include "disk.h"
#include "string.h"

/* for test */
static uint8_t disk[102400];

bool read_disk(void* ptr, uint32_t disk_addr, uint32_t size) {
	memcpy(ptr, (void*)(disk+disk_addr) , size);
}
bool write_disk(void* ptr, uint32_t disk_addr, uint32_t size) {
	memcpy((void*)(disk+disk_addr), ptr, size);
}
