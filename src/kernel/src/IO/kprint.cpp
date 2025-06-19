#include "../../include/IO/kprint.hpp"

MdOS::GOP_Renderer *MdOS::IO::kprintSystem::m_renderer = nullptr;
MdOS::Teletype MdOS::IO::kprintSystem::m_tty = MdOS::Teletype();

void MdOS::IO::kprintSystem::init(MdOS::GOP_Renderer *renderer, PSF1_Font *font) {
	m_renderer = renderer;
	m_tty.init(renderer, font);
}

size_t MdOS::IO::kprintSystem::print(const char *fmt, va_list params) {
	size_t written = 0;
	while (*fmt != '\0') {
		size_t maxrem = INT_MAX - written;

		if (fmt[0] != '%' || fmt[1] == '%') {
			if (fmt[0] == '%') { fmt++; }
			size_t ammount = 1;
			while (fmt[ammount] && fmt[ammount] != '%') { ammount++; }
			if (maxrem < ammount) {
				//TODO: error stuff
				return size_t(0);
			}
			m_tty.print_str(fmt, ammount);
			fmt += ammount;
			written += ammount;
			continue;
		}

		const char *fmt_begun_at = fmt++;

		if (*fmt == 'c') {
			fmt++;
			char c = (char) va_arg(params, int);
			if (maxrem < 1) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(&c, 1);
			written++;
		} else if (*fmt == 's') {
			fmt++;
			const char *str = va_arg(params, const char *);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else if (*fmt == 'd' || *fmt == 'i') {
			fmt++;
			int32_t num = (int32_t) va_arg(params, int32_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'd') || (*fmt == 'l' && fmt[1] == 'i')) {
			fmt += 2;
			int64_t num = (int64_t) va_arg(params, int64_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else if (*fmt == 'u') {
			fmt++;
			uint32_t num = (uint32_t) va_arg(params, uint32_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'u')) {
			fmt += 2;
			uint64_t num = (uint64_t) va_arg(params, uint64_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else if (*fmt == 'x') {
			fmt++;
			uint32_t num = (uint32_t) va_arg(params, uint32_t);
			const char *str = MdOS::string::to_hstring(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'x')) {
			fmt += 2;
			uint64_t num = (uint64_t) va_arg(params, uint64_t);
			const char *str = MdOS::string::to_hstring(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else if (*fmt == 'f') {
			fmt++;
			double num = (double) va_arg(params, double);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return size_t(0);
			}
			m_tty.print_str(str, ammount);
			written += ammount;
		} else {
			fmt = fmt_begun_at;
			size_t len = MdOS::string::strlen(fmt);
			if (maxrem < len) {
				//implement errno
				return size_t(0);
			}
			m_tty.print_str(fmt, len);
			written += len;
			fmt += len;
		}
	}

	return written;
}

size_t MdOS::IO::kprint(const char *fmt, ...) {
	va_list params;
	va_start(params, fmt);

	size_t written = MdOS::IO::kprintSystem::print(fmt, params);

	va_end(params);
	return written;
}