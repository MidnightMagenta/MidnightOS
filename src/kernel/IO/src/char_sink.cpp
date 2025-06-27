#include <IO/char_sink.hpp>
#include <stddef.h>

MdOS::Result MdOS::CharSink::register_char_sink(CharSink sink) {
	for (size_t i = 0; i < MAX_CHAR_SINKS; i++) {
		if (sinks[i] == nullptr) {
			sinks[i] = sink;
			return MdOS::Result::SUCCESS;
		}
	}
	return MdOS::Result::OUT_OF_SPACE;
}

MdOS::Result MdOS::CharSink::deregister_char_sink(CharSink sink) {
	for (size_t i = 0; i < MAX_CHAR_SINKS; i++) {
		if (sinks[i] == sink) {
			sinks[i] = nullptr;
			return MdOS::Result::SUCCESS;
		}
	}
	return MdOS::Result::INVALID_PARAMETER;
}

void MdOS::CharSink::putc(char c) {
	for (size_t i = 0; i < MAX_CHAR_SINKS; i++) {
		if (sinks[i] != nullptr) { sinks[i](c); }
	}
}