#ifndef MAFIA_INTERFACE_COMMAND
#define MAFIA_INTERFACE_COMMAND

#include <string_view>
#include <vector>

#include "../riketi/algorithm.hpp"

namespace maf {
	// Decide whether or not the sequence `seq` of string-like objects matches
	// the array `arr` of commands, which is true exactly when
	// `std::size(seq)` equals `std::size(arr)`, and at each position `i`,
	// either `arr[i]` is a blank string or `seq[i]` equals `arr[i]`.
	template <typename Seq, std::size_t N>
	bool commands_match(const Seq & seq, const std::string_view (&arr)[N])
	{
		auto eq = [](auto& s1, std::string_view s2) {
			return std::empty(s2) || s1 == s2;
		};

		return rkt::matches(seq, arr, eq);
	}

	template <typename Str, std::size_t N>
	bool commands_match(const std::vector<Str> & v,
						const std::string_view (&arr)[N])
	{
		auto eq = [](auto& s1, std::string_view s2) {
			return std::empty(s2) || s1 == s2;
		};

		if constexpr(N == 0) {
			return std::size(v) == 0;
		} else if constexpr(N == 1) {
			return std::size(v) == 1 && eq(v[0], arr[0]);
		} else if constexpr(N == 2) {
			return std::size(v) == 2 && eq(v[0], arr[0]) && eq(v[1], arr[1]);
		} else if constexpr(N == 3) {
			return std::size(v) == 3 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]);
		} else if constexpr(N == 4) {
			return std::size(v) == 4 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]) && eq(v[3], arr[3]);
		} else if constexpr(N == 5) {
			return std::size(v) == 5 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]) && eq(v[3], arr[3]) && eq(v[4], arr[4]);
		} else {
			return rkt::matches(v, arr, eq);
		}
	}

	// Split the string contained in the range `{begin,end}` into a sequence
	// of commands, delimited by spaces ' ' and tabs '\t'.
	//
	// @returns A vector of views into the range.
	//
	// @example `"do X   with Y"` maps to `{"do", "X", "with", "Y"}`.
	//
	// @pre `{begin,end}` is a valid range.
	// @pre `ForwardIter` can be converted to `std::string_view::iterator`.
	template <typename ForwardIter>
	inline auto parse_input(ForwardIter begin, ForwardIter end)
	-> std::vector<std::string_view>
	{
		auto is_space = [](char ch) { return ch == ' ' || ch == '\t'; };
		
		std::vector<std::string_view> commands;
		
		for (auto i = std::find_if_not(begin, end, is_space); i != end; ) {
			auto j = std::find_if(i, end, is_space);
			
			// Note: here we are guaranteed that i != j, since
			// 1) *i is not a space, and 2) either *j is a space or *j == end.
			auto iter = std::string_view::iterator{i};
			auto dist = std::distance(i, j);
			auto size = static_cast<std::string_view::size_type>(dist);
			commands.emplace_back(iter, size);
			
			i = std::find_if_not(j, end, is_space);
		}
		
		return commands;
	}

	inline auto parse_input(std::string_view str)
	-> std::vector<std::string_view>
	{
		return parse_input(str.begin(), str.end());
	}
}

#endif
