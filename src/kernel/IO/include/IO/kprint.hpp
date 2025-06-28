#ifndef KPRINT_H
#define KPRINT_H

#include <IO/GOP_renderer.hpp>
#include <IO/tty.hpp>
#include <k_utils/kstring.hpp>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

namespace MdOS::IO {
size_t kprint(const char *fmt, ...);
}// namespace MdOS::IO

#endif