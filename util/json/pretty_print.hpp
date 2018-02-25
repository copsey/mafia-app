#ifndef JSON_PRETTY_PRINT
#define JSON_PRETTY_PRINT

#include <ostream>

namespace json {
	struct indent_t { unsigned l {0}; };
	
	// Write "  " to out a total of ind.l times.
	inline std::ostream& operator<< (std::ostream& out, indent_t ind) {
		for (unsigned i = 0; i < ind.l; ++i) out << "  ";
		return out;
	}
	
	// Make an object which will write "  " a total of indent_level times
	// when printed.
	inline constexpr indent_t indent(unsigned indent_level) {
		return indent_t {indent_level};
	}
}

#endif