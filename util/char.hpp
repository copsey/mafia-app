#ifndef MAFIA_UTIL_CHAR
#define MAFIA_UTIL_CHAR

#include <cctype>

namespace maf::util {
	// Check if `c1` and `c2` are equal, ignoring differences in case.
	inline bool equal_up_to_case(char c1, char c2) {
		return std::toupper(c1) == std::toupper(c2);
	}
}

#endif
