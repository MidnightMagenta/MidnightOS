#include "../include/basics.h"

int mem_compare(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = (unsigned char *) aptr, *b = (unsigned char *) bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i])
			return 1;
	}
	return 0;
}