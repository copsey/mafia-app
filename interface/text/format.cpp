#include "../format.hpp"
#include "../../util/algorithm.hpp"
#include "../../util/string.hpp"


namespace maf::_format_text_impl {
	using iterator = string_view::iterator;
	using error = format_text_error;
	using errc = error::error_code;

	bool is_format_code(char ch) {
		switch (ch) {
		case '_': case '*': case '@': case '=': case '$': case '~':
			return true;
		default:
			return false;
		}
	}

	bool is_delimiter(char ch) {
		return is_format_code(ch) || ch == '\\';
	}

	bool is_escapable(char ch) {
		return is_delimiter(ch) || ch == '{' || ch == '}';
	}

	iterator find_delimiter(iterator begin, iterator end) {
		return std::find_if(begin, end, is_delimiter);
	}

	iterator find_escapable(iterator begin, iterator end) {
		return std::find_if(begin, end, is_escapable);
	}

	string escaped(iterator begin, iterator end) {
		string output;

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
	iterator parse_escape_sequence(iterator begin, iterator end, string & str)
	{
		if (begin == end) {
			auto substr = util::make_string_view(begin, end);
			throw error{errc::invalid_escape_sequence, substr};
		}

		auto i = begin + 1;

		if (i == end) {
			auto substr = util::make_string_view(begin, i);
			throw error{errc::invalid_escape_sequence, substr};
		}

		if (char ch = *i; *begin == '\\' && is_escapable(ch)) {
			str += ch;
		} else if (*begin == '\\' && ch == '\n') {
			// exclude this character from `str`
		} else {
			auto substr = util::make_string_view(begin, i + 1);
			throw error{errc::invalid_escape_sequence, substr};
		}

		return i + 1;
	}


	void move_block(StyledText & text, string & block,
		StyledString::attributes_t attributes)
	{
		if (!block.empty()) {
			text.emplace_back(block, attributes);
			block.clear();
		}
	}


	void toggle_italic_style(StyledString::attributes_t & attributes) {
		using option = StyledString::attributes_t::style_option;

		switch (attributes.style) {
		case option::italic:
			attributes.style = option::normal;
			break;
		default:
			attributes.style = option::italic;
			break;
		}
	}


	void toggle_bold_weight(StyledString::attributes_t & attributes) {
		using option = StyledString::attributes_t::weight_option;

		switch (attributes.weight) {
		case option::normal:
			attributes.weight = option::bold;
			break;
		case option::bold:
			attributes.weight = option::normal;
			break;
		}
	}


	void toggle_serif_typeface(StyledString::attributes_t & attributes) {
		using option = StyledString::attributes_t::typeface_option;

		switch (attributes.typeface) {
		case option::serif:
			attributes.typeface = option::sans_serif;
			break;
		default:
			attributes.typeface = option::serif;
			break;
		}
	}


	void toggle_monospace_typeface(StyledString::attributes_t & attributes) {
		using option = StyledString::attributes_t::typeface_option;

		switch (attributes.typeface) {
		case option::monospace:
			attributes.typeface = option::sans_serif;
			break;
		default:
			attributes.typeface = option::monospace;
			break;
		}
	}


	void toggle_title_semantics(StyledString::attributes_t & attributes) {
		using option = StyledString::attributes_t::semantics_option;

		switch (attributes.semantics) {
		case option::title:
			attributes.semantics = option::normal;
			break;
		default:
			attributes.semantics = option::title;
			break;
		}
	}


	void toggle_help_text_semantics(StyledString::attributes_t & attributes) {
		using option = StyledString::attributes_t::semantics_option;

		switch (attributes.semantics) {
		case option::help_text:
			attributes.semantics = option::normal;
			break;
		default:
			attributes.semantics = option::help_text;
			break;
		}
	}


	void toggle_flavour_text_semantics(StyledString::attributes_t & attributes) {
		using option = StyledString::attributes_t::semantics_option;

		switch (attributes.semantics) {
		case option::flavour_text:
			attributes.semantics = option::normal;
			break;
		default:
			attributes.semantics = option::flavour_text;
			break;
		}
	}


	void update_style(StyledString::attributes_t & attributes, char format_code) {
		switch (format_code) {
		case '_':
			toggle_italic_style(attributes);
			break;
		case '*':
			toggle_bold_weight(attributes);
			break;
		case '@':
			toggle_monospace_typeface(attributes);
			break;
		case '=':
			toggle_title_semantics(attributes);
			break;
		case '$':
			toggle_help_text_semantics(attributes);
			break;
		case '~':
			toggle_flavour_text_semantics(attributes);
			break;
		default:
			break;
		}
	}


	StyledText format_text(iterator begin, iterator end,
		StyledString::attributes_t attributes)
	{
		string block;
		StyledText output;

		for (auto i = begin; ; ) {
			auto j = find_delimiter(i, end);

			// e.g.
			//       i             j
			//       |             |
			// "... $You can enter @help@ if you need..."

			// e.g.
			//          i            j
			//          |            |
			// "... @ok@ to continue."

			block.append(i, j);

			if (j == end) {
				move_block(output, block, attributes);
				return output;
			}

			if (char ch = *j; is_format_code(ch)) {
				move_block(output, block, attributes);
				update_style(attributes, ch);
				i = j + 1;
			} else { // ch is a backslash
				i = parse_escape_sequence(j, end, block);
			}
		}
	}
}


maf::string maf::escaped(string_view str) {
	return _format_text_impl::escaped(str.begin(), str.end());
}


maf::StyledText maf::format_text(string_view input,
    StyledString::attributes_t attributes)
{
	try {
		auto begin = input.begin();
		auto end = input.end();
		return _format_text_impl::format_text(begin, end, attributes);
	} catch (format_text_error & error) {
		error.input = input;
		throw;
	}
}


maf::index maf::format_text_error::pos() const {
	return param.begin() - input.begin();
}


maf::string maf::format_text_error::message() const {
	string msg;
	auto pos = std::to_string(this->pos());

	switch (this->code) {
	case error_code::invalid_escape_sequence:
		msg += "Invalid escape sequence \"";
		msg += this->param;
		msg += "\" at position ";
		msg += pos;
		break;
	}

	return msg;
}
