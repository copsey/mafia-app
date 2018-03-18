#ifndef JSON_PRETTY_PRINT
#define JSON_PRETTY_PRINT

#include "../ios/repeat.hpp"

namespace json {
	template <typename Stream>
	// A wrapper for I/O streams, used for pretty-printing JSON values.
	// 
	// Automatically adds indentation and newlines during output operations
	// where appropriate. The string used for indentation is customisable,
	// along with the initial indentation level.
	struct pretty_print_t {
		// The type of the underlying stream.
		using stream_type = Stream;
		// The character type in the stream.
		using char_type   = typename Stream::char_type;
		// The character sequence type used for indentation.
		using string_type = const char_type *;
		// The type used for writing indents.
		using indent_type = util::repeat_t<string_type>;
		// The integer type for keeping track of the indentation level.
		using size_type   = typename util::repeat_t<string_type>::size_type;

		pretty_print_t(
			stream_type & stream,
			size_type     indent_level  = 0,
			string_type   indent_string = "  ")
		:
			str_p {&stream},
			ind_s {indent_string},
			level {indent_level}
		{ }

		// The underlying stream.
		stream_type &       base_stream() { return *str_p; }
		const stream_type & base_stream() const { return *str_p; }

		// The sequence of characters used when outputting an indent.
		string_type indent_string() const {
			return ind_s;
		}

		// The current indentation level.
		size_type indent_level() const {
			return level;
		}

		// Subtract 1 from the indentation level.
		// 
		// \pre this->indent_level() is greater than 0.
		void dec() { --level; }

		// Add 1 to the indentation level.
		void inc() { ++level; }

		// Get the current indent.
		indent_type indent() const {
			return indent_type {ind_s, level};
		}

	private:
		stream_type* str_p;
		string_type  ind_s;
		size_type    level {0}; // current indentation level
	};

	template <typename Stream>
	pretty_print_t<Stream> use_pretty_print(
		Stream &                                       stream,
		typename pretty_print_t<Stream>::size_type     indent_level  = 0,
		typename pretty_print_t<Stream>::string_type   indent_string = "  ")
	{
		return pretty_print_t<Stream> {stream, indent_level, indent_string};
	}

	template <typename Stream, typename Obj>
	// Read a value of type Obj from inp.
	// 
	// This simply wraps operator>> for the underlying stream.
	auto operator>> (pretty_print_t<Stream> & inp, Obj& x)
		-> pretty_print_t<Stream> &
	{
		auto& base_str = inp.base_stream();
		base_str >> x;
		return inp;
	}

	template <typename Stream, typename Obj>
	// Write a value of type Obj to out.
	// 
	// By default, this simply wraps operator<< for the underlying stream.
	// This function should be overloaded for JSON value types.
	auto operator<< (pretty_print_t<Stream> & out, const Obj& x)
		-> pretty_print_t<Stream> &
	{
		auto& base_str = out.base_stream();
		base_str << x;
		return out;
	}
}

#endif