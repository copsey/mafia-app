#include "styled_string.hpp"

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
