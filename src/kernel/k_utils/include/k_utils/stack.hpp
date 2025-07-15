#ifndef MDOS_STACK_H
#define MDOS_STACK_H

#include <libk/stdlib.h>
#include <libk/string.h>
#include <stddef.h>

namespace utils {
template<typename t_stack>
class Stack {
public:
	explicit Stack(size_t entryCount) {
		m_stack = (t_stack *) malloc(entryCount * sizeof(t_stack));
		if (m_stack == nullptr) { return; }
		m_maxEntryCount = entryCount;
	}

	Stack(const Stack &other) {
		m_stack = (t_stack *) malloc(other.max_size() * sizeof(t_stack));
		memcpy((void *) m_stack, (void *) other.data(), other.max_size() * sizeof(t_stack));
		m_maxEntryCount = other.max_size();
		m_entryCount = other.size();
	}

	~Stack() {
		if (m_stack != nullptr) {
			free((void *) m_stack);
			m_stack = nullptr;
		}
	}

	void push(t_stack val) {
		if (m_stack == nullptr) { return; }
		if (m_entryCount >= m_maxEntryCount) { resize(m_maxEntryCount + 16); }
		m_stack[m_entryCount++] = val;
	}
	t_stack pop() {
		if (m_stack == nullptr) { return t_stack(); }
		if (m_entryCount == 0) { return t_stack(); }
		return m_stack[--m_entryCount];
	}

	size_t size() const { return m_entryCount; }
	size_t max_size() const { return m_maxEntryCount; }
	t_stack *data() { return m_stack; }

	void shrink_to_fit() {
		if (m_stack == nullptr) { return; }
		t_stack *tmpBuffer = (t_stack *) malloc(m_entryCount * sizeof(t_stack));
		if (tmpBuffer == nullptr) {
			return;//TODO: error
		}
		memcpy((void *) tmpBuffer, (void *) m_stack, m_entryCount * sizeof(t_stack));
		free((void *) m_stack);
		m_stack = tmpBuffer;
		m_maxEntryCount = m_entryCount;
	}

	void resize(size_t newMaxEntryCount) {
		if (m_stack == nullptr) { return; }
		t_stack *tmpBuffer = (t_stack *) malloc(newMaxEntryCount * sizeof(t_stack));
		if (tmpBuffer == nullptr) {
			return;//TODO: error
		}
		memcpy((void *) tmpBuffer, (void *) m_stack, m_entryCount * sizeof(t_stack));
		free((void *) m_stack);
		m_stack = tmpBuffer;
		m_maxEntryCount = newMaxEntryCount;
	}

private:
	t_stack *m_stack = nullptr;
	size_t m_maxEntryCount;
	size_t m_entryCount = 0;
};
}// namespace utils

#endif