#ifndef MAFIA_UTIL_TYPE_TRAITS_H
#define MAFIA_UTIL_TYPE_TRAITS_H

#include <type_traits>

namespace maf {
	template <bool B, class T = void>
	using enable_if = std::enable_if_t<B,T>;

	template <class X, class Y>
	constexpr bool is_same = std::is_same_v<X,Y>;

	template <class B, class D>
	constexpr bool is_base_of = std::is_base_of_v<B,D>;

	template <typename T>
	using decay = std::decay_t<T>;
}

#endif
