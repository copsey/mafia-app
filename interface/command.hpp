#ifndef MAFIA_INTERFACE_COMMAND
#define MAFIA_INTERFACE_COMMAND

#include <string_view>
#include <vector>

#include "../util/algorithm.hpp"
#include "../util/stdlib.hpp"
#include "../util/string.hpp"

namespace maf {
	// A sequence of read-only strings, treated as a set of commands.
	//
	// Note that only views into an input string are stored; make sure that
	// the lifetime of a command sequence doesn't exceed the input string!
	using CmdSequence = vector<string_view>;

	// Decide whether or not the sequence `seq` of string-like objects matches
	// the array `arr` of commands, which is true exactly when
	// `std::size(seq)` equals `std::size(arr)`, and at each position `i`,
	// either `arr[i]` is a blank string or `seq[i]` equals `arr[i]`.
	template <std::size_t N>
	bool commands_match(const CmdSequence & seq, const string_view (&arr)[N]) {
		auto eq = [](auto& s1, string_view s2) {
			return std::empty(s2) || s1 == s2;
		};

		return util::matches(seq, arr, eq);
	}

	// Split the string contained in the range `{begin, end}` into a sequence
	// of commands, delimited by spaces `' '` and tabs `'\t'`.
	//
	// # Returns
	// A vector of views into the range. Make sure the return value doesn't
	// exceed the lifetime of the input range!
	//
	// # Example
	// `"do X   with Y"` maps to `{"do", "X", "with", "Y"}`.
	template <typename Iterator>
	CmdSequence parse_input(Iterator begin, Iterator end) {
		auto is_space = [](char ch) { return ch == ' ' || ch == '\t'; };
		
		CmdSequence commands;
		
		for (auto i = std::find_if_not(begin, end, is_space); i != end; ) {
			auto j = std::find_if(i, end, is_space);
			
			// Here we are guaranteed that i != j, since
			//  1) i points to a non-space character, and
			//  2) either j points to a space or j == end.

			auto cmd = util::make_string_view(i, j);
			commands.push_back(cmd);
			
			i = std::find_if_not(j, end, is_space);
		}
		
		return commands;
	}

	inline CmdSequence parse_input(string_view str) {
		return parse_input(str.begin(), str.end());
	}
}

#endif
