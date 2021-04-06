#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <algorithm>
#include <bitset>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace maf {
	namespace _impl {
		// Check if `ch` is a curly brace, '{' or '}'.
		inline bool is_brace(char ch)
		{
			return ch == '{' || ch == '}';
		}
	
		// Check if `ch` is treated as a special character by `format_text`.
		// Note: Backslashes need to be checked separately.
		inline bool is_formatting_char(char ch)
		{
			return ch == '=' || ch == '_' || ch == '%' || ch == '@';
		}

		// Check if `ch` forms an escape sequence by prefixing it with a
		// backslash '\'.
		inline bool is_escapable_char(char ch)
		{
			return is_brace(ch) || is_formatting_char(ch) || ch == '\\';
		}
	}

	// Create a new string based on the range of characters in `{begin,end}`,
	// but with each escapable character prefixed by a backslash '\'.
	//
	// @pre `{begin,end}` is a valid range.
	template <typename ForwardIter>
	std::string escaped(ForwardIter begin, ForwardIter end)
	{
		std::string output;

		for (auto i = begin; ; ) {
			auto j = std::find_if(i, end, _impl::is_escapable_char);
			output.append(i, j);
			if (j == end) return output;
			output += {'\\', *j};
			i = std::next(j);
		}
	}

	// Create a new string based on `str_view`, but with each escapable
	// character prefixed by a backslash '\'.
	//
	// @example `escaped("My {string}") == "My \\{string\\}"`
	// @example `escaped("under_score") == "under\\_score"`
	inline std::string escaped(std::string_view str_view)
	{
		return escaped(str_view.begin(), str_view.end());
	}
	
	// Check if the range of characters in `{begin,end}` gives an allowed name
	// for a styled-text parameter. This is true if it matches the regex
	// /^[a-zA-Z0-9_\.]+$/.
	//
	// @pre `{begin,end}` is a valid range.
	template <typename InputIter>
	bool is_param_name(InputIter begin, InputIter end)
	{
		for (auto i = begin; i != end; ++i) {
			switch (*i) {
				case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
				case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
				case 's': case 't': case 'u': case 'v': case 'w': case 'x':
				case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
				case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
				case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
				case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
				case 'W': case 'X': case 'Y': case 'Z': case '_': case '.':
				case '0': case '1': case '2': case '3': case '4': case '5':
				case '6': case '7': case '8': case '9':
					break;
				
				default:
					return false;
			}
		}
		
		return begin != end;
	}
	
	// Check if `param` is an allowed name for a styled-text parameter.
	// This is true if it matches the regex /^[a-zA-Z0-9_\.]+$/.
	inline bool is_param_name(std::string_view param)
	{
		return is_param_name(param.begin(), param.end());
	}

	// A map from strings (treated as parameter names) to strings (treated as
	// substitutions).
	//
	// Note that the map only holds views into its keys, whereas the values
	// are fully owned. Typically the parameter name will be a compile-time
	// constant.
	using TextParams = std::map<std::string_view, std::string>;

	// Error code for exceptions that can be thrown when calling
	// `substitute_params`.
	enum class substitute_params_errc {
		invalid_parameter_name,
		missing_parameter,
		too_many_opening_braces,
		too_many_closing_braces
	};

	// Type for exceptions that can be thrown when calling
	// `substitute_params`.
	struct substitute_params_error {
		// Error code for this exception.
		substitute_params_errc errc;

		// Position in input string where error occurred.
		std::string_view::iterator i;
		
		// (Optional) iterator that forms a range {i,j} when paired with i.
		//
		// Only used by invalid_parameter_name and missing_parameter to
		// signify where in the input string the parameter name occurs.
		std::string_view::iterator j{};

		substitute_params_error(substitute_params_errc errc,
								std::string_view::iterator i)
		: errc{errc}, i{i}
		{ }
		
		substitute_params_error(substitute_params_errc errc,
								std::string_view::iterator i,
								std::string_view::iterator j)
		: errc{errc}, i{i}, j{j}
		{ }
	};

	// Find all strings of the form "{substitute}" in the range `{begin,end}`,
	// and replace them with the corresponding value from `params`.
	//
	// Braces preceded by backslashes '\' are considered escaped and are
	// ignored.
	//
	// @example If `params` is `{ {"word1", "NEW"}, {"word2", "STRING"} }`,
	// then
	//     "This is my {word1} {word2} with \{braces\}."
	// will be transformed into
	//     "This is my NEW STRING with \{braces\}."
	//
	// @pre `{begin,end}` is a valid range.
	//
	// @throws `substitute_params_error` with code `invalid_parameter_name` if
	// an invalid parameter name is encountered. (See `is_param_name` for more
	// information.)
	//
	// @throws `substitute_params_error` with code `missing_parameter` for any
	// parameter names that cannot be found in `params`.
	//
	// @throws `substitute_params_error` with code `too_many_opening_braces`
	// or `too_many_closing_braces` if there are different numbers of
	// (unescaped) opening braces '{' and closing braces '}'.
	template <typename BidirectionalIter>
	std::string substitute_params(BidirectionalIter begin,
								  BidirectionalIter end,
								  TextParams const& params)
	{
		std::string output;
		
		for (BidirectionalIter i = begin, j; /**/; i = std::next(j)) {
			j = std::find_if(i, end, _impl::is_brace);
			output.append(i, j);
			if (j == end) return output;
			
			// e.g. 1
			//         i            j
			//         |            |
			// "{Lorem} ipsum dolor {sit} amet"
			
			// e.g. 2
			//          i   j
			//          |   |
			// "Nulla \{sit\} amet"
			
			if (j != begin && *std::prev(j) == '\\') {
				output += *j;
				continue;
			}
			
			if (*j == '}') {
				auto errc = substitute_params_errc::too_many_closing_braces;
				throw substitute_params_error{errc, j};
			}
			
			i = std::next(j);
			j = std::find(i, end, '}');
			if (j == end) {
				auto errc = substitute_params_errc::too_many_opening_braces;
				throw substitute_params_error{errc, std::prev(i)};
			}
			
			// e.g.
			//                       i  j
			//                       |  |
			// "{Lorem} ipsum dolor {sit} amet"
			
			if (!is_param_name(i, j)) {
				auto errc = substitute_params_errc::invalid_parameter_name;
				throw substitute_params_error{errc, i, j};
			}
			
			auto distance = std::distance(i, j);
			auto length = static_cast<std::string_view::size_type>(distance);
			std::string_view name{i, length};
			
			auto param_iter = params.find(name);
			if (param_iter == params.end()) {
				auto errc = substitute_params_errc::missing_parameter;
				throw substitute_params_error{errc, i, j};
			}
			
			std::string_view val = (*param_iter).second;
			output += val;
		}
	}
	
	inline std::string substitute_params(std::string_view input,
								         TextParams const& params)
	{
		return substitute_params(input.begin(), input.end(), params);
	}

	// A string coupled with a suggested style.
	//
	// A style is intended to entail such formatting properties as typeface,
	// font size, colour, etc.
	//
	// The presence of styles can be queried by using bitwise operations,
	// e.g. `(style & StyledString::italic_mask).any()`,
	// `(style ^ StyledString::title_mask).none()`.
	struct StyledString {
		using Style = std::bitset<4>;
		
		static constexpr Style title_mask     = 0b1000;
		static constexpr Style italic_mask    = 0b0100;
		static constexpr Style help_text_mask = 0b0010;
		static constexpr Style monospace_mask = 0b0001;

		std::string string;
		Style style;

		StyledString() = default;

		StyledString(std::string string, Style style)
		: string{string}, style{style}
		{ }
	};

	// A vector of styled strings, used to form a block of text.
	using StyledText = std::vector<StyledString>;

	// Error code for exceptions that can be thrown when calling
	// `format_text`.
	enum class format_text_errc {
		dangling_backslash,
		invalid_escape_sequence
	};

	// Type for exceptions that can be thrown when calling `format_text`.
	struct format_text_error {
		// Error code for this exception.
		format_text_errc errc;
		
		// Position in input string where error occurred.
		std::string_view::iterator i;
		
		format_text_error(format_text_errc errc, std::string_view::iterator i)
		: errc{errc}, i{i}
		{ }
	};

	namespace _impl {
		// Parse the escape sequence starting at `i` and terminating no later
		// than `end`. The escape sequence is expected to begin with a
		// backslash.
		//
		// @returns The character represented by the escape sequence,
		// and the iterator one-past-the-end of the escape sequence.
		//
		// @throws `format_text_error` if the range `{i,end}` does not contain
		// a subsequence `{i,j}` that represents an escape sequence.
		//
		// @pre `*i` is a backslash.
		template <typename ForwardIter>
		inline auto	read_escape_sequence(ForwardIter i, ForwardIter end)
		-> std::pair<char, ForwardIter>
		{
			auto j = std::next(i);
			if (j == end) {
				auto errc = format_text_errc::dangling_backslash;
				throw format_text_error{errc, i};
			}
			
			char ch = *j;
			if (!is_escapable_char(ch)) {
				auto errc = format_text_errc::invalid_escape_sequence;
				throw format_text_error{errc, i};
			}
			
			return {ch, std::next(j)};
		}
		
		inline void move_block(StyledText & text,
							   std::string & block,
							   StyledString::Style style)
		{
			if (!block.empty()) {
				text.emplace_back(block, style);
				block.clear();
			}
		}
		
		// Toggle one of the bits in `style` on or off based on the character
		// passed in.
		inline void update_style(StyledString::Style & style, char bit_name)
		{
			switch (bit_name) {
				case '=':
					style ^= StyledString::title_mask;
					break;
					
				case '_':
					style ^= StyledString::italic_mask;
					break;
					
				case '%':
					style ^= StyledString::help_text_mask;
					break;
					
				case '@':
					style ^= StyledString::monospace_mask;
					break;
				
				default:
					break;
			}
		}
	}

	// Parse the string contained in the range `{begin,end}` to find all of
	// its special characters and perform a sequence of actions as described
	// below.
	//
	// The general idea is that blocks of text will be extracted from
	// `{begin,end}` and added to the output.
	//
	// A set of formatting codes can be used to change the value of
	// `current_style`:
	//   = -- Toggle the "title" bit.
	//   _ -- Toggle the "italic" bit.
	//   % -- Toggle the "help_text" bit.
	//   @ -- Toggle the "monospace" bit.
	//
	// The default value of `current_style` is to have all bits turned off.
	//
	// Each time a formatting code is found, a new block of text is added to
	// the output, using the value of `current_style` just before the
	// formatting code was applied. Its content is taken to be the string
	// between the previous formatting code and the new formatting code.
	//
	// Certain escape sequences are also recognised:
	//   \\ -- Print a single '\' character.
	//   \{ -- Print a single '{' character.
	//   \} -- Print a single '}' character.
	//   \= -- Print a single '=' character.
	//   \_ -- Print a single '_' character.
	//   \% -- Print a single '%' character.
	//   \@ -- Print a single '@' character.
	//
	// @pre `{begin,end}` is a valid range.
	//
	// @throws `format_error` with code `dangling_backslash` if there's an
	// unescaped '\' character at the end of the input string.
	//
	// @throws `format_error` with code `invalid_escape_sequence` if any
	// escape sequence "\x" is encountered that is not in the list above.
	template <typename ForwardIter>
	StyledText format_text(ForwardIter begin, ForwardIter end,
					       StyledString::Style current_style = {})
	{
		auto is_special_char = [](char ch) {
			return _impl::is_formatting_char(ch) || ch == '\\';
		};
		
		std::string block;
		StyledText output;

		for (auto i = begin; /**/; ) {
			auto j = std::find_if(i, end, is_special_char);
			
			// e.g.
			//       i             j
			//       |             |
			// "... %You can enter @help@ if you need..."
			
			// e.g.
			//          i            j
			//          |            |
			// "... @ok@ to continue."
			
			block.append(i, j);
			if (j == end) {
				_impl::move_block(output, block, current_style);
				return output;
			}
			
			if (char ch = *j; _impl::is_formatting_char(ch)) {
				_impl::move_block(output, block, current_style);
				_impl::update_style(current_style, ch);
				i = std::next(j);
			} else { // ch is a backslash
				auto [esc, next] = _impl::read_escape_sequence(j, end);
				block += esc;
				i = next;
			}
		}
	}

	inline StyledText format_text(std::string_view input,
							      StyledString::Style current_style = {})
	{
		return format_text(input.begin(), input.end(), current_style);
	}
}

#endif
