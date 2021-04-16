#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <algorithm>
#include <bitset>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace maf {
	namespace _impl {
		// Check if `ch` signifies a formatting code as defined by
		// `format_text`.
		inline bool is_formatting_char(char ch)
		{
			return ch == '=' || ch == '_' || ch == '%' || ch == '@';
		}

		// Check if `ch` forms an escape sequence by prefixing it with a
		// backslash '\'.
		inline bool is_escapable_char(char ch)
		{
			return (ch == '{' || ch == '}' || ch == '\\'
					|| is_formatting_char(ch));
		}
	}

	// Create a new string based on the range of characters in `{begin,end}`,
	// but with each escapable character prefixed by a backslash '\'.
	//
	// @pre `{begin,end}` is a valid range.
	template <typename Iterator>
	std::string escaped(Iterator begin, Iterator end)
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

	// A parameter used when preprocessing text.
	using TextParam = std::variant<std::string, bool>;

	// A map from strings (treated as parameter names) to text parameters
	// (treated as substitutions).
	//
	// Note that the map only holds views into its keys, whereas the values
	// are fully owned. Typically the parameter name will be a compile-time
	// constant.
	using TextParams = std::map<std::string_view, TextParam>;
	
	// Check if the range of characters in `{i, j}` gives an allowed name
	// for a text parameter. This is true if it matches the regex
	// `/^[a-zA-Z0-9_\.]+$/`.
	//
	// @pre `{i, j}` is a valid range.
	template <typename Iterator>
	bool is_param_name(Iterator i, Iterator j)
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
	
	// Check if `param` is an allowed name for a text parameter.
	// This is true if it matches the regex `/^[a-zA-Z0-9_\.]+$/`.
	inline bool is_param_name(std::string_view param)
	{
		return is_param_name(param.begin(), param.end());
	}

	// Error code for exceptions that can be thrown when calling
	// `preprocess_text`.
	enum class preprocess_text_errc {
		empty_directive,
		invalid_parameter_name,
		missing_command_name,
		missing_parameter_name,
		parameter_not_found,
		too_many_closing_braces,
		too_many_opening_braces,
		too_many_parameter_names,
		unclosed_expression,
		unexpected_command,
		wrong_parameter_type
	};

	// Type for exceptions that can be thrown when calling `preprocess_text`.
	struct preprocess_text_error {
		// Error code for this exception.
		preprocess_text_errc errc;

		// Position in input string where the error occurred.
		std::string_view::iterator i;
		
		// (Optional) iterator that forms a range `{i, j}` when paired with
		// `i`.
		//
		// Only used by the error codes `invalid_parameter_name` and
		// `missing_parameter` to signify where in the input string the
		// parameter name occurs.
		std::string_view::iterator j;

		preprocess_text_error(preprocess_text_errc errc,
							  std::string_view::iterator i)
		: errc{errc}, i{i}, j{}
		{ }
		
		preprocess_text_error(preprocess_text_errc errc,
							  std::string_view::iterator i,
							  std::string_view::iterator j)
		: errc{errc}, i{i}, j{j}
		{ }
	};

	// TODO: Finish writing this description.
	//
	// Find all strings of the form "{substitute}" in the range `{begin,end}`,
	// and replace them with the corresponding value from `params`.
	//
	// Escape sequences of the form "\x" are left intact. In particular,
	// braces preceded by backslashes '\' are not treated as parts of command
	// names, unless the backslash itself is escaped, as in "\\".
	//
	// @example If `params` is `{ {"word1", "NEW"}, {"word2", "STRING"} }`,
	// then
	//     "This is my {word1} {word2} with \{braces\}."
	// will be transformed into
	//     "This is my NEW STRING with \{braces\}."
	//
	// @pre `{begin,end}` is a valid range.
	//
	// @throws `preprocess_text_error` with code `invalid_parameter_name` if
	// an invalid parameter name is encountered. (See `is_param_name` for more
	// information.)
	//
	// @throws `preprocess_text_error` with code `missing_parameter` for any
	// parameter names that cannot be found in `params`.
	//
	// @throws `preprocess_text_error` with code `too_many_opening_braces`
	// or `too_many_closing_braces` if there are different numbers of
	// (unescaped) opening braces '{' and closing braces '}'.
	template <typename Iterator>
	void preprocess_text(Iterator begin, Iterator end, TextParams const& params, std::string & output);
	
	inline std::string preprocess_text(std::string_view input, TextParams const& params)
	{
		std::string output;
		preprocess_text(input.begin(), input.end(), params, output);
		return output;
	}

	namespace _impl {
		inline bool is_space(char ch) { return ch == ' '; }
		
		inline bool is_delimiter_for_directives(char ch) {
			return ch == '{' || ch == '}' || ch == ' ';
		}
		
		enum class directive_type {
			substitution,
			if_directive,
			else_if_directive,
			else_directive,
			end_directive
		};
	
		// Get the parameter from `params` whose name is equal to the range
		// of characters `{i, j}`, and ensure that its type is `ParamType`.
		//
		// @example Given the following range:
		// ```
		//             i  j
		//             |  |
		// "... dolor {sit} amet ..."
		// ```
		// return `params["sit"]`.
		//
		// @pre `{i, j}` is a valid range.
		//
		// @throws `preprocess_text_error` with code `missing_parameter_name`
		// if `{i, j}` is an empty range.
		//
		// @throws `preprocess_text_error` with code `invalid_parameter_name`
		// if the range of characters `{i, j}` is not an allowed parameter
		// name. (See `is_param_name` for more information.)
		//
		// @throws `preprocess_text_error` with code `parameter_not_found` if
		// there is no parameter in `params` whose name is equal to the range
		// of characters `{i, j}`.
		//
		// @throws `preprocess_text_error` with code `wrong_parameter_type` if
		// a parameter was found, but its type is not `ParamType`.
		template <typename ParamType>
		auto get_param(std::string_view name, TextParams const& params)
		-> ParamType const&
		{
			auto iter = params.find(name);
			if (iter == params.end()) {
				auto errc = preprocess_text_errc::parameter_not_found;
				throw preprocess_text_error{errc, name.begin(), name.end()};
			}
			
			auto & param = (*iter).second;
			
			if (auto ptr = std::get_if<ParamType>(&param)) {
				return *ptr;
			} else {
				auto errc = preprocess_text_errc::wrong_parameter_type;
				throw preprocess_text_error{errc, name.begin(), name.end()};
			}
		}
	
		// Find the position of the next closing brace '}' in the range
		// `{i, end}`.
		//
		// @pre `{i, end}` is a valid range.
		//
		// @throws `preprocess_text_error` with code `too_many_opening_braces`
		// if no closing brace could be found.
		template <typename Iterator>
		Iterator find_closing_brace(Iterator i, Iterator end)
		{
			auto j = std::find(i, end, '}');
			if (j == end) {
				auto errc = preprocess_text_errc::too_many_opening_braces;
				throw preprocess_text_error{errc, i};
			}
			return j;
		}
		
		template <typename Iterator>
		auto parse_param_name(Iterator begin, Iterator end)
		-> std::pair<Iterator, std::string_view>
		{
			auto next = std::find_if(begin, end, is_delimiter_for_directives);
			if (next == begin) {
				auto errc = preprocess_text_errc::missing_parameter_name;
				throw preprocess_text_error{errc, begin};
			}
			
			if (!is_param_name(begin, next)) {
				auto errc = preprocess_text_errc::invalid_parameter_name;
				throw preprocess_text_error{errc, begin, next};
			}
			
			auto distance = std::distance(begin, next);
			auto length = static_cast<std::string_view::size_type>(distance);
			auto name = std::string_view{begin, length};
			
			return {next, name};
		}
	
		// Get the command name at the start of the range of characters
		// `{begin, end}`.
		//
		// @example Given the following range:
		// ```
		//            begin               end
		//            |                   |
		//     "Lorem {!else_if ipsum} ..."
		// ```
		// return a string view equal to `"else_if"`.
		//
		// @pre `{begin, end}` is a valid range.
		// @pre `begin` points to the beginning of a directive "{!...".
		//
		// @throws `preprocess_text_error` with error code
		// `missing_command_name` if there is no command in the range
		// `{begin, end}`.
		template <typename Iterator>
		std::string_view get_command_name(Iterator begin, Iterator end)
		{
			auto i = std::next(begin, 2); // skip the "{!" part
			i = std::find_if_not(i, end, is_space);
			auto j = std::find_if(i, end, is_delimiter_for_directives);
			
			if (i == j) {
				auto errc = preprocess_text_errc::missing_command_name;
				throw preprocess_text_error{errc, begin};
			}
			
			auto distance = std::distance(i, j);
			auto length = static_cast<std::string_view::size_type>(distance);
			auto name = std::string_view{i, length};
			
			return name;
		}
	
		template <typename Iterator>
		Iterator scan_plain_text(Iterator begin, Iterator end)
		{
			auto is_special_char = [](char ch) {
				return ch == '{' || ch == '}' || ch == '\\';
			};
			
			return std::find_if(begin, end, is_special_char);
		}
	
		// Form the largest possible subrange `{begin, i}` of `{begin, end}`
		// where none of the characters are handled in a special way by
		// `preprocess_text`. Append this range to `output`.
		//
		// Special case: if the plain text ends in "...\n" and the next
		// block is a directive "{!...}", then do NOT append the final newline
		// character "\n" to `output`.
		//
		// @returns An iterator pointing to the next special character, or
		// `end` if none were found.
		//
		// @example Given the following input:
		// ```
		//  begin            end
		//  |                |
		// " et \= magnis ..."
		//      |
		//      i
		// ```
		// append `" et "` to `output` and return `i`.
		//
		// @example Given the following input:
		// ```
		//         begin         end
		//         |             |
		// "{Lorem} ipsum \n{!if dolor}"
		//                  |
		//                  i
		// ```
		// append `" ipsum "` to `output` without the newline, and return `i`.
		//
		// @pre `{begin, end}` is a valid range.
		template <typename Iterator>
		Iterator append_plain_text(Iterator begin, Iterator end, std::string & output)
		{
			auto i = _impl::scan_plain_text(begin, end);
			
			if (i != begin && *std::prev(i) == '\n'
				&& i != end && *i == '{'
				&& std::next(i) != end && *std::next(i) == '!') {
				output.append(begin, std::prev(i));
			} else {
				output.append(begin, i);
			}
			
			return i;
		}
	
		// @example Given the following input:
		// ```
		//          begin               end
		//          |                   |
		// "If that {staid} old house..."
		//                 |
		//                 next
		// ```
		// return `{next, "staid"}`.
		//
		// @pre `{begin, end}` is a valid range.
		//
		// @pre `begin` points to the start of a directive. The syntax inside
		// the directive need not be correct, as long as it starts with an
		// opening brace, "{...".
		template <typename Iterator>
		auto parse_substitution(Iterator begin, Iterator end)
		-> std::pair<Iterator, std::string_view>
		{
			using std::literals::string_view_literals::operator""sv;
			
			auto i = begin;
			i = std::next(i, "{"sv.size());
			i = std::find_if_not(i, end, is_space);
			
			i = std::find_if_not(i, end, is_space);
			auto [next, name] = _impl::parse_param_name(i, end);
			i = next;
			
			i = std::find_if_not(i, end, is_space);
			if (i == end || *i == '{') {
				auto errc = preprocess_text_errc::too_many_opening_braces;
				throw preprocess_text_error{errc, begin};
			} else if (*i != '}') {
				auto errc = preprocess_text_errc::too_many_parameter_names;
				throw preprocess_text_error{errc, begin};
			}
			
			++i; return {i, name};
		}
		
		// Parse the text inside an "{!if ...}" directive.
		//
		// @returns the name of the parameter inside the "if" directive, and
		// the next iterator after the directive terminates.
		//
		// @example Given the following input:
		// ```
		//        begin                end
		//        |                    |
		// "Donec {!if quam} felis, ..."
		//                  |
		//                  next
		// ```
		// return `{"quam", next}`.
		//
		// @pre `{begin, end}` is a valid range.
		//
		// @pre `begin` points to the start of an "{!if ..." directive. The
		// syntax inside the directive need not be correct, only the "{!"
		// and "if" parts.
		//
		// @throws `preprocess_text_error` with code `too_many_opening_braces`
		// if there's no closing brace '}', or an opening brace '{' is
		// encountered before a closing brace.
		//
		// @throws ...
		template <typename Iterator>
		auto parse_if_directive(Iterator begin, Iterator end)
		-> std::pair<Iterator, std::string_view>
		{
			using std::literals::string_view_literals::operator""sv;
			
			auto i = begin;
			i = std::next(i, "{!"sv.size());
			i = std::find_if_not(i, end, is_space);
			i = std::next(i, "if"sv.size());
			
			i = std::find_if_not(i, end, is_space);
			auto [next, name] = parse_param_name(i, end);
			i = next;
			
			i = std::find_if_not(i, end, is_space);
			if (i == end || *i == '{') {
				auto errc = preprocess_text_errc::too_many_opening_braces;
				throw preprocess_text_error{errc, begin};
			} else if (*i != '}') {
				auto errc = preprocess_text_errc::too_many_parameter_names;
				throw preprocess_text_error{errc, begin};
			}
			
			++i; return {i, name};
		}
		
		template <typename Iterator>
		auto parse_else_if_directive(Iterator begin, Iterator end)
		-> std::pair<Iterator, std::string_view>
		{
			using std::literals::string_view_literals::operator""sv;
			
			auto i = begin;
			i = std::next(i, "{!"sv.size());
			i = std::find_if_not(i, end, is_space);
			i = std::next(i, "else_if"sv.size());
			
			i = std::find_if_not(i, end, is_space);
			auto [next, name] = parse_param_name(i, end);
			i = next;
			
			i = std::find_if_not(i, end, is_space);
			if (i == end || *i == '{') {
				auto errc = preprocess_text_errc::too_many_opening_braces;
				throw preprocess_text_error{errc, begin};
			} else if (*i != '}') {
				auto errc = preprocess_text_errc::too_many_parameter_names;
				throw preprocess_text_error{errc, begin};
			}
			
			++i; return {i, name};
		}
	
		template <typename Iterator>
		Iterator parse_else_directive(Iterator begin, Iterator end)
		{
			using std::literals::string_view_literals::operator""sv;
			
			auto i = begin;
			i = std::next(i, "{!"sv.size());
			i = std::find_if_not(i, end, is_space);
			i = std::next(i, "else"sv.size());
			
			i = std::find_if_not(i, end, is_space);
			if (i == end || *i == '{') {
				auto errc = preprocess_text_errc::too_many_opening_braces;
				throw preprocess_text_error{errc, begin};
			} else if (*i != '}') {
				auto errc = preprocess_text_errc::too_many_parameter_names;
				throw preprocess_text_error{errc, begin};
			}
			
			++i; return i;
		}
	
		// @example Given the following input:
		// ```
		//               begin                   end
		//               |                       |
		// "Ours was the {!end} marsh country ..."
		//                     |
		//                     next
		// ```
		// return `next`.
		//
		// @pre `{begin, end}` is a valid range.
		// @pre `begin` points to an opening brace '{'.
		template <typename Iterator>
		Iterator parse_end_directive(Iterator begin, Iterator end)
		{
			using std::literals::string_view_literals::operator""sv;
			
			auto i = begin;
			i = std::next(i, "{!"sv.size());
			i = std::find_if_not(i, end, is_space);
			i = std::next(i, "end"sv.size());
			
			i = std::find_if_not(i, end, is_space);
			if (i == end || *i == '{') {
				auto errc = preprocess_text_errc::too_many_opening_braces;
				throw preprocess_text_error{errc, begin};
			} else if (*i != '}') {
				auto errc = preprocess_text_errc::too_many_parameter_names;
				throw preprocess_text_error{errc, begin};
			}
			
			++i; return i;
		}
		
		// Parse the given conditional expression, appending text to `output`
		// as appropriate.
		//
		// @returns the next iterator after the conditional expression
		// terminates, and the block to substitute into the output text, if
		// applicable.
		//
		// @example Given the following input:
		// ```
		//        begin                           end
		//        |                               |
		// "Lorem {!if ipsum} dolor sit{!end} amet"
		//                                   |
		//                                   next
		// ```
		// append `" dolor sit"` if `params["ipsum"]` is true, and return
		// `next` in either case.
		//
		// @pre `{begin, end}` is a valid range.
		//
		// @pre `begin` points to the start of an "{!if ..." directive. The
		// syntax inside the directive need not be correct, only the "{!if "
		// part.
		//
		// @throws `preprocess_text_error` with code `unclosed_expression`
		// if there is no "{!end}" directive to match the initial "{!if ...}"
		// directive.
		//
		// @throws the same errors as `parse_if_directive`,
		// `parse_else_directive`, `parse_else_if_directive` and
		// `parse_end_directive` in addition to the above. (See these
		// functions for more information.)
		template <typename Iterator>
		Iterator parse_conditional_expression(Iterator begin, Iterator end, TextParams const& params, std::string & output)
		{
//			int stage = 0; // 0 = allows an "if" directive
			               // 1 = allows an "else_if" or "else" directive
			               // 2 = allows an "end" directive
			
			std::optional<Iterator> subexpr_begin;
			std::optional<Iterator> subexpr_end;
			
			auto i = begin;
			
			{ // Parse the initial "{!if ...}" directive.
				auto [next, name] = _impl::parse_if_directive(i, end);
				bool param = _impl::get_param<bool>(name, params);
				if (param) subexpr_begin = next;
				i = next;
			}
			
			for (int depth = 1; depth > 0; ) {
				i = std::find(i, end, '{');
				if (i == end) {
					auto errc = preprocess_text_errc::unclosed_expression;
					throw preprocess_text_error{errc, begin};
				}
				
				if (std::next(i) != end && *std::next(i) == '!') {
					auto command = _impl::get_command_name(i, end);
					
					if (command == "if") {
						auto [next, name] = _impl::parse_if_directive(i, end);
						
						i = next;
						++depth;
					} else if (command == "else_if") {
						auto [next, name] = _impl::parse_else_if_directive(i, end);
						
						if (!subexpr_begin && depth == 1) {
							bool param = _impl::get_param<bool>(name, params);
							if (param) subexpr_begin = next;
						} else if (!subexpr_end && depth == 1) {
							subexpr_end = i;
						}
						
						i = next;
					} else if (command == "else") {
						auto next = _impl::parse_else_directive(i, end);
						
						if (!subexpr_begin && depth == 1) {
							subexpr_begin = next;
						} else if (!subexpr_end && depth == 1) {
							subexpr_end = i;
						}
						
						i = next;
					} else if (command == "end") {
						auto next = _impl::parse_end_directive(i, end);
						
						if (subexpr_begin && !subexpr_end && depth == 1) {
							subexpr_end = i;
						}
						
						i = next;
						--depth;
					} else {
						auto errc = preprocess_text_errc::unexpected_command;
						throw preprocess_text_error{errc, command.begin(), command.end()};
					}
				} else {
					++i;
				}
			}
			
			if (subexpr_begin && subexpr_end) {
				preprocess_text(*subexpr_begin, *subexpr_end, params, output);
			}
			
			return i;
		}
	
		// Parse an escape sequence in the range `{i, j}` starting from `i`.
		// Append the sequence to `output.`
		//
		// @returns An iterator just after the escape sequence ends.
		//
		// @example Given the following range:
		// ```
		//            i       j
		//            |       |
		//     "Lorem \{ ipsum ..."
		//              |
		//              k
		// ```
		// append `"\{"` to `output` and return `k`.
		//
		// @example Given the following range:
		// ```
		//        ij
		//        ||
		// "Lorem \"
		// ```
		// append `"\"` to `output` and return `j`.
		//
		// @pre `{i, j}` is a valid range with length 1 or more.
		// @pre `i` points to a backslash '\'.
		template <typename Iterator>
		Iterator append_escape_sequence(Iterator i, Iterator j, std::string & output)
		{
			++i;
			if (i == j) {
				output += '\\';
			} else {
				output += {'\\', *i};
				++i;
			}
			return i;
		}
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


/* Template implementations */

template <typename Iterator>
void maf::preprocess_text(Iterator begin, Iterator end, TextParams const& params, std::string & output)
{
	for (auto i = begin; ; ) {
		i = _impl::append_plain_text(i, end, output);
		if (i == end) return;
		
		if (*i == '\\') {
			i = _impl::append_escape_sequence(i, end, output);
		} else if (*i == '}') {
			auto errc = preprocess_text_errc::too_many_closing_braces;
			throw preprocess_text_error{errc, i};
		} else { // *i == '{'
			auto j = _impl::find_closing_brace(i, end);
			
			if (std::next(i) == j) {
				auto errc = preprocess_text_errc::empty_directive;
				throw preprocess_text_error{errc, i};
			}
			
			if (*std::next(i) == '!') {
				auto command_name = _impl::get_command_name(i, end);
				
				if (command_name == "if") {
					i = _impl::parse_conditional_expression(i, end, params, output);
				} else {
					auto errc = preprocess_text_errc::unexpected_command;
					throw preprocess_text_error{errc, command_name.begin(), command_name.end()};
				}
			} else {
				auto [next, name] = _impl::parse_substitution(i, end);
				auto & param = _impl::get_param<std::string>(name, params);
				output += param;
				i = next;
			}
		}
	}
}

#endif
