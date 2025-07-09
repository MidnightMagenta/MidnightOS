#ifndef HWIO_H
#define HWIO_H

#include <stdint.h>

namespace MdOS::IO {
static inline void outb(uint8_t val, uint16_t port) {
	__asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}
static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}
}// namespace MdOS::IO


#endif