#include <utility>

#include "../istream.hpp"
#include "string.hpp"

std::istream& json::read_string(std::istream& in, j_string& str) {
	util::consume(in, '\"');
	
	std::string new_str {};
	for (std::ios::int_type ch = in.get(); in && ch != '\"'; ch = in.get()) {
		switch (ch) {
			case '\\':
				ch = in.get();
				if (in) {
					switch (ch) {
						case '\"':  new_str.push_back('\"'); break;
						case '\\':  new_str.push_back('\\'); break;
						case '/':   new_str.push_back('/'); break;
						case 'b':   new_str.push_back('\b'); break;
						case 'f':   new_str.push_back('\f'); break;
						case 'n':   new_str.push_back('\n'); break;
						case 'r':   new_str.push_back('\r'); break;
						case 't':   new_str.push_back('\t'); break;
						default:    in.setstate(std::ios::failbit);
					}
				}
				break;
			default: 
				new_str.push_back(ch);
		}
	}
	
	if (in) str = std::move(new_str);
	return in;
}

std::ostream& json::write_string(std::ostream& out, const j_string& j_str) {
	out << '\"';
	for (char ch: j_str) {
		switch (ch) {
			case '\"':  out << "\\\""; break;
			case '\\':  out << "\\\\"; break;
			case '/':   out << "\\/"; break;
			case '\b':  out << "\\b"; break;
			case '\f':  out << "\\f"; break;
			case '\n':  out << "\\n"; break;
			case '\r':  out << "\\r"; break;
			case '\t':  out << "\\t"; break;
			default:    out << ch;
		}
	}
	out << '\"';
	
	return out;
}