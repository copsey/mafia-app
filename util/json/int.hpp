#ifndef JSON_INTEGER
#define JSON_INTEGER

#include <cstdint>
#include <istream>
#include <ostream>

namespace json {
	using j_int = std::int32_t;
	
	std::istream& read_int(std::istream& in, j_int& i);
	
	std::ostream& write_int(std::ostream& out, const j_int& i);
}

#endif