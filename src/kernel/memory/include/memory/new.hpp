#ifndef MDOS_NEW_H
#define MDOS_NEW_H

#include <stddef.h>

void *operator new(size_t size __attribute__((unused)), void *ptr) noexcept { return ptr; }
void operator delete(void *, void *) noexcept {}

#endif