#include "disk.h"
#include "string.h"
#include "stdio.h"

/* for test */
//static uint8_t disk[102400];

int read_disk(void* ptr, uint32_t disk_addr, uint32_t size) {
        FILE *fp = fopen("/dev/sdb4", "rb+");
        if (!fp)
                return 0;
        fseek(fp, disk_addr, SEEK_SET);
        int count = fread(ptr, 1, size, fp);
        fclose(fp);
        //printf("read%d\n", count);
        return count;
	//memcpy(ptr, (void*)(disk+disk_addr) , size);
        //return size;
}
int write_disk(void* ptr, uint32_t disk_addr, uint32_t size) {
        FILE *fp = fopen("/dev/sdb4", "rb+");
        if (!fp)
                return 0;
        fseek(fp, disk_addr, SEEK_SET);
        int count = fwrite(ptr, 1, size, fp);
        fclose(fp);
        //printf("write%d\n", count);
        return count;
	//memcpy((void*)(disk+disk_addr), ptr, size);
        //return size;
}
