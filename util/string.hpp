#ifndef MAFIA_UTIL_STRING
#define MAFIA_UTIL_STRING

#include <string>
#include <string_view>

#include "char.hpp"
#include "misc.hpp"

namespace maf {
	using std::literals::operator""sv;

	using string = std::string;
	using string_view = std::string_view;
}

namespace maf::util {
	// Check if `s1` and `s2` are equal, ignoring differences in case.
	inline bool equal_up_to_case(string_view s1, string_view s2) {
		if (s1.size() != s2.size()) return false;

		for (index i = 0; i != s1.size(); ++i) {
			if (!equal_up_to_case(s1[i], s2[i])) return false;
		}

		return true;
	}

	template <typename Iterator>
	string_view make_string_view(Iterator begin, Iterator end) {
		auto data = static_cast<const char *>(begin);
		auto length = static_cast<string_view::size_type>(end - begin);
		return string_view{data, length};
	}

	// Get a subview of `str` with all whitespace removed from the end.
	[[nodiscard]]
	inline string_view drop_whitespace_from_end(string_view str) {
		auto rbegin = str.rbegin();
		auto rend   = str.rend();
		auto rmid   = std::find_if_not(rbegin, rend, is_whitespace);

		auto begin = str.begin();
		auto mid   = rmid.base();
		return make_string_view(begin, mid);
	}

	inline void erase_whitespace_from_end(std::string & str) {
		auto rbegin = str.rbegin();
		auto rend   = str.rend();
		auto rmid   = std::find_if_not(rbegin, rend, is_whitespace);

		auto mid = rmid.base();
		auto end = str.end();
		str.erase(mid, end);
	}
}

#endif
