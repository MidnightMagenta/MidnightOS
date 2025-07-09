#ifndef MDOS_K_UTILS_TYPE_TRAITS_H
#define MDOS_K_UTILS_TYPE_TRAITS_H

namespace utils {
namespace TypeTraits {
template<typename T>
struct IsSigned {
	static constexpr bool value = T(-1) < T(0);
};
}// namespace TypeTraits
}// namespace utils

#endif