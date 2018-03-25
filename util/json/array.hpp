#ifndef JSON_ARRAY
#define JSON_ARRAY

#include <istream>
#include <ostream>
#include <vector>

#include "data.hpp"
#include "pretty_print.hpp"

namespace json {
	using j_array = std::vector<j_data>;
	
	std::istream& read_array(std::istream& in, j_array& arr);
	
	std::ostream& write_array(std::ostream& out, const j_array& arr);
	
	// Pretty print a JSON array to out.
	auto operator<< (pretty_print_t<std::ostream> & out, const j_array & arr)
		-> pretty_print_t<std::ostream> &;
}

#endif