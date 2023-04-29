#ifndef MAFIA_FORMAT
#define MAFIA_FORMAT

#include <map>

#include "../util/misc.hpp"
#include "../util/string.hpp"
#include "../util/variant.hpp"
#include "../util/vector.hpp"

namespace maf {
	// Create a new string based on `str_view`, but with each escapable
	// character prefixed by a backslash `\`.
	//
	// # Examples
	// - `escaped("My {string}")` returns `"My \{string\}"`
	// - `escaped("under_score")` returns `"under\_score"`
	string escaped(string_view str_view);

	// forward declaration needed by TextParam
	class TextParams;

	// A single parameter used when preprocessing text.
	class TextParam : public variant<bool, int, string, vector<TextParams>> {
	public:
		using variant<bool, int, string, vector<TextParams>>::variant;
	};

	// A map from strings to text parameters.
	//
	// Note that the map only holds views into its keys, whereas the values
	// are fully owned. Typically each key will be a compile-time constant.
	class TextParams : public std::map<string_view, TextParam> {
	public:
		using std::map<string_view, TextParam>::map;
	};

	// Type for exceptions that can be thrown when calling `preprocess_text`.
	struct preprocess_text_error {
		enum class error_code {
			missing_integer,
			token_not_integer,
			integer_out_of_range,

			missing_parameter,
			token_not_param_name,

			missing_command,
			token_not_command,

			missing_comp_operand,
			token_not_comp_operand,

			missing_relation,
			token_not_relation,

			missing_escape_sequence,
			token_not_escape_sequence,

			unclosed_directive,
			too_many_closing_braces,
			unexpected_command,
			too_many_tokens,

			unclosed_expression,

			parameter_not_available,
			wrong_parameter_type
		};

		// An error code signifying what went wrong.
		error_code code;

		// The input string that couldn't be processed.
		string_view input;

		// Further info about the error. This will be either:
		// - an iterator pointing to where the error occurred; or
		// - the substring that caused the error.
		variant<string_view::iterator, string_view> param;

		// Position in input string where the error occurred. It's computed
		// from `this->input` and `this->param`.
		index pos() const;

		// Construct an error whose `param` is `iter`.
		//
		// # Preconditions
		// - `iter` must point to a character in `input`.
		preprocess_text_error(error_code code, string_view::iterator iter):
			code{code},
			param{iter}
		{}

		// Construct an error whose `param` is `substr`.
		//
		// # Preconditions
		// - `substr` must be a subview of `input`.
		preprocess_text_error(error_code code, string_view substr):
			code{code},
			param{substr}
		{}

		// Get a description of the error.
		string message() const;
	};

	// Find all directives of the form `"{...}"` in `input` and perform a
	// corresponding set of actions.
	//
	// Escape sequences of the form `"\x"` are left intact. In particular,
	// braces preceded by backslashes `"\"` are not treated as parts of command
	// names, unless the backslash itself is escaped, as in `"\\"`.
	//
	// # Returns
	// An output string after applying the directives in `input`.
	//
	// # Exceptions
	// Throws `preprocess_text_error` with an appropriate code if something goes
	// wrong while parsing. Use `write()` on the error to see more information.
	//
	// # Example
	// If `params` is `{ {"word1", "NEW"}, {"word2", "STRING"} }`,
	// then
	// ```
	// "This is my {word1} {word2} with \{braces\}."
	// ```
	// will be transformed into
	// ```
	// "This is my NEW STRING with \{braces\}."
	// ```
	// There are many more directives possible than this example demonstrates.
	// See the file `resources/txt/cheatsheet.txt` included with the source code
	// for further information.
	string preprocess_text(string_view input, TextParams const& params);


	// A string coupled with a set of suggested attributes. Each attribute is
	// intended to entail such properties as typeface, font size, colour, etc.
	struct StyledString {
		struct attributes_t {
			enum class style_option {
				normal,
				italic
			};

			enum class weight_option {
				normal,
				bold
			};

			enum class typeface_option {
				sans_serif,
				serif,
				monospace
			};

			enum class semantics_option {
				normal,
				title,
				help_text,
				flavour_text
			};

			style_option style;
			weight_option weight;
			typeface_option typeface;
			semantics_option semantics;
		};

		static constexpr attributes_t default_attributes = {};

		static constexpr attributes_t monospace_attributes = {
			attributes_t::style_option::normal,
			attributes_t::weight_option::normal,
			attributes_t::typeface_option::monospace,
			attributes_t::semantics_option::normal};

		static constexpr attributes_t title_attributes = {
			attributes_t::style_option::normal,
			attributes_t::weight_option::normal,
			attributes_t::typeface_option::serif,
			attributes_t::semantics_option::title};

		static constexpr attributes_t help_text_attributes = {
			attributes_t::style_option::normal,
			attributes_t::weight_option::normal,
			attributes_t::typeface_option::serif,
			attributes_t::semantics_option::help_text};

		StyledString() = default;

		StyledString(string str, attributes_t attributes)
		: str{str}, attributes{attributes} { }

		string str;
		attributes_t attributes;
	};

	// A vector of styled strings, used to form a block of text.
	using StyledText = vector<StyledString>;

	// Type for exceptions that can be thrown when calling `format_text`.
	struct format_text_error {
		enum class error_code {
			invalid_escape_sequence
		};

		// An error code signifying what went wrong.
		error_code code;

		// The input string that couldn't be processed.
		string_view input;

		// The substring that caused the error.
		string_view param;

		// Position in input string where the error occurred. It's computed
		// from `this->input` and `this->param`.
		index pos() const;

		format_text_error(error_code code, string_view substr):
			param{substr}
		{}

		// Get a description of the error.
		string message() const;
	};

	// Parse `input` to find all of its special characters and perform a
	// sequence of actions as described below.
	//
	// The general idea is that blocks of text will be extracted from `{begin,
	// end}` and added to the output. A set of formatting codes can be used to
	// change the attributes for a given block. Certain escape sequences are
	// also recognised.
	//
	// Each time a formatting code is found, a new block of text is added to
	// the output, using the attributes from just before the formatting code
	// was applied. Its content is taken to be the string between the previous
	// formatting code and the new formatting code.
	//
	// See the file `resources/txt/cheatsheet.txt` included with this source
	// code for more information.
	StyledText format_text(string_view input,
		StyledString::attributes_t attributes = StyledString::default_attributes);
}

#endif
