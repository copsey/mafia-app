#ifndef RIKETI_CONTAINER
#define RIKETI_CONTAINER

#include <cstddef>
#include <type_traits>

namespace rkt {
	namespace container {
		template <typename Cont>
		struct _store_value_type {
			using type = typename Cont::value_type;
		};
		
		template <typename T, std::size_t n>
		struct _store_value_type<T[n]> {
			using type = T;
		};
		
		// The value type associated with `Cont`.
		template <typename Cont>
		using value_type = typename _store_value_type<Cont>::type;
		
		template <typename Cont>
		struct _store_size_type {
			using type = typename Cont::size_type;
		};
		
		template <typename T, std::size_t n>
		struct _store_size_type<T[n]> {
			using type = std::size_t;
		};
		
		// The size type associated with `Cont`.
		template <typename Cont>
		using size_type = typename _store_size_type<Cont>::type;
		
		template <typename Cont>
		struct _store_difference_type {
			using type = typename Cont::difference_type;
		};
		
		template <typename T, std::size_t n>
		struct _store_difference_type<T[n]> {
			using type = std::ptrdiff_t;
		};
		
		// The difference type associated with `Cont`.
		template <typename Cont>
		using difference_type = typename _store_difference_type<Cont>::type;
		
		template <typename Cont>
		struct _store_iterator_type {
			using type = typename Cont::iterator;
		};
		
		template <typename T, std::size_t n>
		struct _store_iterator_type<T[n]> {
			using type = T *;
		};
		
		// The iterator type associated with `Cont`.
		template <typename Cont>
		using iterator_type = typename _store_iterator_type<Cont>::type;
		
		template <typename Cont>
		struct _store_const_iterator_type {
			using type = typename Cont::const_iterator;
		};
		
		template <typename T, std::size_t n>
		struct _store_const_iterator_type<T[n]> {
			using type = const T *;
		};
		
		// The const iterator type associated with `Cont`.
		template <typename Cont>
		using const_iterator_type = typename _store_const_iterator_type<Cont>::type;
		
		// The number of elements that `c` contains.
		template <typename Cont>
		[[deprecated("use std::size instead")]]
		size_type<Cont> size(const Cont & c) {
			return c.size();
		}
		
		template <typename T, std::size_t n>
		[[deprecated("use std::size instead")]]
		constexpr std::size_t size(T (& arr)[n]) {
			return n;
		}
		
		// Check if `c` is empty.
		template <typename Cont>
		[[deprecated("use std::empty instead")]]
		bool is_empty(const Cont & c) {
			return c.empty();
		}
		
		template <typename T, std::size_t n>
		[[deprecated("use std::empty instead")]]
		constexpr bool is_empty(T (& arr)[n]) {
			return n == 0;
		}
	}
	
	// Check if `A` is a container, and store the result as `value`.
	//
	// This test is rather crude. `A` is defined to be a container
	// precisely when it has associated value and iterator types.
	template <typename A>
	class store_is_container {
		template <typename B>
		static constexpr bool test(typename B::value_type *, typename B::iterator *) {
			return true;
		}
		
		template <typename>
		static constexpr bool test(...) {
			return false;
		}
		
	public:
		static constexpr bool value = test<A>(nullptr, nullptr);
	};
	
	template <typename T, std::size_t n>
	struct store_is_container<T[n]> {
		static constexpr bool value = true;
	};
	
	// Check if `A` is a container.
	//
	// This test is rather crude. `A` is defined to be a container
	// precisely when it has associated value and iterator types.
	template <typename A>
	constexpr bool is_container = store_is_container<A>::value;
}

#endif

#ifdef RIKETI_GENERIC
	#ifndef RIKETI_CONTAINER_GENERIC
	#define RIKETI_CONTAINER_GENERIC
	
	namespace rkt {
		template <typename Cont>
		struct _store_value_type<Cont, std::enable_if_t<store_is_container<Cont>::value>> {
			using type = container::value_type<Cont>;
		};
		
		template <typename Cont>
		struct _store_size_type<Cont, std::enable_if_t<store_is_container<Cont>::value>> {
			using type = container::size_type<Cont>;
		};
		
		template <typename Cont>
		struct _store_difference_type<Cont, std::enable_if_t<store_is_container<Cont>::value>> {
			using type = container::difference_type<Cont>;
		};
	}
	
	#endif
#endif
