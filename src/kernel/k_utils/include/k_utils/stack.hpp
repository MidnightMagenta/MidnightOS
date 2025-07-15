#ifndef MDOS_STACK_H
#define MDOS_STACK_H

#include <stddef.h>

namespace utils {
template<typename t_stack>
class Stack {
public:
    explicit Stack(size_t maxEntryCount);

private:
    t_stack *m_stack;
    size_t m_maxEntryCount;
    size_t m_entryCount;
};
}// namespace utils

#endif