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

class kprintSystem {
public:
	kprintSystem() {}
	~kprintSystem() {}

	static void init();
	static size_t print(const char *fmt, va_list params);

private:
	static void print_str(const char *str, size_t len);
};
}// namespace MdOS::IO

#endif