#include <cctype>
#include <cstddef>
#include <stdexcept>

#include "string.hpp"

bool rkt::equal_up_to_case(std::string_view s1, std::string_view s2) {
	if (s1.size() != s2.size()) return false;
	
	for (std::string_view::size_type i{0}; i != s1.size(); ++i) {
		if (std::toupper(s1[i]) != std::toupper(s2[i])) return false;
	}
	
	return true;
}

int rkt::s_to_i(const std::string & str, int base) {
	std::size_t pos;
	auto x = std::stoi(str, &pos, base);
	
	if (pos < str.size()) {
		throw std::invalid_argument{"rkt: s_to_i: excess chars at end"};
	}
	
	return x;
}

long rkt::s_to_l(const std::string & str, int base) {
	std::size_t pos;
	auto x = std::stol(str, &pos, base);
	
	if (pos < str.size()) {
		throw std::invalid_argument{"rkt: s_to_l: excess chars at end"};
	}
	
	return x;
}

long long rkt::s_to_ll(const std::string & str, int base) {
	std::size_t pos;
	auto x = std::stoll(str, &pos, base);
	
	if (pos < str.size()) {
		throw std::invalid_argument{"rkt: s_to_ll: excess chars at end"};
	}
	
	return x;
}

unsigned long rkt::s_to_ul(const std::string & str, int base) {
	std::size_t pos;
	auto x = std::stoul(str, &pos, base);
	
	if (pos < str.size()) {
		throw std::invalid_argument{"rkt: s_to_ul: excess chars at end"};
	}
	
	return x;
}

unsigned long long rkt::s_to_ull(const std::string & str, int base) {
	std::size_t pos;
	auto x = std::stoull(str, &pos, base);
	
	if (pos < str.size()) {
		throw std::invalid_argument{"rkt: s_to_ull: excess chars at end"};
	}
	
	return x;
}
