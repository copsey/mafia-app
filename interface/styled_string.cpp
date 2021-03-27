#include <iterator>
#include <stdexcept>

#include "styled_string.hpp"

std::string maf::substitute_params(std::string_view str_with_params, TextParams const& params)
{
	using iterator_type = std::string_view::const_iterator;
	auto is_brace = [](char ch) { return ch == '{' || ch == '}'; };
	
	std::string str;
	str.reserve(str_with_params.size()); // new string will have (roughly)
	                                     // same length as old string
	
	iterator_type begin = str_with_params.begin();
	iterator_type end = str_with_params.end();
	
	for (auto i = begin; ; ) {
		// Find the next brace, either '{' or '}'.
		// Append all of the text up to (but excluding) the brace.
		// Stop when there are no more braces in the string.
		
		auto j = std::find_if(i, end, is_brace);
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
		
		// If the brace is preceded by a caret '^',
		// append the brace and go to the next loop iteration.
		
		if (j != begin && *(j - 1) == '^') {
			str += *j;
			i = j; ++i;
			continue;
		}
		
		// Otherwise, check the type of brace identified. We're trying to form
		// a parameter name here, so a closing brace '}' counts as an error.
		
		if (*j == '}') {
			std::string err_msg = "Too many '}' chars in the following string:\n";
			err_msg.append(str_with_params);
			throw std::invalid_argument(err_msg);
		}
		
		// Find the range between the two braces, '{' and '}'.
		// It's an error if the closing brace '}' is missing.
		
		i = j; ++i;
		j = std::find(i, end, '}');
		if (j == end) {
			std::string err_msg = "Too many '{' chars in the following string:\n";
			err_msg.append(str_with_params);
			throw std::invalid_argument(err_msg);
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
			throw std::invalid_argument(err_msg);
		}
		
		// Look up the parameter name in the dictionary.
		// It's an error if the parameter is missing.
		
		auto length = static_cast<std::string_view::size_type>(j - i);
		std::string_view key{i, length};
		auto val_iter = params.find(key);
		
		if (val_iter == params.end()) {
			std::string err_msg = "Unrecognised parameter name \"";
			err_msg.append(key);
			err_msg.append("\" in the following string:\n");
			err_msg.append(str_with_params);
			throw std::invalid_argument(err_msg);
		}
		
		// Append the parameter's value to the string,
		// and prepare for the next loop iteration.
		
		std::string_view val = (*val_iter).second;
		str.append(val);
		
		i = j; ++i;
	}
}

maf::Styled_text maf::apply_tags(std::string_view str_with_tags)
{
	using iterator_type = std::string_view::const_iterator;
	
	std::string str;
	Styled_text text;
	Styled_string::Style style_stack[9] = {Styled_string::Style::game};
	auto style_iter = std::begin(style_stack);
	
	iterator_type begin = str_with_tags.begin();
	iterator_type end = str_with_tags.end();

	for (auto i = begin; ; ) {
		// Find the next caret '^'.
		// Copy all of the text up to (but excluding) the caret.
		// Stop when there are no more carets in the input.
		
		auto j = std::find(i, end, '^');
		str.append(i, j);
		if (j == end) {
			if (!str.empty()) text.emplace_back(str, *style_iter);
			return text;
		}
		
		// e.g.
		//      i                  j
		//      |                  |
		// ...^^ some example text ^cand so on...
		
		// Check there's a character after the caret.
		// It's an error if there are no more characters.
		
		i = j; ++i;
		if (i == end) {
			std::string err_msg = "There is a dangling '^' at the end of the following tagged string:\n";
			err_msg.append(str_with_tags);
			throw std::invalid_argument(err_msg);
		}
		
		// e.g.
		//                         ji
		//                         ||
		// ...^^ some example text ^cand so on...

		auto new_style = Styled_string::Style::game;
		bool push_style = false;
		bool pop_style = false;

		switch (auto ch = *i) {
			case 'g':
			case 'h':
			case 'i':
			case 'c':
			case 'T':
				push_style = true;
				new_style = get_style(ch);
				break;

			case '/':
				pop_style = true;
				break;

			// Replace "^^" with "^" in the output.
			// Likewise for "^{" -> "{" and "^}" -> "}".
			case '^':
			case '{':
			case '}':
				str += ch;
				break;

			default: {
				std::string err_msg = "The tag ^";
				err_msg += ch;
				err_msg.append(" is invalid, and appears in the following tagged string:\n");
				err_msg.append(str_with_tags);
				throw std::invalid_argument(err_msg);
			}
		}
		
		// Check if the style needs to be changed.
		// If so, append the current block of text and update the style stack.
		
		bool style_has_changed = (push_style && *style_iter != new_style) || pop_style;

		if (style_has_changed) {
			if (!str.empty()) {
				text.emplace_back(str, *style_iter);
				str.clear();
			}
			
			if (push_style) {
				++style_iter;
				if (style_iter == std::end(style_stack)) {
					std::string err_msg = "Attempted to push too many style tags onto the stack, in the following tagged string:\n";
					err_msg.append(str_with_tags);
					throw std::invalid_argument(err_msg);
				}
				
				*style_iter = new_style;
			} else { // pop_style
				if (style_iter == std::begin(style_stack)) {
					std::string err_msg = "Attempted to pop too many style tags from the stack, in the following tagged string:\n";
					err_msg.append(str_with_tags);
					throw std::invalid_argument(err_msg);
				}
				--style_iter;
			}
		}
		
		// Prepare for the next loop iteration.
		
		++i;
	}
}

maf::Styled_text maf::styled_text_from(std::string_view str_with_params_and_tags, TextParams const& params)
{
	auto str_with_tags = substitute_params(str_with_params_and_tags, params);
	return apply_tags(str_with_tags);
}
