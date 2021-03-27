#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace maf {
	// Create a new string, identical to `str` but with the following
	// substitutions:
	//   - Each '^' character is replaced with "^^"
	//   - Each '{' character is replaced with "^{"
	//   - Each '}' character is replaced with "^}"
	std::string escaped(std::string_view str);
	
	// Check if the range of characters in {i,j} gives an allowed name
	// for a styled-text parameter.
	// This is true if it matches the regex /^[a-zA-Z0-9_\.]+$/.
	//
	// @pre {i,j} is a valid range.
	template <typename Iter>
	bool is_param_name(Iter i, Iter j)
	{
		if (i == j) return false;
		
		for (; i != j; ++i) {
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
		
		return true;
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

	// Find all strings of the form "{substitute}" in `str_with_params`, and
	// replace them with the corresponding value from `params`.
	//
	// Braces preceded by carets "^" are considered escaped and are ignored.
	//
	// @example If `params` is `{ {"word1", "NEW"}, {"word2", "STRING"} }`,
	// then
	//     "This is my {word1} {word2} with ^{braces^}."
	// will be transformed into
	//     "This is my NEW STRING with ^{braces^}."
	//
	// @throws `std::invalid_argument` if an invalid parameter name is
	// encountered. (See `is_param_name` for more information.)
	// @throws `std::invalid_argument` for any parameter names that cannot be
	// found in `params`.
	// @throws `std::invalid_argument` if there are different numbers of
	// (unescaped) opening braces "{" and closing braces "}".
	std::string substitute_params(std::string_view str_with_params,
								  TextParams const& params);

	// Often, styled text is created from what is called a _tagged string_:
	// this is simply a `std::string` containing substrings of the form "^x",
	// which are referred to as tags.
	//
	// This function parses a given string to find all of its tags and perform
	// some actions on the string as described below.
	//
	// The following tags are possible:
	//   ^g - Create a new block of text using the style returned by
	//        `get_style(x)`, where `x` is the character following '^' for
	//        this tag. The previous style is pushed onto a stack, which can
	//        hold a maximum of 8 styles.
	//   ^h - See above.
	//   ^i - See above.
	//   ^c - See above.
	//   ^T - See above.
	//   ^/ - Close the current block of text, so that what follows uses the
	//        previous style.
	//   ^^ - Print a single '^' character.
	//   ^{ - Print a single '{' character.
	//   ^} - Print a single '}' character.
	//
	// The default string style is `Styled_string::Style::game`, which means
	// strings don't need to begin with "^g".
	//
	// @throws `std::invalid_argument` if any tag "^x" is encountered that is
	// not in the list above.
	// @throws `std::invalid_argument` if the maximum depth of the style
	// stack is exceeded.
	Styled_text apply_tags(std::string_view str_with_tags);

	// Convert a tagged string into styled text, using `params` as a
	// dictionary of text replacements. See `substitute_params` and
	// `apply_tags` for more information.
	Styled_text styled_text_from(std::string_view str_with_params_and_tags,
	                             TextParams const& params = {});
}

#endif
