#include <iterator>

#include "styled_string.hpp"

std::string maf::substitute_params(std::string_view input, TextParams const& params)
{
	using iterator_type = std::string_view::const_iterator;
	auto is_brace = [](char ch) { return ch == '{' || ch == '}'; };
	
	std::string output;
	output.reserve(input.size()); // new string will have (roughly) same
	                              // length as old string
	
	iterator_type begin = input.begin();
	iterator_type end = input.end();
	
	for (auto i = begin; ; ) {
		// Find the next brace, either '{' or '}'.
		// Append all of the text up to (but excluding) the brace.
		// Stop when there are no more braces in the string.
		
		auto j = std::find_if(i, end, is_brace);
		output.append(i, j);
		if (j == end) return output;
		
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
			output += *j;
			i = j; ++i;
			continue;
		}
		
		// Otherwise, check the type of brace identified. We're trying to form
		// a parameter name here, so a closing brace '}' counts as an error.
		
		if (*j == '}') {
			throw substitute_params_error{substitute_params_errc::too_many_closing_braces, j};
		}
		
		// Find the range between the two braces, '{' and '}'.
		// It's an error if the closing brace '}' is missing.
		
		i = j; ++i;
		j = std::find(i, end, '}');
		if (j == end) {
			throw substitute_params_error{substitute_params_errc::too_many_opening_braces, i - 1};
		}
		
		// e.g.
		//                       i  j
		//                       |  |
		// "{lorem} ipsum dolor {sit} amet ..."
		
		// Check that the parameter name in the range {i,j} is valid.
		
		if (!is_param_name(i, j)) {
			throw substitute_params_error{substitute_params_errc::invalid_parameter_name, i, j};
		}
		
		// Look up the parameter name in the dictionary.
		// It's an error if the parameter is missing.
		
		auto length = static_cast<std::string_view::size_type>(j - i);
		std::string_view key{i, length};
		auto val_iter = params.find(key);
		
		if (val_iter == params.end()) {
			throw substitute_params_error{substitute_params_errc::missing_parameter, i, j};
		}
		
		// Append the parameter's value to the string,
		// and prepare for the next loop iteration.
		
		std::string_view val = (*val_iter).second;
		output += val;
		
		i = j; ++i;
	}
}

maf::Styled_text maf::apply_tags(std::string_view input)
{
	using iterator_type = std::string_view::const_iterator;
	
	std::string current_block;
	Styled_text output;
	Styled_string::Style style_stack[9] = {Styled_string::Style::game};
	auto style_iter = std::begin(style_stack);
	
	iterator_type begin = input.begin();
	iterator_type end = input.end();

	for (auto i = begin; ; ) {
		// Find the next caret '^'.
		// Copy all of the text up to (but excluding) the caret.
		// Stop when there are no more carets in the input.
		
		auto j = std::find(i, end, '^');
		current_block.append(i, j);
		if (j == end) {
			if (!current_block.empty()) output.emplace_back(current_block, *style_iter);
			return output;
		}
		
		// e.g.
		//      i                  j
		//      |                  |
		// ...^^ some example text ^cand so on...
		
		// Check there's a character after the caret.
		// It's an error if there are no more characters.
		
		i = j; ++i;
		if (i == end) {
			throw apply_tags_error{apply_tags_errc::dangling_caret, j};
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
				current_block += ch;
				break;

			default:
				throw apply_tags_error{apply_tags_errc::invalid_tag, j};
		}
		
		// Check if the style needs to be changed.
		// If so, append the current block of text and update the style stack.
		
		bool style_has_changed = (push_style && *style_iter != new_style) || pop_style;

		if (style_has_changed) {
			if (!current_block.empty()) {
				output.emplace_back(current_block, *style_iter);
				current_block.clear();
			}
			
			if (push_style) {
				++style_iter;
				if (style_iter == std::end(style_stack)) {
					throw apply_tags_error{apply_tags_errc::too_many_styles, i};
				}
				
				*style_iter = new_style;
			} else { // pop_style
				if (style_iter == std::begin(style_stack)) {
					throw apply_tags_error{apply_tags_errc::extra_closing_tag, i};
				}
				--style_iter;
			}
		}
		
		// Prepare for the next loop iteration.
		
		++i;
	}
}
