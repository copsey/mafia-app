#ifndef MAFIA_UTIL_RANDOM_H
#define MAFIA_UTIL_RANDOM_H

#include <algorithm>
#include <iterator>
#include <random>
#include <ranges>

namespace maf::util {
	// A `std::mt19937` used by various algorithms.
	// Automatically seeded when the program first starts.
	inline std::mt19937 random_engine{std::random_device{}()};

	// Pick a random position in `[b,e)` using `g`.
	//
	// If `b == e`, returns `e` instead.
	template <std::forward_iterator Iter, typename RNG>
	Iter pick_with(Iter b, Iter e, RNG && g) {
		if (b == e) return e;

		auto n = std::distance(b, e);
		std::uniform_int_distribution<decltype(n)> dist{0, n - 1};

		std::advance(b, dist(g));
		return b;
	}

	// Pick a random position in `c` using `g`.
	//
	// If `c` is empty, returns `std::end(c)` instead.
	template <typename Cont, typename RNG>
	auto pick_with(Cont & c, RNG && g) -> decltype(std::begin(c)) {
		if (std::empty(c)) return std::end(c);

		auto n = std::size(c);
		std::uniform_int_distribution<decltype(n)> dist(0, n - 1);

		auto b = std::begin(c);
		std::advance(b, dist(g));
		return b;
	}

	// Pick a random position in `[b,e)` using `util::random_engine`.
	//
	// If `b == e`, returns `e` instead.
	template <std::forward_iterator Iter>
	Iter pick(Iter b, Iter e) {
		return pick_with(b, e, random_engine);
	}

	// Pick a random position in `c` using `util::random_engine`.
	//
	// If `c` is empty, returns `std::end(c)` instead.
	template <typename Cont>
	auto pick(Cont & c) -> decltype(std::begin(c)) {
		return pick_with(c, random_engine);
	}
}

#endif
