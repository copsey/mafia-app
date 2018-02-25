#include <utility>

#include "../istream.hpp"
#include "object.hpp"
#include "pretty_print.hpp"
#include "string.hpp"

namespace json {
	std::istream& read_pair(std::istream& in, j_object& obj);
}

std::istream& json::read_pair(std::istream& in, j_object& obj) {
	j_string key;
	read_string(in, key);
	if (obj.count(key) > 0) in.setstate(std::ios::failbit);
	
	util::consume(in, ':');
	
	j_data data;
	read_data(in, data);
	
	if (in) obj[key] = std::move(data);
	
	return in;
}

std::istream& json::read_object(std::istream& in, j_object& obj) {
	j_object new_obj {};
	
	// read the opening brace
	if (!util::consume(in, '{')) return in;
	
	// read the key-value pairs
	while (true) {
		auto pos = in.tellg();
		
		// all but first pair will have a preceding comma
		if (new_obj.size() != 0) util::consume(in, ',');
		
		read_pair(in, new_obj);
		
		if (!in) {
			in.clear();
			in.seekg(pos);
			break;
		}
	}
	
	// read the closing brace
	if (!util::consume(in, '}')) return in;
	
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

std::ostream& json::pretty_print_object(std::ostream& out, const j_object& obj, int indent_level) {
	// write the opening brace
	out << '{';
	
	// write the key-value pairs
	for (auto i = obj.begin(); i != obj.end(); ++i) {
		const j_string& key  = (*i).first;
		const j_data&   data = (*i).second;
		
		// write a leading comma for all but the first pair
		if (i != obj.begin()) out << ",";
		
		out << "\n" << indent(indent_level + 1);
		write_string(out, key);
		out << ": ";
		pretty_print_data(out, data, indent_level + 1);
	}
	
	// write the closing brace
	out << "\n";
	for (int l = 0; l < indent_level; ++l) out << "  ";
	out << '}';
	
	return out;
}