#ifndef KPRINT_H
#define KPRINT_H

#include "../../include/IO/graphics/GOP_renderer.hpp"
#include "../../include/IO/tty/tty.hpp"
#include "../../include/kstring.hpp"
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

namespace MdOS::IO {
int kprint(const char *fmt, ...);

class kprintSystem {
public:
	kprintSystem() {}
	~kprintSystem() {}

	static void init(MdOS::GOP_Renderer *renderer, PSF1_Font *font);
	static int print(const char *fmt, va_list params);

private:
	static MdOS::GOP_Renderer *m_renderer;
	static MdOS::Teletype m_tty;
};
}// namespace MdOS::IO

#endif