#ifndef RIKETI_ITERATOR
#define RIKETI_ITERATOR

#include <iterator>

namespace rkt {
	namespace iterator {
		// The value type associated with `Iter`.
		template <typename Iter>
		using value_type = typename std::iterator_traits<Iter>::value_type;
		
		// The difference type associated with `Iter`.
		template <typename Iter>
		using difference_type = typename std::iterator_traits<Iter>::difference_type;
		
		// The iterator category of `Iter`.
		template <typename Iter>
		using category = typename std::iterator_traits<Iter>::iterator_category;
	}
	
	// Check if `A` is an iterator, and store the result as `value`.
	//
	// This test is rather crude. `A` is defined to be an iterator
	// precisely when it has an associated iterator category.
	template <typename A>
	class store_is_iterator {
		template <typename B>
		static constexpr bool test(iterator::category<B> *) {
			return true;
		}
		
		template <typename B>
		static constexpr bool test(...) {
			return false;
		}
		
	public:
		static constexpr bool value = test<A>(nullptr);
	};
	
	#if __cplusplus >= 201402L
		// Check if `A` is an iterator.
		//
		// This test is rather crude. `A` is defined to be an iterator
		// precisely when it has an associated iterator category.
		template <typename A>
		constexpr bool is_iterator = store_is_iterator<A>::value;
	#endif
}

#endif

#ifdef RIKETI_GENERIC
	#ifndef RIKETI_ITERATOR_GENERIC
	#define RIKETI_ITERATOR_GENERIC
	
	namespace rkt {
		template <typename Iter>
		struct _store_value_type<Iter, enable_if<store_is_iterator<Iter>::value>> {
			using type = iterator::value_type<Iter>;
		};
		
		template <typename Iter>
		struct _store_difference_type<Iter, enable_if<store_is_iterator<Iter>::value>> {
			using type = iterator::difference_type<Iter>;
		};
	}
	
	#endif
#endif
