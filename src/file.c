#include "file.h"


int f_read(void *data, __u32 addr, __u32 size, struct file *f)
{

}

int f_write(void *data, __u32 addr, __u32 size, struct file *f)
{}

struct file *f_open(__u32 iid);
void f_close(struct file *f);

int f_create(__u32 *iid);
// It need to close file before deleting.
int f_delete(__u32 iid);
