#ifndef MDOS_CHAR_SINK_H
#define MDOS_CHAR_SINK_H

#include <k_utils/result.hpp>
#include <stddef.h>

namespace MdOS::CharSink {
constexpr size_t max_char_sinks = 4;
using CharSink = void (*)(char);

inline CharSink sinks[max_char_sinks];
MdOS::Result register_char_sink(CharSink sink);
MdOS::Result deregister_char_sink(CharSink sink);
void putc(char c);
}// namespace MdOS::CharSink

#endif