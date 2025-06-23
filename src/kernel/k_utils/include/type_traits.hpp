#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

namespace utils {
namespace TypeTraits {
template<typename T>
struct IsSigned {
	static constexpr bool value = T(-1) < T(0);
};
}// namespace TypeTraits
}// namespace utils

#endif