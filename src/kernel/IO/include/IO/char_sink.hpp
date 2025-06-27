#ifndef MDOS_CHAR_SINK_H
#define MDOS_CHAR_SINK_H

#include <k_utils/result.hpp>

#define MAX_CHAR_SINKS 4

namespace MdOS::CharSink {
using CharSink = void (*)(char);
inline CharSink sinks[MAX_CHAR_SINKS];
MdOS::Result register_char_sink(CharSink sink);
MdOS::Result deregister_char_sink(CharSink sink);
void putc(char c);
}// namespace MdOS::CharSink


#endif