#ifndef MAFIA_UTIL_CHAR
#define MAFIA_UTIL_CHAR

#include <cctype>

namespace maf::util {
	// Check if `c1` and `c2` are equal, ignoring differences in case.
	inline bool equal_up_to_case(char c1, char c2) {
		return std::toupper(c1) == std::toupper(c2);
	}

	// Check if `ch` is a decimal digit.
	inline bool is_digit(char ch) {
		switch (ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return true;
		default:
			return false;
		}
	}

	// Check if `ch` produces whitespace.
	inline bool is_whitespace(char ch) {
		switch (ch) {
		case ' ': case '\t': case '\n': case '\r':
			return true;
		default:
			return false;
		}
	}
}

#endif
