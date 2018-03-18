#ifndef JSON_OBJECT
#define JSON_OBJECT

#include <istream>
#include <map>
#include <ostream>
#include <string>

#include "../ios/repeat.hpp"
#include "data.hpp"
#include "pretty_print.hpp"
#include "string.hpp"

namespace json {
	using j_object = std::map<j_string, j_data>;
	
	// Read a JSON object.
	// 
	// \post Set failbit on in if the object could not be parsed.
	std::istream& read_object(std::istream& in, j_object& obj);
	
	std::ostream& write_object(std::ostream& out, const j_object& obj);
	
	std::ostream& pretty_print_object(
		std::ostream& out,
		const j_object& obj,
		util::repeat_t<const char*, std::string> indent = util::repeat("  "));
	
	// Pretty print a j_object to out.
	auto operator<< (pretty_print_t<std::ostream> & out, const j_object & obj)
		-> pretty_print_t<std::ostream> &;
}

#endif