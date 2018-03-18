#include <utility>

#include "../ios/discard.hpp"
#include "../ios/istream.hpp"
#include "array.hpp"

using util::discard;
using util::istream_backup;

std::istream& json::read_array(std::istream& in, j_array& arr) {
	j_array new_arr {};
	
	// read the opening brace
	if (!(in >> discard('['))) return in;
	
	// read the array's elements
	while (true) {
		istream_backup backup{in};
		
		// all but first element will have a preceding comma
		if (new_arr.size() != 0) in >> discard(',');
		
		j_data data {};
		if (!read_data(in, data)) break;

		new_arr.push_back(std::move(data));
	}
	
	// read the closing brace
	if (!(in >> discard(']'))) return in;
	
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

std::ostream& json::pretty_print_array(
	std::ostream& out,
	const j_array& arr,
	util::repeat_t<const char*, std::string> this_indent)
{
	auto next_indent = this_indent + 1;

	// write the opening brace
	out << '[';
	
	// write the elements
	for (auto i = arr.begin(); i != arr.end(); ++i) {
		const j_data& data = *i;
		
		// write a leading comma for all but the first element
		if (i != arr.begin()) out << ",";
		
		out << "\n" << next_indent;
		pretty_print_data(out, data, next_indent);
	}
	
	// write the closing brace
	out << "\n" << this_indent << ']';
	return out;
}

auto json::operator<< (pretty_print_t<std::ostream> & out, const j_array & arr)
	-> json::pretty_print_t<std::ostream> &
{
	// write the opening brace
	out << '[';
	
	// write the key-value pairs
	out.inc();
	for (auto i = arr.begin(); i != arr.end(); ++i) {
		// write a leading comma for all but the first pair
		if (i != arr.begin()) out << ",";
		
		out << "\n" << out.indent();
		out << (*i);
	}
	out.dec();
	
	// write the closing brace
	out << "\n" << out.indent();
	out << ']';

	return out;
}