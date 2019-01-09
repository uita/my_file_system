#include "disk.h"
#include "super_block.h"

static __u32 _disk_capacity;
static __u32 _super_block_addr;
static __u32 _inode_zone_addr;
static __u32 _block_zone_addr;
static __u32 _inode_size;
static __u32 _block_size;
static __u32 _max_inode_num;
static __u32 _max_block_num;

void rw_init(struct super_block *sb)
{
        _disk_capacity = sb->disk_capacity;
        _super_block_addr = sb->super_block_addr;
        _inode_zone_addr = sb->inode_zone_addr;
        _block_zone_addr = sb->block_zone_addr;
        _inode_size = sb->inode_size;
        _block_size = sb->block_size;
        _max_inode_num = sb->max_inode_num;
        _max_block_num = sb->max_block_num;
}

void rw_uninit(struct super_block *sb)
{
        sb->disk_capacity = _disk_capacity;
        sb->super_block_addr = _super_block_addr;
        sb->inode_zone_addr = _inode_zone_addr;
        sb->block_zone_addr = _block_zone_addr;
        sb->inode_size = _inode_size;
        sb->block_size = _block_size;
        sb->max_inode_num = _max_inode_num;
        sb->max_block_num = _max_block_num;
}

int read_super_block(struct super_block *sb)
{
        if (!sb)
                return false;
        return read_disk((void*)sb, sb->super_block_addr, sizeof(struct super_block));
}

int write_super_block(struct super_block *sb)
{
        if (!sb)
                return false;
        return write_disk((void*)sb, sb->super_block_addr, sizeof(struct super_block));
}

int read_inode(void *inode, __u32 iid)
{
        if (iid >= _max_inode_num)
                return false;
        return read_disk(inode, _inode_zone_addr + _inode_size * iid, _inode_size);
}

int write_inode(void *inode, __u32 iid)
{
        if (iid >= _max_inode_num)
                return false;
        return write_disk((void*)inode, _inode_zone_addr + _inode_size * iid, _inode_size);
}

int read_block(void *block, __u32 bid) 
{
        if (bid >= _max_block_num)
                return false;
        return read_disk(block, _block_zone_addr + _block_size * bid, _block_size);
}

int write_block(void *block, __u32 bid) 
{
        if (bid >= _max_block_num)
                return false;
        return write_disk(block, _block_zone_addr + _block_size * bid, _block_size);
}

