#ifndef JSON_ARRAY
#define JSON_ARRAY

#include <istream>
#include <ostream>
#include <vector>

#include "../ios/repeat.hpp"
#include "data.hpp"

namespace json {
	using j_array = std::vector<j_data>;
	
	std::istream& read_array(std::istream& in, j_array& arr);
	
	std::ostream& write_array(std::ostream& out, const j_array& arr);
	
	std::ostream& pretty_print_array(
		std::ostream& out,
		const j_array& arr,
		util::repeat_t<const char*, std::string> indent = util::repeat("  "));
}

#endif