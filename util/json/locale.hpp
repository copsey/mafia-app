#ifndef JSON_LOCALE
#define JSON_LOCALE

#include <locale>

namespace json {
	// The classic "C" ctype adapted for parsing JSON values.
	struct json_ctype: std::ctype<char> {
		explicit json_ctype(std::size_t refs = 0):
			std::ctype<char> {make_table(), false, refs}
		{ }

	private:
		static const mask * make_table();
	};

	// The classic "C" locale adapted for parsing JSON values.
	std::locale get_json_locale();
}

#endif