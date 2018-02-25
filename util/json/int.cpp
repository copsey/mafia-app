#include "int.hpp"

std::istream& json::read_int(std::istream& in, j_int& i) {
	j_int new_i = 0;
	
	in >> new_i;
	
	if (in) i = new_i;
	return in;
}

std::ostream& json::write_int(std::ostream& out, const j_int& i) {
	return out << i;
}