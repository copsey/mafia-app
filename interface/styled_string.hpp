#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <algorithm>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace maf {
	// Create a new string based on the range of characters in {begin, end},
	// but with the following substitutions:
	//   - Each '^' character is replaced with "^^"
	//   - Each '{' character is replaced with "^{"
	//   - Each '}' character is replaced with "^}"
	//
	// @pre {begin, end} is a valid range.
	template <typename ForwardIterator>
	std::string escaped(ForwardIterator begin, ForwardIterator end)
	{
		auto needs_escaping = [](char ch) {
			return ch == '^' || ch == '{' || ch == '}';
		};

		std::string output;

		for (auto i = begin; ; ) {
			auto j = std::find_if(i, end, needs_escaping);
			output.append(i, j);
			if (j == end) return output;

			output += {'^', *j};
			i = j; ++i;
		}
	}

	// Create a new string, identical to `str` but with the following
	// substitutions:
	//   - Each '^' character is replaced with "^^"
	//   - Each '{' character is replaced with "^{"
	//   - Each '}' character is replaced with "^}"
	inline std::string escaped(std::string_view str_view)
	{
		return escaped(str_view.begin(), str_view.end());
	}
	
	// Check if the range of characters in {begin, end} gives an allowed name
	// for a styled-text parameter. This is true if it matches the regex
	// /^[a-zA-Z0-9_\.]+$/.
	//
	// @pre {begin, end} is a valid range.
	template <typename InputIterator>
	bool is_param_name(InputIterator begin, InputIterator end)
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

	// A string coupled with a suggested style.
	// A style is intended to entail such formatting properties as typeface,
	// font size, colour, etc.
	struct Styled_string {
		enum class Style {
			game,    // Default style.
			help,    // Game setup and general help.
			italic,  // Italic modifier on current style. Used for descriptive
			         // text.
			command, // Commands that can be inputted into the console.
			title    // Title of section. A maximum of one title should occur
			         // per message, as the first item.
		};

		std::string string;
		Style style{Style::game};

		Styled_string() = default;

		Styled_string(std::string string, Style style)
		: string{string}, style{style}
		{ }
	};

	// Convert the character 'x' at the end of a tag "^x" into its
	// corresponding style.
	//
	// The following characters are recognised:
	//   'g' -> `Style::game`
	//   'h' -> `Style::help`
	//   'i' -> `Style::italic`
	//   'c' -> `Style::command`
	//   'T' -> `Style::title`
	//
	// If `ch` is not in the list above, return `Styled_string::Style::game`.
	inline Styled_string::Style get_style(char ch)
	{
		switch (ch) {
			case 'g':  return Styled_string::Style::game;
			case 'h':  return Styled_string::Style::help;
			case 'i':  return Styled_string::Style::italic;
			case 'c':  return Styled_string::Style::command;
			case 'T':  return Styled_string::Style::title;
			default:   return Styled_string::Style::game;
		}
	}

	// A vector of styled strings, used to form a block of text.
	using Styled_text = std::vector<Styled_string>;
	
	// A map from strings (treated as parameter names) to strings (treated as
	// substitutions).
	//
	// Note that the map only holds views into its keys, whereas the values
	// are fully owned. Typically, the parameter name will be a compile-time
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
	// Braces preceded by carets "^" are considered escaped and are ignored.
	//
	// @example If `params` is `{ {"word1", "NEW"}, {"word2", "STRING"} }`,
	// then
	//     "This is my {word1} {word2} with ^{braces^}."
	// will be transformed into
	//     "This is my NEW STRING with ^{braces^}."
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
	template <typename BidirectionalIterator>
	std::string substitute_params(BidirectionalIterator begin,
								  BidirectionalIterator end,
								  TextParams const& params)
	{
		auto is_brace = [](char ch) { return ch == '{' || ch == '}'; };
		
		std::string output;
		
		for (auto i = begin; ; ) {
			auto j = std::find_if(i, end, is_brace);
			output.append(i, j);
			if (j == end) return output;
			
			// e.g. 1
			//         i            j
			//         |            |
			// "{Lorem} ipsum dolor {sit} amet"
			
			// e.g. 2
			//          i   j
			//          |   |
			// "Nulla ^{sit^} amet"
			
			if (j != begin && *std::prev(j) == '^') {
				output += *j;
				i = j; ++i;
				continue;
			}
			
			if (*j == '}') {
				auto errc = substitute_params_errc::too_many_closing_braces;
				throw substitute_params_error{errc, j};
			}
			
			i = j; ++i;
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
			
			i = j; ++i;
		}
	}
	
	inline std::string substitute_params(std::string_view input,
								         TextParams const& params)
	{
		return substitute_params(input.begin(), input.end(), params);
	}

	// Error code for exceptions that can be thrown when calling `apply_tags`.
	enum class apply_tags_errc {
		dangling_caret,
		invalid_tag,
		too_many_styles,
		extra_closing_tag
	};

	// Type for exceptions that can be thrown when calling `apply_tags`.
	struct apply_tags_error {
		// Error code for this exception.
		apply_tags_errc errc;
		
		// Position in input string where error occurred.
		std::string_view::iterator i;
		
		apply_tags_error(apply_tags_errc errc, std::string_view::iterator i)
		: errc{errc}, i{i}
		{ }
	};

	// Often, styled text is created from what is called a _tagged string_:
	// this is simply a `std::string` containing substrings of the form "^x",
	// which are referred to as _tags_.
	//
	// This function parses a given string to find all of its tags and perform
	// a sequence of actions on the string as described below.
	//
	// The following tags are possible:
	//   ^g - Start a new block of text using the `game` style.
	//   ^h - Start a new block of text using the `help` style.
	//   ^i - Start a new block of text using the `italic` style.
	//   ^c - Start a new block of text using the `command` style.
	//   ^T - Start a new block of text using the `title` style.
	//   ^/ - Close the current block of text, and start a new block of text
	//        using the previous style.
	//   ^^ - Print a single '^' character.
	//   ^{ - Print a single '{' character.
	//   ^} - Print a single '}' character.
	//
	// Whenever a new block of text is created, the previous style is pushed
	// onto a stack that can hold a maximum of 8 styles. The default style is
	// `game`, so that the input string doesn't need to begin with "^g".
	//
	// @throws `apply_tags_error` with code `dangling_caret` if there's an
	// unescaped '^' character at the end of the input string.
	//
	// @throws `apply_tags_error` with code `invalid_tag` if any tag "^x" is
	// encountered that is not in the list above.
	//
	// @throws `apply_tags_error` with code `too_many_styles` if the maximum
	// depth of the style stack is exceeded.
	//
	// @throws `apply_tags_error` with code `extra_closing_tag` if there's a
	// "^/" tag without a corresponding block of text.
	Styled_text apply_tags(std::string_view input);
}

#endif
