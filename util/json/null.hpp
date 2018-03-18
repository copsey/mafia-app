#ifndef JSON_NULL
#define JSON_NULL

#include <istream>
#include <ostream>

namespace json {
	// A JSON null value.
	struct j_null { };

	std::istream & operator>> (std::istream & inp, j_null & null);
	std::ostream & operator<< (std::ostream & out, const j_null & null);
}

#endif