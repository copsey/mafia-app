#include <cctype>
#include <string>

#include "../istream.hpp"
#include "null.hpp"

std::istream& json::read_null(std::istream& in, j_null& null) {
	if ((in.flags() & std::ios::skipws) != 0) std::ws(in);
	
	std::string str {};
	while (std::isalpha(in.peek())) {
		str.push_back(in.get());
	}
	
	if (str != "null") {
		in.setstate(std::ios::failbit);
	}
	
	return in;
}

std::ostream& json::write_null(std::ostream& out, const j_null& null) {
	return out << "null";
}