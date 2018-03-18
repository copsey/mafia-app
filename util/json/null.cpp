#include "null.hpp"

#include "../ios/keyword.hpp"

using util::keyword;

std::istream & json::operator>> (std::istream & inp, j_null & null) {
	return inp >> keyword("null");
}

std::ostream & json::operator<< (std::ostream& out, const j_null & null) {
	return out << keyword("null");
}