#ifndef RIKETI_MEMORY
#define RIKETI_MEMORY

#include <memory>

namespace rkt {
	// Check if `x` and `y` refer to the same location in memory.
	template <typename A, typename B>
	bool same_address(const A & x, const B & y) {
		return std::addressof(x) == std::addressof(y);
	}
	
	// Check if `t1` and `t2` refer to exactly the same object.
	template <typename T>
	bool identical(const volatile T & t1, const volatile T & t2) {
		return same_address(t1, t2);
	}
}

#endif
