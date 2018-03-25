#include "parse.hpp"

#include <fstream>

#include "locale.hpp"
#include "pretty_print.hpp"

void json::prep_stream(std::ios& stream) {
	stream.flags(std::ios::dec | std::ios::boolalpha | std::ios::skipws);
	stream.imbue(get_json_locale());
}

bool json::load_json(j_data & val, const std::string & path) {
	std::ifstream inp {path};
	prep_stream(inp);

	read_data(inp, val);

	return static_cast<bool>(inp);
}

bool json::save_json(const j_data & val, const std::string & path, bool pretty_print) {
	std::ofstream out {path};
	prep_stream(out);

	if (pretty_print) {
		auto pretty_out = use_pretty_print(out);
		pretty_out << val;
	} else {
		write_data(out, val);
	}

	return static_cast<bool>(out);
}