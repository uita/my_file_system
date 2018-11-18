#include "list.h"
#include "stdio.h"
struct file {
	int gg;
	struct list_node ld;
};
struct file1{
	double hh;
	struct list_node lld;
};
int main() {
	create_list(stack);
	struct file array[10];
	int i;
	for (i = 0; i < 10; ++i)
		array[i].gg = i;
	for (i = 0; i < 10; ++i)
		add_to_list_head(array+i, stack, ld);
	struct file* tmp;
	for_each(tmp, stack, struct file, ld) {
		printf("%d ", tmp->gg);
	}

	create_list(stack1);
	struct file1 array1[10];
	for (i = 0; i < 10; ++i)
		array1[i].hh = i;
	for (i = 0; i < 10; ++i)
		add_to_list_head(array1+i, stack1, lld);
	struct file1* tmp1;
	for_each(tmp1, stack1, struct file1, lld) {
		printf("%f ", tmp1->hh);
	}
}
