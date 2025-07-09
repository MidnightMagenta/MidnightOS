#ifndef MDOS_K_UTILS_NUMERIC_LIMITS_H
#define MDOS_K_UTILS_NUMERIC_LIMITS_H

#include <stdint.h>

namespace utils {
template<typename T>
struct NumericLimits { /*void*/
};

template<>
struct NumericLimits<uint8_t> {
	static constexpr uint8_t max() noexcept { return UINT8_MAX; }
	static constexpr uint8_t min() noexcept { return 0; }
};
template<>
struct NumericLimits<uint16_t> {
	static constexpr uint16_t max() noexcept { return UINT16_MAX; }
	static constexpr uint16_t min() noexcept { return 0; }
};
template<>
struct NumericLimits<uint32_t> {
	static constexpr uint32_t max() noexcept { return UINT32_MAX; }
	static constexpr uint32_t min() noexcept { return 0; }
};
template<>
struct NumericLimits<uint64_t> {
	static constexpr uint64_t max() noexcept { return UINT64_MAX; }
	static constexpr uint64_t min() noexcept { return 0; }
};

template<>
struct NumericLimits<int8_t> {
	static constexpr int8_t max() noexcept { return INT8_MAX; }
	static constexpr int8_t min() noexcept { return INT8_MIN; }
};
template<>
struct NumericLimits<int16_t> {
	static constexpr int16_t max() noexcept { return INT16_MAX; }
	static constexpr int16_t min() noexcept { return INT16_MIN; }
};
template<>
struct NumericLimits<int32_t> {
	static constexpr int32_t max() noexcept { return INT32_MAX; }
	static constexpr int32_t min() noexcept { return INT32_MIN; }
};
template<>
struct NumericLimits<int64_t> {
	static constexpr int64_t max() noexcept { return INT64_MAX; }
	static constexpr int64_t min() noexcept { return INT64_MIN; }
};
}// namespace utils
#endif