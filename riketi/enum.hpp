#ifndef RIKETI_ENUM
#define RIKETI_ENUM

#include <type_traits>

namespace rkt {
	namespace enums {
		// Get the underlying type of `E`.
		template <typename E>
		using underlying_type = typename std::underlying_type<E>::type;
		
		// Get the underlying value of `x`.
		template <typename E>
		constexpr underlying_type<E> value(const E & x) {
			return static_cast<underlying_type<E>>(x);
		}
	}
}

#endif
