#ifndef MDOS_CHAR_SINK_H
#define MDOS_CHAR_SINK_H

#include <k_utils/result.h>
#include <stddef.h>

namespace MdOS::CharSink {
constexpr size_t max_char_sinks = 4;
using CharSink = void (*)(char);

inline CharSink sinks[max_char_sinks];
Result register_char_sink(CharSink sink);
Result deregister_char_sink(CharSink sink);
void putc(char c);
}// namespace MdOS::CharSink

#endif