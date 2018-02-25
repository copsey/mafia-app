#include <cctype>
#include <string>

#include "../istream.hpp"
#include "bool.hpp"

std::istream& json::read_bool(std::istream& in, j_bool& b) {
	if ((in.flags() & std::ios::skipws) != 0) std::ws(in);
	
	std::string str {};
	while (std::isalpha(in.peek())) {
		str.push_back(in.get());
	}
	
	if (str == "true") {
		b = true;
	} else if (str == "false") {
		b = false;
	} else {
		in.setstate(std::ios::failbit);
	}
	
	return in;
}

std::ostream& json::write_bool(std::ostream& out, const j_bool& b) {
	if (b) {
		out << "true";
	} else {
		out << "false";
	}
	
	return out;
}