#ifndef BITMAP_H
#define BITMAP_H

#include "../include/memory/bump_allocator.hpp"
#include "../k_utils/include/utils.hpp"
#include <stddef.h>
#include <stdint.h>

namespace utils {
template<size_t bmp_size>
class FixedBitmap {
public:
	FixedBitmap();
	~FixedBitmap();

private:
	uint8_t bitmap[bmp_size];
	size_t m_size = bmp_size;
};

//caller is responsible for staying within bounds
template<typename t_bmp>
class Bitmap {
public:
	Bitmap() {}
	~Bitmap() {}

	bool init(size_t size) {
		//TODO: if the main allocator is available use the main allocator
		m_size = ROUND_NTH(size, m_bmpEntrySize);
		m_bitmap = (t_bmp *) MdOS::Memory::g_bumpAlloc.alloc(m_size / m_bmpEntrySize);
		if (!m_bitmap) {
			m_size = 0;
			return false;
		}
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = 0; }
		return true;
	}
	size_t size() const { return m_size; }
	size_t entry_size() const { return m_bmpEntrySize; }

	bool operator[](size_t index) const {
		if (index >= m_size) { return false; }
		return (m_bitmap[index / m_bmpEntrySize] >> (index % m_bmpEntrySize)) & 1;
	}

	bool set(size_t index) {
		if (index >= m_size) { return false; }
		t_bmp tmp = 1 << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] |= tmp;
		return true;
	}
	bool clear(size_t index) {
		if (index >= m_size) { return false; }
		t_bmp tmp = 1 << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] &= ~tmp;
		return true;
	}
	bool toggle(size_t index) {
		if (index >= m_size) { return false; }
		t_bmp tmp = 1 << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] ^= tmp;
		return true;
	}

	size_t find_first_set() {
		for (size_t i = 0; i < m_size; i++) {
			if ((*this)[i]) { return i; }
		}
		return m_size;
	}
	size_t find_first_clear() {
		for (size_t i = 0; i < m_size; i++) {
			if (!(*this)[i]) { return i; }
		}
		return m_size;
	}

	void set_all() {
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = 1; }
	}

	void clear_all() {
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = 0; }
	}

	void set_range(size_t start, size_t end) {
		if (start >= m_size || end <= start) { return; }
		size_t limit = (end >= m_size) ? m_size : end;
		for (size_t i = start; i < limit; i++) { set(i); }
	}

	void clear_range(size_t start, size_t end) {
		if (start >= m_size || end <= start) { return; }
		size_t limit = (end >= m_size) ? m_size : end;
		for (size_t i = start; i < limit; i++) { clear(i); }
	}

private:
	t_bmp *m_bitmap = nullptr;
	size_t m_size = 0;
	const size_t m_bmpEntrySize = sizeof(t_bmp) * 8;
};

}// namespace utils

#endif