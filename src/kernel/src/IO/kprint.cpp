#include "../../include/IO/kprint.hpp"

MdOS::GOP_Renderer *MdOS::IO::kprintSystem::m_renderer = nullptr;
MdOS::Teletype MdOS::IO::kprintSystem::m_tty = MdOS::Teletype();

void MdOS::IO::kprintSystem::Initialize(MdOS::GOP_Renderer *renderer, PSF1_Font *font) {
	m_renderer = renderer;
	m_tty.Initialize(renderer, font);
}

int MdOS::IO::kprintSystem::print(const char *fmt, va_list params) {
	int written = 0;
	while (*fmt != '\0') {
		size_t maxrem = INT_MAX - written;

		if (fmt[0] != '%' || fmt[1] == '%') {
			if (fmt[0] == '%') { fmt++; }
			size_t ammount = 1;
			while (fmt[ammount] && fmt[ammount] != '%') { ammount++; }
			if (maxrem < ammount) {
				//TODO: error stuff
				return -1;
			}
			m_tty.PrintString(fmt, ammount);
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
				return -1;
			}
			m_tty.PrintString(&c, 1);
			written++;
		} else if (*fmt == 's') {
			fmt++;
			const char *str = va_arg(params, const char *);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else if (*fmt == 'd' || *fmt == 'i') {
			fmt++;
			int32_t num = (int32_t) va_arg(params, int32_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'd') || (*fmt == 'l' && fmt[1] == 'i')) {
			fmt += 2;
			int64_t num = (int64_t) va_arg(params, int64_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else if (*fmt == 'u') {
			fmt++;
			uint32_t num = (uint32_t) va_arg(params, uint32_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'u')) {
			fmt += 2;
			uint64_t num = (uint64_t) va_arg(params, uint64_t);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else if (*fmt == 'x') {
			fmt++;
			uint32_t num = (uint32_t) va_arg(params, uint32_t);
			const char *str = MdOS::string::to_hstring(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'x')) {
			fmt += 2;
			uint64_t num = (uint64_t) va_arg(params, uint64_t);
			const char *str = MdOS::string::to_hstring(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else if (*fmt == 'f') {
			fmt++;
			double num = (double) va_arg(params, double);
			const char *str = MdOS::string::to_string(num);
			size_t ammount = MdOS::string::strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return -1;
			}
			m_tty.PrintString(str, ammount);
			written += ammount;
		} else {
			fmt = fmt_begun_at;
			size_t len = MdOS::string::strlen(fmt);
			if (maxrem < len) {
				//implement errno
				return -1;
			}
			m_tty.PrintString(fmt, len);
			written += len;
			fmt += len;
		}
	}

	return written;
}

int MdOS::IO::kprint(const char *fmt, ...) {
	va_list params;
	va_start(params, fmt);

	int written = MdOS::IO::kprintSystem::print(fmt, params);

	va_end(params);
	return written;
}