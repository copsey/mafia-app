#ifndef JSON_STRING
#define JSON_STRING

#include <istream>
#include <string>

namespace json {
	using j_string = std::string;
	
	// Read a JSON quoted string into data.
	//
	// \note Unicode character literals (\uxxxx) are not supported.
	//
	// \post Set failbit on in if the string could not be parsed.
	std::istream& read_string(std::istream& in, j_string& str);
	
	// Write a JSON quoted string to out.
	std::ostream& write_string(std::ostream& out, const j_string& str);
}

#endif