#include "locale.hpp"

#include <vector>

const json::json_ctype::mask * json::json_ctype::make_table() {
	// use classic "C" locale as base
	static const auto classic_tbl = std::ctype<char>::classic_table();
	static std::vector<mask> tbl {classic_tbl, classic_tbl + table_size};

	static bool run_update = true;
	if (run_update) {
		// restrict characters that are considered whitespace
		tbl['\v'] &= ~space;
		tbl['\f'] &= ~space;

		// only need to make the updates once
		run_update = false;
	}

	return &tbl[0];
}

std::locale json::get_json_locale() {
	return std::locale {std::locale::classic(), new json::json_ctype};
}