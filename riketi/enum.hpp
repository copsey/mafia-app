#ifndef RIKETI_ENUM
#define RIKETI_ENUM

#include <type_traits>

namespace rkt {
	template <typename E>
	constexpr std::underlying_type_t<E> underlying_value(E e) {
		return static_cast<std::underlying_type_t<E>>(e);
	}
}

#endif
