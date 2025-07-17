#include <libk/string.h>

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
	const unsigned char *ptr1Ptr = (const unsigned char *) ptr1;
	const unsigned char *ptr2Ptr = (const unsigned char *) ptr2;
	while (num--) {
		if (*ptr1Ptr != *ptr2Ptr) { return *ptr1Ptr - *ptr2Ptr; }
		ptr1Ptr++;
		ptr2Ptr++;
	}
	return 0;
}
