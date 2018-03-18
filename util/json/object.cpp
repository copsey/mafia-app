#include <utility>

#include "../ios/discard.hpp"
#include "../ios/istream.hpp"
#include "object.hpp"
#include "string.hpp"

using util::discard;
using util::istream_backup;

namespace json {
	std::istream& read_pair(std::istream& in, j_object& obj);
}

std::istream& json::read_pair(std::istream& in, j_object& obj) {
	j_string key;
	read_string(in, key);
	if (obj.count(key) > 0) in.setstate(std::ios::failbit);
	
	in >> discard(':');
	
	j_data data;
	read_data(in, data);
	
	if (in) obj[key] = std::move(data);
	
	return in;
}

std::istream& json::read_object(std::istream& in, j_object& obj) {
	j_object new_obj {};
	
	// read the opening brace
	if (!(in >> discard('{'))) return in;
	
	// read the key-value pairs
	while (true) {
		istream_backup backup{in};
		
		// all but first pair will have a preceding comma
		if (new_obj.size() != 0) in >> discard(',');
		
		if (!read_pair(in, new_obj)) break;
	}
	
	// read the closing brace
	if (!(in >> discard('}'))) return in;
	
	obj = std::move(new_obj);
	return in;
}

std::ostream& json::write_object(std::ostream& out, const j_object& obj) {
	// write the opening brace
	out << '{';
	
	// write the key-value pairs
	for (auto i = obj.begin(); i != obj.end(); ++i) {
		const j_string& key  = (*i).first;
		const j_data&   data = (*i).second;
		
		// write a leading comma for all but the first pair
		if (i != obj.begin()) out << ",";
		
		write_string(out, key);
		out << ":";
		write_data(out, data);
	}
	
	// write the closing brace
	out << '}';
	
	return out;
}

auto json::operator<< (pretty_print_t<std::ostream> & out, const j_object & obj)
	-> json::pretty_print_t<std::ostream> &
{
	// write the opening brace
	out << '{';
	
	// write the key-value pairs
	{
		auto block = get_indent_block(out);

		for (auto i = obj.begin(); i != obj.end(); ++i) {
			const j_string& key  = (*i).first;
			const j_data&   data = (*i).second;
			
			// write a leading comma for all but the first pair
			if (i != obj.begin()) out << ",";
			
			out << "\n" << out.indent();
			out << key << ": " << data;
		}
	}
	
	// write the closing brace
	out << "\n" << out.indent();
	out << '}';
	
	return out;
}