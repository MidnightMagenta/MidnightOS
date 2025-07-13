#include <IO/char_sink.hpp>
#include <stddef.h>

Result MdOS::CharSink::register_char_sink(CharSink sink) {
	for (size_t i = 0; i < max_char_sinks; i++) {
		if (sinks[i] == nullptr) {
			sinks[i] = sink;
			return MDOS_SUCCESS;
		}
	}
	return MDOS_OUT_OF_SPACE;
}

Result MdOS::CharSink::deregister_char_sink(CharSink sink) {
	for (size_t i = 0; i < max_char_sinks; i++) {
		if (sinks[i] == sink) {
			sinks[i] = nullptr;
			return MDOS_SUCCESS;
		}
	}
	return MDOS_INVALID_PARAMETER;
}

void MdOS::CharSink::putc(char c) {
	for (size_t i = 0; i < max_char_sinks; i++) {
		if (sinks[i] != nullptr) { sinks[i](c); }
	}
}