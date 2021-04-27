#ifndef MAFIA_UTIL_TYPE_TRAITS
#define MAFIA_UTIL_TYPE_TRAITS

#include <type_traits>

namespace maf {
	template <bool B, class T = void>
	using enable_if = typename std::enable_if_t<B,T>::type;

	template <class B, class D>
	constexpr bool is_base_of = std::is_base_of_v<B,D>;
}

#endif
