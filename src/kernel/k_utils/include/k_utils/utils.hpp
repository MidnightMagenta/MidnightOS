#ifndef MDOS_K_UTILS_H
#define MDOS_K_UTILS_H

#include <IO/kprint.h>
#include <stdint.h>

#define ROUND_NTH(val, n) ((val + n - 1) / n) * n
#define ALIGN_ADDR(val, alignment, castType) (castType(val) + (castType(alignment) - 1)) & (~(castType(alignment) - 1))
#define ALIGN_UP(val, alignment, castType) (castType(val) + (castType(alignment) - 1)) & (~(castType(alignment) - 1))
#define ALIGN_DOWN(val, alignment, castType) (castType(val) & ~(castType(alignment) - 1))
inline uint64_t rdtsc() {
	uint32_t lo, hi;
	__asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t) hi << 32) | lo;
}

class ScopedProfiler {
public:
	explicit ScopedProfiler(const char *name) : m_name(name), m_start(rdtsc()) {}

	~ScopedProfiler() {
		uint64_t end = rdtsc();
		kprint("[PROFILE] %s: %lu cycles\n", m_name, end - m_start);
	}

private:
	const char *m_name;
	uint64_t m_start;
};

#define MDOS_INTERNAL_CONCAT(x, y) x##y
#define CONCAT(x, y) MDOS_INTERNAL_CONCAT(x, y)

#ifdef _DEBUG
#define PROFILE_SCOPE(name)                                                                                            \
	ScopedProfiler CONCAT(_profiler_, __LINE__) { name }
#else
#define PROFILE_SCOPE(name) /*void*/
#endif

#endif