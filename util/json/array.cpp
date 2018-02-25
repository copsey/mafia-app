#include <utility>

#include "../istream.hpp"
#include "array.hpp"
#include "pretty_print.hpp"

std::istream& json::read_array(std::istream& in, j_array& arr) {
	j_array new_arr {};
	
	// read the opening brace
	if (!util::consume(in, '[')) return in;
	
	// read the array's elements
	while (true) {
		auto pos = in.tellg();
		
		// all but first element will have a preceding comma
		if (new_arr.size() != 0) util::consume(in, ',');
		
		j_data data {};
		read_data(in, data);
		
		if (in) {
			new_arr.push_back(std::move(data));
		} else {
			in.clear();
			in.seekg(pos);
			break;
		}
	}
	
	// read the closing brace
	if (!util::consume(in, ']')) return in;
	
	arr = std::move(new_arr);
	return in;
}

std::ostream& json::write_array(std::ostream& out, const j_array& arr) {
	// write the opening brace
	out << '[';
	
	// write the elements
	for (auto i = arr.begin(); i != arr.end(); ++i) {
		const j_data& data = *i;
		
		// write a leading comma for all but the first element
		if (i != arr.begin()) out << ",";

		write_data(out, data);
	}
	
	// write the closing brace
	out << ']';
	
	return out;
}

std::ostream& json::pretty_print_array(std::ostream& out, const j_array& arr, int indent_level) {
	// write the opening brace
	out << '[';
	
	// write the elements
	for (auto i = arr.begin(); i != arr.end(); ++i) {
		const j_data& data = *i;
		
		// write a leading comma for all but the first element
		if (i != arr.begin()) out << ",";
		
		out << "\n" << indent(indent_level + 1);
		pretty_print_data(out, data, indent_level + 1);
	}
	
	// write the closing brace
	out << "\n";
	for (int l = 0; l < indent_level; ++l) out << "  ";
	out << ']';
	
	return out;
}