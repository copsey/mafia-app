#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace maf {
	// Create a new string, identical to `str` but with each tag character
	// '^' replaced with '^^'.
	std::string escape_tags(std::string_view str);
	
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
				case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
				case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
				case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
				case 'y': case 'z': case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
				case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
				case 'W': case 'X': case 'Y': case 'Z': case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7': case '8': case '9': case '_': case '.':
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
			game,    // Default style during game.
			help,    // Game set-up and general help.
			italic,  // Italic modifier on current style. Used for descriptive text.
			command, // Commands that can be inputted into the console.
			title    // Title of section. A maximum of one title can occur
			         // per message, as the first item.
		};

		std::string string{};
		Style style{Style::game};

		Styled_string() = default;

		Styled_string(std::string string, Style style)
			: string{string}, style{style}
		{ }
	};

	// A vector of styled strings, used to form a block of text.
	using Styled_text = std::vector<Styled_string>;
	
	// A map from strings to strings, used as substitutions (e.g. "{exampleStr}")
	// when creating a block of text.
	using TextParams = std::map<std::string, std::string>;

	// Often, styled text is created from what is called a tagged string: this is
	// simply a std::string containing formatting codes of the form ^x.
	// The following tags are possible:
	//    ^g = game
	//    ^h = help
	//    ^i = italic
	//    ^c = command
	//    ^T = title
	// ^/ closes the current tag, so that what follows uses the previous tag.
	// (Note that the maximum-supported tag depth is 9, including the default.)
	//
	// ^^ prints a single '^' character. Similarly, ^{ prints a single '{' character,
	// and ^} prints a single '}' character.
	//
	// The appearance of any other two-character substring of the form ^x results in
	// an exception.
	//
	// Note that the default string style is Style::game, and hence game-styled
	// strings need not be prepended with ^g.

	/// Find all strings of the form "{substitute}" in `str_with_params`, and replace them
	/// with the corresponding value from `params`.
	///
	/// Braces preceded by carets "^" are considered escaped and are ignored.
	///
	/// @example If `params` is `{ {"word1", "NEW"}, {"word2", "STRING"} }`, then
	///     "This is my {word1} {word2} with ^{braces^}."
	/// will be transformed into
	///     "This is my NEW STRING with ^{braces^}."
	///
	/// @throws `std::invalid_argument` if an invalid parameter name is encountered.
	/// (See `is_param_name` for more information.)
	/// @throws `std::invalid_argument` for any parameter names that cannot be
	/// found in `params`.
	/// @throws `std::invalid_argument` if there are different numbers of (unescaped)
	/// opening braces "{" and closing braces "}".
	std::string substitute_params(std::string_view str_with_params, TextParams const& params);

	// Convert a tagged string into styled text, using `params` as a dictionary of
	// text replacements.
	Styled_text styled_text_from(std::string_view tagged_str,
	                             TextParams const& params = {});
}

#endif
