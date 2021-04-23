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

	// Get a subview of `str` with all whitespace removed from the end.
	[[nodiscard]] inline auto drop_whitespace_from_end(std::string_view str)
	-> std::string_view {
		auto rbegin = str.rbegin(), rend = str.rend();
		auto rmid = std::find_if_not(rbegin, rend, is_whitespace);
		auto mid = rmid.base();
		return make_string_view(str.begin(), mid);
	}

	inline void erase_whitespace_from_end(std::string & str) {
		auto rbegin = str.rbegin(), rend = str.rend();
		auto rmid = std::find_if_not(rbegin, rend, is_whitespace);
		auto mid = rmid.base();
		str.erase(mid, str.end());
	}
}

#endif
