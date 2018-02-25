#ifndef JSON_BOOLEAN
#define JSON_BOOLEAN

#include <istream>
#include <ostream>

namespace json {
	using j_bool = bool;
	
	std::istream& read_bool(std::istream& in, j_bool& b);
	
	std::ostream& write_bool(std::ostream& out, const j_bool& b);
}

#endif