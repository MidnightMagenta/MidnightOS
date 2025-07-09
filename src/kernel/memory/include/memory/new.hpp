#ifndef MDOS_NEW_H
#define MDOS_NEW_H

#include <stddef.h>

inline void *operator new(size_t size __attribute__((unused)), void *ptr) noexcept { return ptr; }
inline void operator delete(void *, void *) noexcept {}

#endif