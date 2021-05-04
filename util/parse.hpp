#ifndef MAFIA_UTIL_PARSE_H
#define MAFIA_UTIL_PARSE_H

#include <charconv>

#include "string.hpp"

namespace maf::util {
	inline auto from_chars(string_view str, int & value, int base = 10)
	-> std::from_chars_result {
		auto begin = str.data();
		auto end = str.data() + str.size();
		return std::from_chars(begin, end, value, base);
	}
}

#endif
