#ifndef MAFIA_UTIL_PARSE
#define MAFIA_UTIL_PARSE

#include <charconv>
#include <string_view>

namespace maf::util {
	inline auto from_chars(std::string_view str, int & value, int base = 10)
	-> std::from_chars_result {
		auto begin = str.data();
		auto end = str.data() + str.size();
		return std::from_chars(begin, end, value, base);
	}
}

#endif
