#include "../ios/keyword.hpp"
#include "null.hpp"

using util::keyword;

std::istream& json::read_null(std::istream& inp, j_null& null) {
	return inp >> keyword("null");
}

std::ostream& json::write_null(std::ostream& out, const j_null& null) {
	return out << keyword("null");
}