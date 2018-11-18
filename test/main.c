#include "stdio.h"
struct A {
	int a;
};
struct B {
	double b;
};
struct C {
	struct A a;
	struct B b;
};
int main() {
	int size = sizeof(struct C);
	printf("%u\n", size);
}
