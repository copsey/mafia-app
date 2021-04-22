#ifndef MAFIA_UTIL_STRING
#define MAFIA_UTIL_STRING

#include <string>
#include <string_view>

#include "char.hpp"

namespace maf::util {
	// Check if `s1` and `s2` are equal, ignoring differences in case.
	inline bool equal_up_to_case(std::string_view s1, std::string_view s2) {
		if (s1.size() != s2.size()) return false;

		for (std::string_view::size_type i = 0; i != s1.size(); ++i) {
			if (!equal_up_to_case(s1[i], s2[i])) return false;
		}

		return true;
	}

	inline auto make_string_view(std::string_view::iterator begin,
	                             std::string_view::iterator end)
	-> std::string_view {
		auto length = static_cast<std::string_view::size_type>(end - begin);
		return std::string_view{begin, length};
	}
}

#endif
