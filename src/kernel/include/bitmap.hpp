#ifndef BITMAP_H
#define BITMAP_H

#include "../include/IO/kprint.hpp"
#include "../include/memory/bump_allocator.hpp"
#include "../k_utils/include/limits.hpp"
#include "../k_utils/include/utils.hpp"
#include <stddef.h>
#include <stdint.h>

namespace utils {
template<typename t_bmp, size_t bmp_size>
class FixedBitmap {
public:
	FixedBitmap() {}
	~FixedBitmap() {}

	static_assert(__is_same(t_bmp, uint8_t) || __is_same(t_bmp, uint16_t) || __is_same(t_bmp, uint32_t) ||
				  __is_same(t_bmp, uint64_t));

	size_t size() const { return m_size; }
	size_t entry_size() const { return m_bmpEntrySize; }

	bool operator[](size_t index) const {
		if (index >= m_size) { return false; }
		return (m_bitmap[index / m_bmpEntrySize] >> (index % m_bmpEntrySize)) & 1;
	}

	bool set(size_t index) {
		if (index >= m_size) { return false; }
		t_bmp tmp = t_bmp(1) << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] |= tmp;
		return true;
	}
	bool clear(size_t index) {
		if (index >= m_size) { return false; }
		t_bmp tmp = t_bmp(1) << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] &= ~tmp;
		return true;
	}
	bool toggle(size_t index) {
		if (index >= m_size) { return false; }
		t_bmp tmp = t_bmp(1) << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] ^= tmp;
		return true;
	}

	size_t find_first_set_bit() {
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] > t_bmp(1)) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if ((*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}
	size_t find_first_clear_bit() {
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] < NumericLimits<t_bmp>::max()) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if (!(*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}

	size_t find_next_set_bit(size_t index) {
		for (size_t i = index / m_bmpEntrySize; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] > t_bmp(1)) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if ((*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}
	size_t find_next_clear_bit(size_t index) {
		for (size_t i = index / m_bmpEntrySize; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] < NumericLimits<t_bmp>::max()) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if (!(*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}

	void set_all() {
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = utils::NumericLimits<t_bmp>::max(); }
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
	uint8_t m_bitmap[bmp_size];
	size_t m_size = bmp_size;
	const size_t m_bmpEntrySize = sizeof(t_bmp) * 8;
};

//caller is responsible for staying within bounds
template<typename t_bmp>
class Bitmap {
public:
	Bitmap() {}
	~Bitmap() {}

	static_assert(__is_same(t_bmp, uint8_t) || __is_same(t_bmp, uint16_t) || __is_same(t_bmp, uint32_t) ||
				  __is_same(t_bmp, uint64_t));

	bool init(size_t size) {
		if (m_initialized) { return false; }
		//TODO: if the main allocator is available use the main allocator
		m_memSize = ROUND_NTH(size, m_bmpEntrySize);
		m_bitmap = (t_bmp *) MdOS::Memory::BumpAllocator::aligned_alloc(m_memSize / m_bmpEntrySize, alignof(t_bmp));
		if (m_bitmap == nullptr) {
			m_size = 0;
			m_memSize = 0;
			return false;
		}
		m_size = size;
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = 0; }
		m_initialized = true;
		return true;
	}
	bool init(size_t size, bool initValue) {
		if (m_initialized) { return false; }
		//TODO: if the main allocator is available use the main allocator
		m_memSize = ROUND_NTH(size, m_bmpEntrySize);
		m_bitmap = (t_bmp *) MdOS::Memory::BumpAllocator::aligned_alloc(m_memSize / m_bmpEntrySize, sizeof(t_bmp));
		if (m_bitmap == nullptr) {
			m_size = 0;
			m_memSize = 0;
			return false;
		}
		m_size = size;
		t_bmp initVal = initValue ? utils::NumericLimits<t_bmp>::max() : t_bmp(0);
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = initVal; }
		m_initialized = true;
		kprint("Size: %lu mem size: %lu entry size: %lu\n", m_size, m_memSize, m_bmpEntrySize);
		return true;
	}
	size_t size() const {
		if (!m_initialized) { return 0; }
		return m_size;
	}
	size_t entry_size() const {
		if (!m_initialized) { return 0; }
		return m_bmpEntrySize;
	}

	bool operator[](size_t index) const {
		if (!m_initialized) { return false; }
		if (index >= m_size) { return false; }
		return (m_bitmap[index / m_bmpEntrySize] >> (index % m_bmpEntrySize)) & t_bmp(1);
	}

	bool set(size_t index) {
		if (!m_initialized) { return false; }
		if (index >= m_size) { return false; }
		t_bmp tmp = t_bmp(1) << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] |= tmp;
		return true;
	}
	bool clear(size_t index) {
		if (!m_initialized) { return false; }
		if (index >= m_size) { return false; }
		t_bmp tmp = t_bmp(1) << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] &= ~tmp;
		return true;
	}
	bool toggle(size_t index) {
		if (!m_initialized) { return false; }
		if (index >= m_size) { return false; }
		t_bmp tmp = t_bmp(1) << (index % m_bmpEntrySize);
		m_bitmap[index / m_bmpEntrySize] ^= tmp;
		return true;
	}

	size_t find_first_set_bit() {
		if (!m_initialized) { return 0; }
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] > t_bmp(1)) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if ((*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}
	size_t find_first_clear_bit() {
		if (!m_initialized) { return 0; }
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] < NumericLimits<t_bmp>::max()) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if (!(*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}

	size_t find_next_set_bit(size_t index) {
		if (!m_initialized) { return 0; }
		for (size_t i = index / m_bmpEntrySize; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] > t_bmp(1)) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if ((*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}
	size_t find_next_clear_bit(size_t index) {
		if (!m_initialized) { return 0; }
		for (size_t i = index / m_bmpEntrySize; i < m_size / m_bmpEntrySize; i++) {
			if (m_bitmap[i] < NumericLimits<t_bmp>::max()) {
				for (size_t j = i * m_bmpEntrySize; j < (i * m_bmpEntrySize) + m_bmpEntrySize; j++) {
					if (!(*this)[j]) { return j; }
				}
			}
		}
		return m_size;
	}

	void set_all() {
		if (!m_initialized) { return; }
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = utils::NumericLimits<t_bmp>::max(); }
	}

	void clear_all() {
		if (!m_initialized) { return; }
		for (size_t i = 0; i < m_size / m_bmpEntrySize; i++) { m_bitmap[i] = 0; }
	}

	void set_range(size_t start, size_t end) {
		if (!m_initialized) { return; }
		if (start >= m_size || end <= start) { return; }
		size_t limit = (end >= m_size) ? m_size : end;
		for (size_t i = start; i < limit; i++) { set(i); }
	}

	void clear_range(size_t start, size_t end) {
		if (!m_initialized) { return; }
		if (start >= m_size || end <= start) { return; }
		size_t limit = (end >= m_size) ? m_size : end;
		for (size_t i = start; i < limit; i++) { clear(i); }
	}

private:
	t_bmp *m_bitmap = nullptr;
	size_t m_size = 0;
	size_t m_memSize = 0;
	static constexpr size_t m_bmpEntrySize = sizeof(t_bmp) * 8;
	bool m_initialized = false;
};

}// namespace utils

#endif