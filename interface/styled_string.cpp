#include <algorithm>
#include <stdexcept>

#include "../common/stdlib.h"
#include "styled_string.hpp"

using std::invalid_argument;


string maf::escape_tags(string_view str)
{
	string esc_str = {};

	for (auto ch: str) {
		if (ch == '^' || ch == '{' || ch == '}') esc_str.push_back('^');
		esc_str.push_back(ch);
	}

	return esc_str;
}

std::string maf::substitute_params(std::string_view str_with_params, TextParams const& params)
{
	using iterator_type = std::string_view::const_iterator;
	auto is_brace = [](char ch) { return ch == '{' || ch == '}'; };
	
	std::string str;
	str.reserve(str_with_params.size()); // new string will have (roughly) same length as old string
	
	iterator_type begin = str_with_params.begin();
	iterator_type end = str_with_params.end();
	
	for (iterator_type i = begin, j; ; ) {
		// Find the next brace, either '{' or '}'.
		// Stop when there are no more braces in the string.
		
		j = std::find_if(i, end, is_brace);
		str.append(i, j);
		if (j == end) return str;
		
		// e.g. 1
		//         i            j
		//         |            |
		// "{lorem} ipsum dolor {sit} amet ..."
		
		// e.g. 2
		//          i   j
		//          |   |
		// "Nulla ^{sit^} amet ..."
		
		// If the brace is preceded by a caret '^', treat it as an escaped character:
		// append the brace '{' or '}' and go to the next loop iteration.
		
		if (j != begin) {
			char prev_ch = *(j - 1);
			if (prev_ch == '^') {
				str += *j;
				i = j + 1;
				continue;
			}
		}
		
		// Otherwise, check the type of brace identified. We're trying to form a
		// parameter name here, so a closing brace '}' counts as an error.
		
		if (*j == '}') {
			std::string err_msg = "Too many '}' chars in the following string:\n";
			err_msg.append(str_with_params);
			throw invalid_argument(err_msg);
		}
		
		// Find the range between the two braces, '{' and '}'.
		// It's an error if the closing brace '}' is missing.
		
		i = j + 1;
		j = std::find(i, end, '}');
		if (j == end) {
			std::string err_msg = "Too many '{' chars in the following string:\n";
			err_msg.append(str_with_params);
			throw invalid_argument(err_msg);
		}
		
		// e.g.
		//                       i  j
		//                       |  |
		// "{lorem} ipsum dolor {sit} amet ..."
		
		// Check that the parameter name in the range {i,j} is valid.
		
		if (!is_param_name(i, j)) {
			std::string err_msg = "Invalid parameter name \"";
			err_msg.append(i, j);
			err_msg.append("\" in the following string:\n");
			err_msg.append(str_with_params);
			throw invalid_argument(err_msg);
		}
		
		// Look up the parameter name in the dictionary.
		// It's an error if the parameter is missing.
		
		std::string key{i, j};
		auto val_iter = params.find(key);
		
		if (val_iter == params.end()) {
			string err_msg = "Unrecognised parameter name \"";
			err_msg.append(key);
			err_msg.append("\" in the following string:\n");
			err_msg.append(str_with_params);
			throw invalid_argument(err_msg);
		}
		
		// Append the parameter's value to the string,
		// and prepare for the next loop iteration.
		
		std::string_view val = (*val_iter).second;
		str.append(escape_tags(val));
		
		i = j + 1;
	}
}

maf::Styled_text maf::styled_text_from(string_view str_with_params_and_tags, TextParams const& params)
{
	string str = "";
	Styled_text text = {};

	Styled_string::Style style_stack[8] = {Styled_string::Style::game};
	auto style_i = std::begin(style_stack);
	
	auto str_with_tags = substitute_params(str_with_params_and_tags, params);

	for (auto i = str_with_tags.begin(), j = i, end = str_with_tags.end(); i != end; ) {
		j = std::find(j, end, '^');
		
		if (j == end) {
			str.append(i, j);

			i = j;
		}
		else {
			// e.g.
			//      i                  j
			//      |                  |
			// ...^^ some example text ^cand so on...

			++j;

			if (j == end) {
				string err_msg = "There is a dangling '^' at the end of the following tagged string:\n";
				err_msg.append(str_with_params_and_tags);

				throw invalid_argument(err_msg);
			}
			else {
				// e.g.
				//      i                   j
				//      |                   |
				// ...^^ some example text ^cand so on...

				auto new_style = Styled_string::Style::game;
				bool push_style = false;
				bool pop_style = false;

				switch (auto ch = *j) {
					case 'g':
						push_style = true;
						new_style = Styled_string::Style::game;
						break;

					case 'h':
						push_style = true;
						new_style = Styled_string::Style::help;
						break;

					case 'i':
						push_style = true;
						new_style = Styled_string::Style::italic;
						break;

					case 'c':
						push_style = true;
						new_style = Styled_string::Style::command;
						break;

					case 'T':
						push_style = true;
						new_style = Styled_string::Style::title;
						break;

					case '/':
						pop_style = true;
						break;

					// replace "^^" with "^" in output
					case '^': {
						str.append(i, j-1);
						str += '^';
						i = ++j;
						break;
					}
					
					// replace "^{" with "{" in output
					case '{': {
						str.append(i, j-1);
						str += '{';
						i = ++j;
						break;
					}
					
					// replace "^}" with "}" in output
					case '}': {
						str.append(i, j-1);
						str += '}';
						i = ++j;
						break;
					}

					default: {
						string err_msg = "The tag ^";
						err_msg += ch;
						err_msg.append(" is invalid, and appears in the following tagged string:\n");
						err_msg.append(str_with_params_and_tags);

						throw invalid_argument(err_msg);
					}
				}

				if (push_style) {
					str.append(i, j - 1); // [i,j-1) excludes the tag "^x"

					if (*style_i != new_style) {
						if (!str.empty()) {
							text.emplace_back(str, *style_i);
							str.clear();
						}

						++style_i;

						if (style_i == std::end(style_stack)) {
							string err_msg = "Attempted to push too many style tags onto the stack, in the following tagged string:\n";
							err_msg.append(str_with_params_and_tags);

							throw invalid_argument(err_msg);
						} else {
							*style_i = new_style;
						}
					}

					i = ++j;
				}
				else if (pop_style) {
					str.append(i, j - 1); // [i,j-1) excludes the tag "^/"

					if (!str.empty()) {
						text.emplace_back(str, *style_i);
						str.clear();
					}

					if (style_i == std::begin(style_stack)) {
						string err_msg = "Attempted to pop too many style tags from the stack, in the following tagged string:\n";
						err_msg.append(str_with_params_and_tags);

						throw invalid_argument(err_msg);
					} else {
						--style_i;
					}

					i = ++j;
				}
			}
		}
	}

	if (!str.empty()) {
		text.emplace_back(str, *style_i);
	}

	return text;
}
