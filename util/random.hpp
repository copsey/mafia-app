#ifndef MAFIA_UTIL_RANDOM_H
#define MAFIA_UTIL_RANDOM_H

#include <algorithm>
#include <concepts>
#include <iterator>
#include <random>
#include <ranges>

namespace maf::util::random {
	// A `std::default_random_engine` used by various algorithms.
	// Automatically seeded when the program first starts.
	inline auto default_generator = std::default_random_engine{std::random_device{}()};

	// Generate a single result from a uniform integer distribution with
	// minimum value `a` and maximum value `b`.
	template <std::integral Int = int>
	auto uniform_int_trial(Int a, Int b) -> Int {
		auto dist = std::uniform_int_distribution{a, b};
		return dist(default_generator);
	}

	// Generate a single result from a Bernoulli distribution with probability
	// `p` of success.
	inline bool bernoulli_trial(double p) {
		auto dist = std::bernoulli_distribution{p};
		return dist(default_generator);
	}

	// Generate a single result from a discrete distribution with the
	// provided `weights`.
	template <std::integral ResultType = int>
	auto discrete_trial(auto&& weights) -> ResultType {
		auto b    = std::begin(weights);
		auto e    = std::end(weights);
		auto dist = std::discrete_distribution<ResultType>{b, e};

		return dist(default_generator);
	}

	// Pick a random position in `range` using `random::default_generator`.
	//
	// If `range` is empty, returns `std::end(range)` instead.
	auto pick(auto& range) -> decltype(std::begin(range)) {
		if (std::empty(range)) return std::end(range);

		auto i = std::begin(range);
		auto n = uniform_int_trial<int>(0, std::size(range) - 1);
		std::advance(i, n);

		return i;
	}

	// Pick a random position in `range` using `random::default_generator`.
	// The likelihood of each position being selected is determined by
	// the corresponding weight in `weights`.
	//
	// If `range` is empty, returns `std::end(range)` instead.
	//
	// Undefined behaviour if `range` and `weights` are not the same size.
	auto pick(auto& range, auto&& weights) -> decltype(std::begin(range)) {
		if (std::empty(range)) return std::end(range);

		auto i = std::begin(range);
		auto n = discrete_trial(weights);
		std::advance(i, n);

		return i;
	}
}

#endif
