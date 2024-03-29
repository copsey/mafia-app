#ifndef MAFIA_UTIL_ALGORITHM_H
#define MAFIA_UTIL_ALGORITHM_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <type_traits>

#include "random.hpp"

namespace maf::util {
	// Perform `f(x)` on each member `x` of `cont`.
	template <typename Container, typename Function>
	Function for_each(Container & cont, Function f) {
		using std::begin, std::end;
		return std::for_each(begin(cont), end(cont), f);
	}

	// Check if `p(t)` is true for all `t` in `c`.
	template <typename Cont, typename Pred>
	bool all_of(Cont const& c, Pred p) {
		using std::begin, std::end;
		return std::all_of(begin(c), end(c), p);
	}

	// Check if `p(t)` is true for at least one `t` in `c`.
	template <typename Cont, typename Pred>
	bool any_of(Cont const& c, Pred p) {
		using std::begin, std::end;
		return std::any_of(begin(c), end(c), p);
	}

	// Check if `p(t)` is false for all `t` in `c`.
	template <typename Cont, typename Pred>
	bool none_of(Cont const& c, Pred p) {
		using std::begin, std::end;
		return std::none_of(begin(c), end(c), p);
	}

	// Check if `t` is equal to any values in the range `[i,j)`.
	template <typename Iter, typename T>
	bool contains(Iter i, Iter j, const T & t) {
		for ( ; i != j; ++i) {
			if (*i == t) return true;
		}

		return false;
	}

	// Check if `t` is equal to any elements in `c`.
	template <typename Cont, typename T>
	bool contains(const Cont & c, const T & t) {
		using std::begin, std::end;
		return util::contains(begin(c), end(c), t);
	}

	// Check if the two sequences `[i1,j1)` and `[i2,j2)` match,
	// using `eq` for comparison of values.
	//
	// example 1:
	//   seq 1: [`x1`, `x2`, `x3`, `x4`]
	//   seq 2: [`y1`, `y2`, `y3`, `y4`]
	// `matches(i1, j1, i2, j2, eq)` is true if `eq(xn,yn)` for each n.
	//
	// example 2:
	//   seq 1: [`x1`, `x2`]
	//   seq 2: [`y1`, `y2`, `y3`]
	// `matches(i1, j1, i2, j2, eq)` is false because the sequences have
	// different lengths.
	template <typename Iter1, typename Iter2, typename Eq>
	bool matches(Iter1 i1, Iter1 j1, Iter2 i2, Iter2 j2, Eq eq) {
		for ( ; i1 != j1; ++i1, ++i2) {
			if (i2 == j2) return false;
			if (!eq(*i1, *i2)) return false;
		}

		return i2 == j2;
	}

	// Same as `matches(i1, j1, i2, j2, eq)`, where
	//   `i1 == std::begin(c1)`, `j1 == std::end(c1)`,
	//   `i2 == std::begin(c2)`, `j2 == std::end(c2)`.
	template <typename Cont1, typename Cont2, typename Eq>
	bool matches(const Cont1 & c1, const Cont2 & c2, Eq eq) {
		using std::begin, std::end;
		return util::matches(begin(c1), end(c1), begin(c2), end(c2), eq);
	}

	// Check if the two sequences `[i1,j1)` and `[i2,j2)` are equivalent.
	// Comparison of values is made using `==`.
	//
	// example 1:
	//   seq 1: [`x1`, `x2`, `x3`, `x4`]
	//   seq 2: [`y1`, `y2`, `y3`, `y4`]
	// `equivalent(i1, j1, i2, j2)` is true if `xn == yn` for each n.
	//
	// example 2:
	//   seq 1: [`x1`, `x2`]
	//   seq 2: [`y1`, `y2`, `y3`]
	// `equivalent(i1, j1, i2, j2)` is false because the sequences have
	// different lengths.
	template <typename Iter1, typename Iter2>
	bool equivalent(Iter1 i1, Iter1 j1, Iter2 i2, Iter2 j2) {
		using T1 = decltype(*i1);
		using T2 = decltype(*i2);

		auto eq = [](const T1 & t1, const T2 & t2) { return t1 == t2; };
		return util::matches(i1, j1, i2, j2, eq);
	}

	// Same as `equivalent(i1, j1, i2, j2)`, where
	//   `i1 == std::begin(c1)`, `j1 == std::end(c1)`,
	//   `i2 == std::begin(c2)`, `j2 == std::end(c2)`.
	template <typename Cont1, typename Cont2>
	bool equivalent(const Cont1 & c1, const Cont2 & c2) {
		using std::begin, std::end;
		return util::equivalent(begin(c1), end(c1), begin(c2), end(c2));
	}

	// Find the first position `i` in `c` such that `*i == t`.
	//
	// If no such position exists, `std::end(c)` is returned instead.
	template <typename Cont, typename T>
	auto find(Cont & c, const T & t) -> decltype(std::begin(c)) {
		using std::begin, std::end;
		return std::find(begin(c), end(c), t);
	}

	// Find the first position `i` in `c` such that `p(*i)` is true.
	//
	// If no such position exists, `std::end(c)` is returned instead.
	template <typename Cont, typename Pred>
	auto find_if(Cont & c, Pred p) -> decltype(std::begin(c)) {
		using std::begin, std::end;
		return std::find_if(begin(c), end(c), p);
	}

	// Count the number of elements `x` in `c` such that `p(x)` is true.
	template <typename Cont, typename Pred>
	auto count_if(const Cont & c, Pred p) -> decltype(std::size(c)) {
		using std::begin, std::end;

		decltype(std::size(c)) n{0};

		for (auto i = begin(c); i != end(c); ++i) {
			if (p(*i)) ++n;
		}

		return n;
	}

	// Count the number of elements in `c` equal to `t`.
	template <typename Cont, typename T>
	auto count(const Cont & c, const T & t) -> decltype(std::size(c)) {
		auto eq_t = [&t](auto&& v) { return v == t; };
		return util::count_if(c, eq_t);
	}

	// Find the position of the largest element in `c` with respect to `<`.
	//
	// If `c` is empty, `std::end(c)` is returned instead.
	template <typename Cont>
	auto max_element(Cont & c) -> decltype(std::begin(c)) {
		using std::begin, std::end;
		return std::max_element(begin(c), end(c));
	}

	// Find the position of the largest element in `c` with respect to `lt`.
	//
	// If `c` is empty, `std::end(c)` is returned instead.
	template <typename Cont, typename Comp>
	auto max_element(Cont & c, Comp lt) -> decltype(std::begin(c)) {
		using std::begin, std::end;
		return std::max_element(begin(c), end(c), lt);
	}

	// Copy the elements of `cont` into `out`.
	template <typename Container, typename Iterator>
	void copy(Container & cont, Iterator out) {
		using std::begin, std::end;
		auto first = begin(cont);
		auto last = end(cont);
		std::copy(first, last, out);
	}

	// Replace every element in `cont` with a copy of `t`.
	template <typename Cont, typename T>
	void fill(Cont & cont, const T & t) {
		using std::begin, std::end;
		std::fill(begin(cont), end(cont), t);
	}

	// Fill `out` with the elements of `in` after being transformed by
	// `unary_op`.
	template <typename Container, typename OutIter, typename UnaryOperation>
	void transform(Container const& cont, OutIter out, UnaryOperation unary_op) {
		using std::begin, std::end;
		std::transform(begin(cont), end(cont), out, unary_op);
	}

	// Create a container of type `OutCont` filled with the elements of `cont`
	// after each has been transformed by `unary_op`.
	template <typename OutContainer,
	          typename InContainer,
	          typename UnaryOperation>
	auto transform_into(InContainer const& cont, UnaryOperation unary_op)
	-> OutContainer {
		OutContainer out{};
		util::transform(cont, std::back_inserter(out), unary_op);
		return out;
	}

	// Get a copy of `cont` where each element has been transformed by
	// `unary_op`.
	template <template <typename...> class Container,
	          typename InType,
	          typename UnaryOperation,
	          typename OutType = std::invoke_result_t<UnaryOperation, InType>,
	          typename... Xs>
	auto transformed_copy(Container<InType, Xs...> const& cont, UnaryOperation unary_op)
	-> Container<OutType> {
		return util::transform_into<Container<OutType>>(cont, unary_op);
	}

	// Get references to the elements of `cont`.
	template <template <typename...> class Container,
			  typename T,
			  typename... Xs>
	auto get_refs(Container<T, Xs...> const& cont)
	-> Container<std::reference_wrapper<T>> {
		auto get_ref = [](T& x) -> std::reference_wrapper<T> {
			return std::ref(x);
		};

		return util::transformed_copy(cont, get_ref);
	}

	// Get constant references to the elements of `cont`.
	template <template <typename...> class Container,
			  typename T,
			  typename... Xs>
	auto get_crefs(Container<T, Xs...> const& cont)
	-> Container<std::reference_wrapper<const T>> {
		auto get_cref = [](T const& x) -> std::reference_wrapper<const T> {
			return std::cref(x);
		};

		return util::transformed_copy(cont, get_cref);
	}

	// Remove each element from `c` that is equal to `t`.
	template <typename Cont, typename T>
	void remove(Cont & c, const T & t) {
		using std::begin, std::end;
		c.erase(std::remove(begin(c), end(c), t), end(c));
	}

	// Remove each element `t` from `c` for which `p(t)` is true.
	template <typename Cont, typename Pred>
	void remove_if(Cont & c, Pred p) {
		using std::begin, std::end;
		c.erase(std::remove_if(begin(c), end(c), p), end(c));
	}

	// Create a container of type `OutContainer` holding the elements `x` of
	// `cont` for which `pred(x)` is true.
	template <typename OutContainer,
			  typename InContainer,
			  typename Predicate>
	OutContainer filter_into(InContainer const& cont, Predicate pred) {
		using std::begin, std::end;
		OutContainer out{};
		std::copy_if(begin(cont), end(cont), std::back_inserter(out), pred);
		return out;
	}

	// Get the elements `x` of `cont` for which `pred(x)` is true.
	template <template <typename...> class Container,
			  typename T,
			  typename Predicate,
			  typename... Xs>
	auto filtered_copy(Container<T, Xs...> const& cont, Predicate pred)
	-> Container<T> {
		return util::filter_into<Container<T>>(cont, pred);
	}

	// Get references to the elements `x` of `cont` for which `pred(x)` is true.
	template <template <typename...> class Container,
			  typename T,
			  typename Predicate,
			  typename... Xs>
	auto filtered_refs(Container<T, Xs...> const& cont, Predicate pred)
	-> Container<std::reference_wrapper<T>> {
		using T_Ref = std::reference_wrapper<T>;
		return util::filter_into<Container<T_Ref>>(cont, pred);
	}

	// Get constant references to the elements `x` of `cont` for which `pred(x)`
	// is true.
	template <template <typename...> class Container,
			  typename T,
			  typename Predicate,
			  typename... Xs>
	auto filtered_crefs(Container<T, Xs...> const& cont, Predicate pred)
	-> Container<std::reference_wrapper<const T>> {
		using T_Ref = std::reference_wrapper<const T>;
		return util::filter_into<Container<T_Ref>>(cont, pred);
	}

	// Sort the elements in `c` by `<`.
	template <typename Cont>
	void sort(Cont & c) {
		using std::begin, std::end;
		std::sort(begin(c), end(c));
	}

	// Sort the elements in `c` by `lt`.
	template <typename Cont, typename Comp>
	void sort(Cont & c, Comp lt) {
		using std::begin, std::end;
		std::sort(begin(c), end(c), lt);
	}

	// Reverse the order of elements in `c`.
	template <typename Cont>
	void reverse(Cont & c) {
		using std::begin, std::end;
		std::reverse(begin(c), end(c));
	}

	// Randomise the order of elements in `[b,e)` using the default random
	// number generator.
	template <std::random_access_iterator Iter>
	void shuffle(Iter b, Iter e) {
		std::shuffle(b, e, random::default_generator);
	}

	// Randomise the order of elements in `range` using the default random
	// number generator.
	void shuffle(auto&& range) {
		using std::begin, std::end;
		std::shuffle(begin(range), end(range), random::default_generator);
	}
}

#endif
