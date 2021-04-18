#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <algorithm>
#include <bitset>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace maf {
	// Create a new string based on `str`, but with each escapable
	// character prefixed by a backslash `\`.
	//
	// # Examples
	// - `escaped("My {string}")` returns `"My \{string\}"`
	// - `escaped("under_score")` returns `"under\_score"`
	inline std::string escaped(std::string_view str_view);

	// A single parameter used when preprocessing text.
	struct TextParam;

	// A map from strings to text parameters.
	//
	// Note that the map only holds views into its keys, whereas the values
	// are fully owned. Typically each key will be a compile-time constant.
	struct TextParams;
	
	// Check if `str` is an allowed name for a text parameter.
	// This is true if it matches the regex `/^[a-zA-Z0-9_\.]+$/`.
	inline bool is_param_name(std::string_view str) {
		for (char ch: str) {
			switch (ch) {
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

		return !str.empty();
	}

	// Error code for exceptions that can be thrown when calling
	// `preprocess_text`.
	enum class preprocess_text_errc {
		invalid_command_name,
		invalid_parameter_name,
		missing_command_name,
		missing_parameter_name,
		parameter_not_found,
		too_many_closing_braces,
		too_many_parameter_names,
		unclosed_directive,
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
	// @throws `preprocess_text_error` with code `unclosed_directive`
	// or `too_many_closing_braces` if there are different numbers of
	// (unescaped) opening braces '{' and closing braces '}'.
	inline std::string preprocess_text(std::string_view input, TextParams const& params);


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
	inline StyledText format_text(std::string_view input, StyledString::Style current_style = {});
}


/*
 * Implementation details
 */

namespace maf::_textparam_impl {
	using Base = std::variant<
		bool,
		int,
		std::string,
		std::vector<TextParams>>;
}

struct maf::TextParam: _textparam_impl::Base {
	using _textparam_impl::Base::variant;
};


namespace maf::_textparams_impl {
	using Base = std::map<std::string_view, TextParam>;
}

struct maf::TextParams: _textparams_impl::Base {
	using _textparams_impl::Base::map;
};


namespace maf::_preprocess_text_impl {
	using iterator = std::string_view::iterator;

	inline bool is_brace(char ch) {
		return ch == '{' || ch == '}';
	}

	// # Example
	// Given the following input:
	// ```
	//        begin  next                end
	//          |    |                     |
	// "easy {grace  } of her attitude, ..."
	// ```
	// return `next`.
	inline iterator find_brace(iterator begin, iterator end) {
		return std::find_if(begin, end, is_brace);
	}

	inline bool is_whitespace(char ch) {
		return ch == ' ';
	}

	// # Example
	// Given the following input:
	// ```
	//      begin  next             end
	//         |   |                  |
	// "Wemmick    explained to me ..."
	// ```
	// return `next`.
	inline iterator skip_whitespace(iterator begin, iterator end) {
		return std::find_if_not(begin, end, is_whitespace);
	}


	struct directive {
		enum class type_t {
			comment,
			substitution,
			if_command,
			else_if_command,
			else_command,
			list_command,
			end_command
		};

		type_t type;
		std::string_view command_name;
		std::string_view param_name;

		static bool is_delimiter(char ch) {
			return is_brace(ch) || is_whitespace(ch);
		}

		// # Example
		// Given the following input:
		// ```
		//    begin  next               end
		//       |   |                    |
		// "The {flag} had been struck ..."
		// ```
		// return `next`.
		static iterator find_delimiter(iterator begin, iterator end) {
			return std::find_if(begin, end, is_delimiter);
		}

		static type_t to_type(std::string_view command_name) {
			if (command_name == "if") {
				return type_t::if_command;
			} else if (command_name == "else_if") {
				return type_t::else_if_command;
			} else if (command_name == "else") {
				return type_t::else_command;
			} else if (command_name == "list") {
				return type_t::list_command;
			} else if (command_name == "end") {
				return type_t::end_command;
			} else {
				auto errc = preprocess_text_errc::invalid_command_name;
				throw preprocess_text_error{errc, command_name.begin(), command_name.end()};
			}
		}

		// Throw `preprocess_text_error` if `this->type` is not `type`.
		void verify(type_t type) {
			if (type != this->type) {
				switch (this->type) {
				case type_t::comment:
					{
						// FIXME: Throw a bespoke exception.
						break;
					}

				case type_t::substitution:
					{
						// FIXME: Throw a bespoke exception.
						break;
					}

				case type_t::if_command:
				case type_t::else_if_command:
				case type_t::else_command:
				case type_t::list_command:
				case type_t::end_command:
					{
						auto errc = preprocess_text_errc::unexpected_command;
						auto name = command_name;
						throw preprocess_text_error{errc, name.begin(), name.end()};
					}
				}
			}
		}

		// # Examples
		//
		// 1. Given the following input:
		// ```
		//      begin  next     end
		//      |      |          |
		// "You {stock} and stone!"
		// ```
		// set `type` to `substitution`, set `param_name` to "stock", and
		// return `next`.
		//
		// 2. Given the following input:
		// ```
		//  begin       next             end
		//  |            |                 |
		// "{!if Estella} looked at her ..."
		// ```
		// set `type` to `if_command`, set `param_name` to "Estella", and
		// return `next`.
		//
		// 3. Given the following input:
		// ```
		//  begin                   end
		//  |                         |
		// "{!else}                   |
		//  so hard and thankless, ..."
		//  |
		//  next
		// ```
		// set `type` to `else_command` and return `next`, skipping the single
		// newline after `"{!else}"`.
		iterator parse(iterator begin, iterator end, std::string_view input);

	private:
		iterator _parse_as_comment(iterator begin, iterator closing_brace);
		iterator _parse_as_substitution(iterator begin, iterator closing_brace);
		iterator _parse_as_command(iterator begin, iterator closing_brace);
	};


	// # Example
	// Given the following input:
	// ```
	//        begin  next                 end
	//           |    |                     |
	// "After a {short} pause of repose, ..."
	// ```
	// set `name` to `"short"` and return `next`.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with error code
	// `missing_parameter_name` if there is no parameter name starting from
	// `begin`.
	// - Throws `preprocess_text_error` with error code
	// `invalid_parameter_name` if a string was found, but it's not a valid
	// parameter name.
	inline iterator parse_param_name(iterator begin, iterator end,
									 std::string_view & name)
	{
		auto next = directive::find_delimiter(begin, end);

		if (next == begin) {
			auto errc = preprocess_text_errc::missing_parameter_name;
			throw preprocess_text_error{errc, begin};
		}

		auto length = static_cast<std::string_view::size_type>(next - begin);
		std::string_view str{begin, length};

		if (!is_param_name(str)) {
			auto errc = preprocess_text_errc::invalid_parameter_name;
			throw preprocess_text_error{errc, begin, next};
		}

		name = str;
		return next;
	}


	// # Example
	// Given the following input:
	// ```
	//          begin  next     end
	//          |      |          |
	// "Lorem {!else_if ipsum} ..."
	// ```
	// set `name` to `"else_if"` and return `next`.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with error code `missing_command_name`
	// if there is no command name starting from `begin`.
	inline iterator parse_command_name(iterator begin, iterator end,
									   std::string_view & name)
	{
		auto next = directive::find_delimiter(begin, end);

		if (next == begin) {
			auto errc = preprocess_text_errc::missing_command_name;
			throw preprocess_text_error{errc, begin};
		}

		auto length = static_cast<std::string_view::size_type>(next - begin);
		name = std::string_view{begin, length};

		return next;
	}


	// # Example
	// Given the following input:
	// ```
	//            begin       end
	//            |           |
	//     "Lorem \{ ipsum ..."
	//              |
	//              next
	// ```
	// append "\{" to `str` and return `next`.
	inline iterator parse_escape_sequence(iterator begin, iterator end,
										  std::string & str)
	{
		// FIXME: Throw an exception if `begin` does not point to a backslash.

		auto i = begin + 1;

		if (i == end) {
			// FIXME: Throw an exception due to partial escape sequence.
		}

		str += {'\\', *i};
		++i; return i;
	}


	struct expression {
		virtual ~expression() = default;
		virtual void write(std::string & output, TextParams const& params) const = 0;
		virtual iterator parse(iterator begin, iterator end, std::string_view input) = 0;
	};


	// Get the parameter from `params` whose key is equal to `name`.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with code `parameter_not_found` if
	// there is no parameter in `params` whose key is equal to `name`.
	inline auto get_param(std::string_view name, TextParams const& params)
	-> TextParam const& {
		auto iter = params.find(name);

		if (iter == params.end()) {
			auto errc = preprocess_text_errc::parameter_not_found;
			throw preprocess_text_error{errc, name.begin(), name.end()};
		}

		return (*iter).second;
	}


	// Get the parameter from `params` whose key is equal to `name`, and
	// ensure that its type is `ParamType`.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with code `parameter_not_found` if
	// there is no parameter in `params` whose key is equal to `name`.
	// - Throws `preprocess_text_error` with code `wrong_parameter_type` if a
	// parameter was found, but its type is not `ParamType`.
	template <typename ParamType>
	auto get_param_as(std::string_view name, TextParams const& params)
	-> ParamType const& {
		auto& param = get_param(name, params);

		if (auto ptr = std::get_if<ParamType>(&param)) {
			return *ptr;
		} else {
			auto errc = preprocess_text_errc::wrong_parameter_type;
			throw preprocess_text_error{errc, name.begin(), name.end()};
		}
	}


	// Visit the parameter from `params` whose key is equal to `name` using the
	// provided function.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with code `parameter_not_found` if
	// there is no parameter in `params` whose key is equal to `name`.
	template <typename Visitor>
	auto visit_param(Visitor && f, std::string_view name, TextParams const& params)
	-> decltype(std::visit(f, TextParam{})) {
		auto& param = get_param(name, params);
		return std::visit(f, param);
	}

	
	struct plain_text: expression {
		std::string str;
		
		static bool is_delimiter(char ch) {
			return ch == '\\' || ch == '{' || ch == '}';
		}
		
		static iterator find_delimiter(iterator begin, iterator end) {
			return std::find_if(begin, end, is_delimiter);
		}
		
		void write(std::string & output, TextParams const& params) const
		override {
			output += str;
		}
		
		// Form the largest possible subrange of `{begin, end}` starting from
		// `begin` where none of the characters are braces. Append the
		// subrange to `this->str`.
		//
		// Escape sequences are left untouched.
		//
		// # Example
		// Given the following input:
		// ```
		//  begin       next       end
		//  |            |           |
		// "Donec \{quam {felis}, ..."
		// ```
		// append `"Donec \{quam "` to `this->str` and return `next`.
		iterator parse(iterator begin, iterator end, std::string_view input) override;
	};
	
	
	struct substitution: expression {
		std::string_view param_name;
		
		void write(std::string & output, TextParams const& params) const
		override {
			auto print = [&](auto && arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, int>) {
					output += std::to_string(arg);
				} else if constexpr (std::is_same_v<T, std::string>) {
					output += arg;
				} else {
					auto errc = preprocess_text_errc::wrong_parameter_type;
					throw preprocess_text_error{errc, param_name.begin(), param_name.end()};
				}
			};

			visit_param(print, param_name, params);
		}
		
		// # Example
		// Given the following input:
		// ```
		//          begin  next       end
		//          |      |            |
		// "If that {staid} old house..."
		// ```
		// set `this->param_name` to `"staid"` and return `next`.
		iterator parse(iterator begin, iterator end, std::string_view input) override;
	};
	
	
	struct sequence: expression {
		std::vector<std::unique_ptr<expression>> subexprs;
		
		void write(std::string & output, TextParams const& params) const
		override {
			for (auto & expr : subexprs) {
				expr->write(output, params);
			}
		}

		iterator parse(iterator begin, iterator end, std::string_view input) override;
	};
	
	
	struct conditional: expression {
		std::vector<std::pair<std::string_view, sequence>> conditional_subexprs;
		std::optional<sequence> default_subexpr;
		
		void write(std::string & output, TextParams const& params) const
		override {
			for (auto& [name, expr]: conditional_subexprs) {
				auto& param = get_param_as<bool>(name, params);
				
				if (param) {
					expr.write(output, params);
					return;
				}
			}
			
			if (default_subexpr) default_subexpr->write(output, params);
		}

		// # Example
		// Given the following input:
		// ```
		//        begin              next  end
		//        |                     |    |
		// "Lorem {!if ipsum} ... {!end} amet"
		// ```
		// append `{"ipsum", /* sequence parsed from "..." */}` to
		// `this->conditional_subexprs`, and return `next`.
		iterator parse(iterator begin, iterator end, std::string_view input) override;
	};


	struct loop: expression {
		std::string_view param_name;
		sequence subexpr;

		void write(std::string & output, TextParams const& params) const
		override {
			auto& v = get_param_as<std::vector<TextParams>>(param_name, params);

			for (auto& subparams: v) {
				subexpr.write(output, subparams);
			}
		}

		// # Example
		// Given the following input:
		// ```
		//       begin                  next           end
		//       |                       |               |
		// "easy {!list grace} ... {!end} of her attitude"
		// ```
		// set `this->param_name` to `"grace"`, set `this->subexpr` to the
		// result of parsing `...` as a sequence, and return `next`.
		iterator parse(iterator begin, iterator end, std::string_view input) override;
	};


	/*
	 * Definitions of "parse" functions
	 */

	inline iterator directive::parse(iterator begin, iterator end, std::string_view input) {
		using std::literals::operator""sv;

		if (char ch = *begin; ch == '}') {
			auto errc = preprocess_text_errc::too_many_closing_braces;
			throw preprocess_text_error{errc, begin};
		} else if (ch != '{') {
			// FIXME: Throw exception due to invalid syntax.
		}

		auto i = begin + "{"sv.size();
		auto directive_end = find_brace(i, end);

		if (directive_end == end || *directive_end != '}') {
			auto errc = preprocess_text_errc::unclosed_directive;
			throw preprocess_text_error{errc, begin};
		}

		if (char ch = *i; ch == '!') {
			i = _parse_as_command(begin, directive_end);
		} else if (ch == '-') {
			i = _parse_as_comment(begin, directive_end);
		} else {
			i = _parse_as_substitution(begin, directive_end);
		}

		bool no_text_before = (begin == input.begin() || *(begin - 1) == '\n');
		bool no_text_after = (i == input.end() || *i == '\n');

		if (no_text_before && no_text_after && type != type_t::substitution) {
			if (i != end) i += "\n"sv.size();
		}

		return i;
	}


	inline iterator directive::_parse_as_comment(iterator begin, iterator directive_end) {
		using std::literals::operator""sv;

		type = type_t::comment;
		return directive_end + "}"sv.size();
	}


	inline iterator directive::_parse_as_substitution(iterator begin, iterator directive_end) {
		using std::literals::operator""sv;

		type = type_t::substitution;

		auto i = begin + "{"sv.size();
		i = skip_whitespace(i, directive_end);
		i = parse_param_name(i, directive_end, param_name);
		i = skip_whitespace(i, directive_end);

		if (i != directive_end) {
			auto errc = preprocess_text_errc::too_many_parameter_names;
			throw preprocess_text_error{errc, i};
		}

		return i + "}"sv.size();
	}


	inline iterator directive::_parse_as_command(iterator begin, iterator directive_end) {
		using std::literals::operator""sv;

		auto i = begin + "{!"sv.size();
		i = skip_whitespace(i, directive_end);

		i = parse_command_name(i, directive_end, command_name);
		type = to_type(command_name);

		if (type == type_t::if_command
			|| type == type_t::else_if_command
			|| type == type_t::list_command)
		{
			i = skip_whitespace(i, directive_end);
			i = parse_param_name(i, directive_end, param_name);
		}

		i = skip_whitespace(i, directive_end);

		if (i != directive_end) {
			auto errc = preprocess_text_errc::too_many_parameter_names;
			throw preprocess_text_error{errc, i};
		}

		return i + "}"sv.size();
	}


	inline iterator plain_text::parse(iterator begin, iterator end, std::string_view input) {
		for (iterator i = begin; ; ) {
			auto next = find_delimiter(begin, end);
			str.append(i, next);
			i = next;

			if (i != end && *i == '\\') {
				i = parse_escape_sequence(i, end, str);
			} else {
				return i;
			}
		}
	}


	inline iterator substitution::parse(iterator begin, iterator end, std::string_view input) {
		using std::literals::string_view_literals::operator""sv;

		directive d;
		auto next = d.parse(begin, end, input);

		// FIXME: Throw an exception if `d.type` is not `substitution`.

		param_name = d.param_name;
		return next;
	}


	inline iterator sequence::parse(iterator begin, iterator end, std::string_view input) {
		for (auto i = begin; i != end; ) {
			std::unique_ptr<expression> expr;

			if (char ch = *i; is_brace(ch)) {
				directive d;
				auto next = d.parse(i, end, input);

				switch (d.type) {
				case directive::type_t::comment:
					i = next;
					continue;
				case directive::type_t::substitution:
					expr = std::make_unique<substitution>();
					break;
				case directive::type_t::if_command:
					expr = std::make_unique<conditional>();
					break;
				case directive::type_t::list_command:
					expr = std::make_unique<loop>();
					break;
				default:
					return i; // stop when directive is not recognised
				}
			} else {
				expr = std::make_unique<plain_text>();
			}

			i = expr->parse(i, end, input);
			subexprs.push_back(std::move(expr));
		}

		return end;
	}


	inline iterator conditional::parse(iterator begin, iterator end, std::string_view input) {
		iterator i = begin;

		// # Key
		// - Stage 1: Parse the "if" command
		// - Stage 2: Parse the (optional) "else_if" and "else" commands
		// - Stage 3: Parse the "end" command
		for (int stage = 1; ; ) {
			directive d;
			auto next = d.parse(i, end, input);

			if (stage == 1) {
				d.verify(directive::type_t::if_command);

				sequence expr;
				i = expr.parse(next, end, input);
				conditional_subexprs.emplace_back(d.param_name, std::move(expr));
				++stage;
			} else if (stage == 2) {
				if (d.type == directive::type_t::else_if_command) {
					sequence expr;
					i = expr.parse(next, end, input);
					conditional_subexprs.emplace_back(d.param_name, std::move(expr));
				} else if (d.type == directive::type_t::else_command) {
					sequence expr;
					i = expr.parse(next, end, input);
					default_subexpr = std::move(expr);
					++stage;
				} else {
					++stage;
				}
			} else /* stage == 3 */ {
				d.verify(directive::type_t::end_command);

				return next;
			}

			if (i == end) {
				auto errc = preprocess_text_errc::unclosed_expression;
				throw preprocess_text_error{errc, begin};
			}
		}
	}


	inline iterator loop::parse(iterator begin, iterator end, std::string_view input) {
		auto i = begin;

		{ // Stage 1: Parse the "list" command
			directive d;
			i = d.parse(i, end, input);
			d.verify(directive::type_t::list_command);
			param_name = d.param_name;
		}

		i = subexpr.parse(i, end, input);

		if (i == end) {
			auto errc = preprocess_text_errc::unclosed_expression;
			throw preprocess_text_error{errc, begin};
		}

		{ // Stage 2: Parse the "end" command
			directive d;
			i = d.parse(i, end, input);
			d.verify(directive::type_t::end_command);
		}

		return i;
	}
}


inline std::string maf::preprocess_text(std::string_view input,
								        TextParams const& params)
{
	using namespace _preprocess_text_impl;

	std::string output;

	sequence expr;
	auto next = expr.parse(input.begin(), input.end(), input);

	if (next != input.end()) {
		directive d;
		d.parse(next, input.end(), input);

		auto errc = preprocess_text_errc::unexpected_command;
		auto name = d.command_name;
		throw preprocess_text_error{errc, name.begin(), name.end()};
	}

	expr.write(output, params);

	return output;
}


namespace maf::_format_text_impl {
	using iterator = std::string_view::iterator;

	inline bool is_format_code(char ch) {
		return ch == '=' || ch == '_' || ch == '%' || ch == '@';
	}

	inline bool is_delimiter(char ch) {
		return is_format_code(ch) || ch == '\\';
	}

	inline bool is_escapable(char ch) {
		return is_delimiter(ch) || ch == '{' || ch == '}';
	}

	inline iterator find_delimiter(iterator begin, iterator end) {
		return std::find_if(begin, end, is_delimiter);
	}

	inline iterator find_escapable(iterator begin, iterator end) {
		return std::find_if(begin, end, is_escapable);
	}

	inline std::string escaped(iterator begin, iterator end) {
		std::string output;

		for (auto i = begin; ; ) {
			auto j = find_escapable(i, end);
			output.append(i, j);
			if (j == end) return output;
			output += {'\\', *j};
			i = j + 1;
		}

		return output;
	}


	// # Example
	// Given the following input:
	// ```
	//   begin  end
	//   |      |
	// " \{ ... "
	//     |
	//     next
	// ```
	// append "{" to `str` and return `next`.
	//
	// # Exceptions
	// - Throws `format_text_error` if the range `{begin, begin+1}` does not
	// represent an escape sequence.
	inline iterator	parse_escape_sequence(iterator begin, iterator end,
										  std::string & str)
	{
		// FIXME: Throw exception if `begin` does not point at a backslash.

		auto i = begin + 1;

		if (i == end) {
			auto errc = format_text_errc::dangling_backslash;
			throw format_text_error{errc, begin};
		}

		if (char ch = *i; is_escapable(ch)) {
			str += ch;
		} else {
			auto errc = format_text_errc::invalid_escape_sequence;
			throw format_text_error{errc, begin};
		}

		return i + 1;
	}


	inline void move_block(StyledText & text, std::string & block,
						   StyledString::Style style)
	{
		if (!block.empty()) {
			text.emplace_back(block, style);
			block.clear();
		}
	}


	inline void update_style(StyledString::Style & style, char bit_name) {
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


	inline StyledText format_text(iterator begin, iterator end,
								  StyledString::Style current_style)
	{
		std::string block;
		StyledText output;

		for (auto i = begin; ; ) {
			auto j = find_delimiter(i, end);

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
				move_block(output, block, current_style);
				return output;
			}

			if (char ch = *j; is_format_code(ch)) {
				move_block(output, block, current_style);
				update_style(current_style, ch);
				i = j + 1;
			} else { // ch is a backslash
				i = parse_escape_sequence(j, end, block);
			}
		}
	}
}


inline std::string maf::escaped(std::string_view str) {
	using namespace _format_text_impl;
	return _format_text_impl::escaped(str.begin(), str.end());
}


inline maf::StyledText maf::format_text(std::string_view input,
							            StyledString::Style current_style)
{
	using namespace _format_text_impl;
	return _format_text_impl::format_text(input.begin(), input.end(), current_style);
}


#endif
