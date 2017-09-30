#ifndef RIKETI_RANDOM
#define RIKETI_RANDOM

#include <algorithm>
#include <iterator>
#include <random>

#include "container.hpp"

namespace rkt {
	// A `std::mt19937` used by various algorithms.
	// Automatically seeded when the program first starts.
	extern std::mt19937 random_engine;
	
	// Pick a random position in `[b,e)` using `g`.
	//
	// If `b == e`, returns `e` instead.
	template <typename Iter, typename RNG>
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
	container::iterator_type<Cont> pick_with(Cont & c, RNG && g) {
		if (container::is_empty(c)) return std::end(c);
		
		auto n = container::size(c);
		std::uniform_int_distribution<decltype(n)> dist{0, n - 1};
		
		auto b = std::begin(c);
		std::advance(b, dist(g));
		return b;
	}
	template <typename Cont, typename RNG>
	container::const_iterator_type<Cont> pick_with(const Cont & c, RNG && g) {
		if (container::is_empty(c)) return std::end(c);
		
		auto n = container::size(c);
		std::uniform_int_distribution<decltype(n)> dist{0, n - 1};
		
		auto b = std::begin(c);
		std::advance(b, dist(g));
		return b;
	}
	
	// Pick a random position in `[b,e)` using `rkt::random_engine`.
	//
	// If `b == e`, returns `e` instead.
	template <typename Iter>
	Iter pick(Iter b, Iter e) {
		return pick_with(b, e, random_engine);
	}
	
	// Pick a random position in `c` using `rkt::random_engine`.
	//
	// If `c` is empty, returns `std::end(c)` instead.
	template <typename Cont>
	container::iterator_type<Cont> pick(Cont & c) {
		return pick_with(c, random_engine);
	}
	template <typename Cont>
	container::const_iterator_type<Cont> pick(const Cont & c) {
		return pick_with(c, random_engine);
	}
	
	// Randomise the order of elements in `c` using `g`.
	template <typename Cont, typename RNG>
	void shuffle_with(Cont & c, RNG && g) {
		std::shuffle(std::begin(c), std::end(c), g);
	}
	
	// Randomise the order of elements in `[b,e)` using `rkt::random_engine`.
	template <typename Iter>
	void shuffle(Iter b, Iter e) {
		std::shuffle(b, e, random_engine);
	}
	
	// Randomise the order of elements in `c` using `rkt::random_engine`.
	template <typename Cont>
	void shuffle(Cont & c) {
		std::shuffle(std::begin(c), std::end(c), random_engine);
	}
}

#endif
