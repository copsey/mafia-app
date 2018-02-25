#ifndef JSON_OBJECT
#define JSON_OBJECT

#include <istream>
#include <map>
#include <ostream>
#include <string>

#include "data.hpp"
#include "string.hpp"

namespace json {
	using j_object = std::map<j_string, j_data>;
	
	// Read a JSON object.
	// 
	// \post Set failbit on in if the object could not be parsed.
	std::istream& read_object(std::istream& in, j_object& obj);
	
	std::ostream& write_object(std::ostream& out, const j_object& obj);
	
	std::ostream& pretty_print_object(std::ostream& out, const j_object& obj, int indent_level = 0);
}

#endif