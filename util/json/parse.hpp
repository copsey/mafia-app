#ifndef JSON_PARSE
#define JSON_PARSE

#include <ios>
#include <string>

#include "data.hpp"

namespace json {
	// Prepare a stream for handling JSON data.
	void prep_stream(std::ios & str);
	
	// Read a JSON value from the file at path.
	// 
	// \return true if a JSON value was read successfully; false otherwise.
	bool load_json(j_data & val, const std::string & path);
	
	// Write a JSON value to the file at path, optionally with pretty-printing
	// enabled.
	// 
	// \return true if the JSON value was written successfully; false otherwise.
	bool save_json(const j_data & val, const std::string & path, bool pretty_print = false);
}

#endif