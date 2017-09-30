#ifndef RIKETI_STRING
#define RIKETI_STRING

#include <string>

namespace rkt {
	// Check if `s1` and `s2` are equal, ignoring differences in case.
	bool equal_up_to_case(const std::string & s1, const std::string & s2);
	
	// Read the contents of `str` as an `int`,
	// using `base` as the numeric base.
	//
	// This is the same as `std::stoi(str, 0, base)`, except that
	// `std::invalid_argument` is thrown if there are excess `char`s at
	// the end of `str` which were not used in the conversion.
	int s_to_i(const std::string & str, int base = 10);
	
	// Read the contents of `str` as a `long`,
	// using `base` as the numeric base.
	//
	// This is the same as `std::stol(str, 0, base)`, except that
	// `std::invalid_argument` is thrown if there are excess `char`s at
	// the end of `str` which were not used in the conversion.
	long s_to_l(const std::string & str, int base = 10);
	
	// Read the contents of `str` as a `long long`,
	// using `base` as the numeric base.
	//
	// This is the same as `std::stoll(str, 0, base)`, except that
	// `std::invalid_argument` is thrown if there are excess `char`s at
	// the end of `str` which were not used in the conversion.
	long long s_to_ll(const std::string & str, int base = 10);
	
	// Read the contents of `str` as an `unsigned long`,
	// using `base` as the numeric base.
	//
	// This is the same as `std::stoul(str, 0, base)`, except that
	// `std::invalid_argument` is thrown if there are excess `char`s at
	// the end of `str` which were not used in the conversion.
	unsigned long s_to_ul(const std::string & str, int base = 10);
	
	// Read the contents of `str` as an `unsigned long long`,
	// using `base` as the numeric base.
	//
	// This is the same as `std::stoull(str, 0, base)`, except that
	// `std::invalid_argument` is thrown if there are excess `char`s at
	// the end of `str` which were not used in the conversion.
	unsigned long long s_to_ull(const std::string & str, int base = 10);
}

#endif
