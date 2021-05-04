#ifndef MAFIA_UTIL_MAP_H
#define MAFIA_UTIL_MAP_H

#include <iterator>
#include <memory>

// A map is defined to be an associative container `M` with a key type
// `M::key_type` and an item type `M::mapped_type`. It is also assumed that
// `M::value_type` is `std::pair<const M::key_type, M::mapped_type>`.
//
// This concept has been modelled on `std::map`, and in general it can be
// assumed that any type reasonably similar in functionality to `std::map`
// qualifies as a map.

namespace maf::util {
	namespace map {
		// The key type associated with `Map`.
		template <typename Map>
		using key_type = typename Map::key_type;

		// The item type associated with `Map`.
		//
		// This is the same as `Map::mapped_type`.
		template <typename Map>
		using item_type = typename Map::mapped_type;
	}

	// An iterator that traverses the keys of a `Map`.
	// The reference to each key is `const`.
	template <typename Map>
	struct key_iterator {
		using map_type = Map;
		using map_iterator = typename map_type::iterator;
		using map_const_iterator = typename map_type::const_iterator;

		using value_type = map::key_type<map_type>;
		using difference_type = typename std::iterator_traits<map_iterator>::difference_type;
		using pointer = const value_type *;
		using reference = const value_type &;
		using iterator_category = typename std::iterator_traits<map_iterator>::iterator_category;

		key_iterator() = default;

		key_iterator(const map_iterator & p): _p{p} { }

		key_iterator(const map_const_iterator & p): _p{p} { }

		// A `map_const_iterator` pointing to the same entry in the
		// underlying map as `this`.
		operator map_const_iterator () const {
			return _p;
		}

		bool operator== (const key_iterator & x) const {
			return _p == x._p;
		}

		bool operator!= (const key_iterator & x) const {
			return _p != x._p;
		}

		key_iterator & operator++ () {
			++_p;
			return *this;
		}

		key_iterator operator++ (int) {
			return _p++;
		}

		key_iterator & operator-- () {
			--_p;
			return *this;
		}

		key_iterator operator-- (int) {
			return _p--;
		}

		reference operator* () const {
			return (*_p).first;
		}

		pointer operator-> () const {
			return std::addressof((*_p).first);
		}

	private:
		map_const_iterator _p{};
	};

	// Get a `key_iterator` pointing to the beginning of `map`.
	template <typename Map>
	key_iterator<Map> key_begin(const Map & map) {
		return key_iterator<Map>{std::begin(map)};
	}

	// Get a `key_iterator` pointing to the end of `map`.
	template <typename Map>
	key_iterator<Map> key_end(const Map & map) {
		return key_iterator<Map>{std::end(map)};
	}

	// An iterator that traverses the items of a `Map`.
	template <typename Map>
	struct item_iterator {
		using map_type = Map;
		using map_iterator = typename map_type::iterator;

		using value_type = map::item_type<map_type>;
		using difference_type = typename std::iterator_traits<map_iterator>::difference_type;
		using pointer = value_type *;
		using reference = value_type &;
		using iterator_category = typename std::iterator_traits<map_iterator>::iterator_category;

		item_iterator() = default;

		item_iterator(const map_iterator & p):
			_p{p}
		{ }

		// A `map_iterator` pointing to the same entry in the underlying map
		// as `this`.
		operator map_iterator () const {
			return _p;
		}

		bool operator== (const item_iterator & x) const {
			return _p == x._p;
		}

		bool operator!= (const item_iterator & x) const {
			return _p != x._p;
		}

		item_iterator & operator++ () {
			++_p;
			return *this;
		}

		item_iterator operator++ (int) {
			return _p++;
		}

		item_iterator & operator-- () {
			--_p;
			return *this;
		}

		item_iterator operator-- (int) {
			return _p--;
		}

		reference operator* () const {
			return (*_p).second;
		}

		pointer operator-> () const {
			return std::addressof((*_p).second);
		}

	private:
		map_iterator _p{};
	};

	// An iterator that traverses the items of a `Map`.
	// The reference to each item is `const`.
	template <typename Map>
	struct const_item_iterator {
		using map_type = Map;
		using map_iterator = typename map_type::iterator;
		using map_const_iterator = typename map_type::const_iterator;

		using value_type = map::item_type<map_type>;
		using difference_type = typename std::iterator_traits<map_const_iterator>::difference_type;
		using pointer = const value_type *;
		using reference = const value_type &;
		using iterator_category = typename std::iterator_traits<map_const_iterator>::iterator_category;

		const_item_iterator() = default;

		const_item_iterator(const item_iterator<map_type> & x):
			_p{static_cast<map_iterator>(x)}
		{ }

		const_item_iterator(const map_iterator & p):
			_p{p}
		{ }

		const_item_iterator(const map_const_iterator & p):
			_p{p}
		{ }

		// A `map_const_iterator` pointing to the same entry in the
		// underlying map as `this`.
		operator map_const_iterator () const {
			return _p;
		}

		bool operator== (const const_item_iterator & x) const {
			return _p == x._p;
		}

		bool operator!= (const const_item_iterator & x) const {
			return _p != x._p;
		}

		const_item_iterator & operator++ () {
			++_p;
			return *this;
		}

		const_item_iterator operator++ (int) {
			return _p++;
		}

		const_item_iterator & operator-- () {
			--_p;
			return *this;
		}

		const_item_iterator operator-- (int) {
			return _p--;
		}

		reference operator* () const {
			return (*_p).second;
		}

		pointer operator-> () const {
			return std::addressof((*_p).second);
		}

	private:
		map_const_iterator _p{};
	};

	// Get an `item_iterator` pointing to the beginning of `map`.
	template <typename Map>
	item_iterator<Map> item_begin(Map & map) {
		return item_iterator<Map>{map.begin()};
	}

	// Get a `const_item_iterator` pointing to the beginning of `map`.
	template <typename Map>
	const_item_iterator<Map> item_begin(const Map & map) {
		return const_item_iterator<Map>{map.begin()};
	}

	// Get an `item_iterator` pointing to the end of `map`.
	template <typename Map>
	item_iterator<Map> item_end(Map & map) {
		return item_iterator<Map>{map.end()};
	}

	// Get a `const_item_iterator` pointing to the end of `map`.
	template <typename Map>
	const_item_iterator<Map> item_end(const Map & map) {
		return const_item_iterator<Map>{map.end()};
	}
}

#endif
