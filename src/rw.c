#include "super_block.h"
#include "rw.h"
#include "disk.h"
#include "stdint.h"
#include "stdbool.h"

static uint32_t _disk_capacity;
static uint32_t _super_block_addr;
static uint32_t _fcb_zone_addr;
static uint32_t _fsb_zone_addr;
static uint32_t _fcb_size;
static uint32_t _fsb_size;
static uint32_t _max_fcb_num;
static uint32_t _max_fsb_num;

void rw_init(struct super_block *sb)
{
        _disk_capacity = sb->disk_capacity;
        _super_block_addr = sb->super_block_addr;
        _fcb_zone_addr = sb->fcb_zone_addr;
        _fsb_zone_addr = sb->fsb_zone_addr;
        _fcb_size = sb->fcb_size;
        _fsb_size = sb->fsb_size;
        _max_fcb_num = sb->max_fcb_num;
        _max_fsb_num = sb->max_fsb_num;
}

void rw_uninit(struct super_block *sb)
{
        sb->disk_capacity = _disk_capacity;
        sb->super_block_addr = _super_block_addr;
        sb->fcb_zone_addr = _fcb_zone_addr;
        sb->fsb_zone_addr = _fsb_zone_addr;
        sb->fcb_size = _fcb_size;
        sb->fsb_size = _fsb_size;
        sb->max_fcb_num = _max_fcb_num;
        sb->max_fsb_num = _max_fsb_num;
}

bool read_super_block(struct super_block *sb)
{
        if (!sb)
                return false;
        return read_disk((void*)sb, sb->super_block_addr,
                        sizeof(struct super_block));
}

bool write_super_block(struct super_block *sb)
{
        if (!sb)
                return false;
        return write_disk((void*)sb, sb->super_block_addr,
                        sizeof(struct super_block));
}

bool read_fcb(uint8_t *fcb, uint32_t fcb_id)
{
        if (fcb_id >= _max_fcb_num)
                return false;
        return read_disk((void*)fcb,
                        _fcb_zone_addr + _fcb_size * fcb_id, _fcb_size);
}

bool write_fcb(uint8_t *fcb, uint32_t fcb_id)
{
        if (fcb_id >= _max_fcb_num)
                return false;
        return write_disk((void*)fcb,
                        _fcb_zone_addr + _fcb_size * fcb_id, _fcb_size);
}

bool read_fsb(uint8_t *fsb, uint32_t fsb_id) 
{
        if (fsb_id >= _max_fsb_num)
                return false;
        return read_disk((void*)fsb, 
                        _fsb_zone_addr + _fsb_size * fsb_id, _fsb_size);
}

bool write_fsb(uint8_t *fsb, uint32_t fsb_id) 
{
        if (fsb_id >= _max_fsb_num)
                return false;
        return write_disk((void*)fsb,
                        _fsb_zone_addr + _fsb_size * fsb_id, _fsb_size);
}

