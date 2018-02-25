#ifndef JSON_NULL
#define JSON_NULL

#include <cstddef>
#include <istream>
#include <ostream>

namespace json {
	using j_null = std::nullptr_t;
	
	std::istream& read_null(std::istream& in, j_null& null);
	
	std::ostream& write_null(std::ostream& out, const j_null& null);
}

#endif