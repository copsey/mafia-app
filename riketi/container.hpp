#ifndef RIKETI_CONTAINER
#define RIKETI_CONTAINER

#include <cstddef>

namespace rkt {
   /// Perform a crude check as to whether `A` is a container, and store the
   /// result in a wrapper.
   ///
   /// This is defined to be true precisely when `A` has associated value and
   /// iterator types.
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

   template <class T, std::size_t n>
   struct store_is_container<T[n]> {
      static constexpr bool value = true;
   };

   #if __cplusplus >= 201402L
      /// Perform a crude check as to whether `A` is a container.
      ///
      /// This is defined to be true precisely when `A` has associated value and
      /// iterator types.
      template <typename A>
      constexpr bool is_container = store_is_container<A>::value;
   #endif

   namespace container {
      template <typename Cont>
      struct _store_value_type {
         using type = typename Cont::value_type;
      };

      template <typename T, std::size_t n>
      struct _store_value_type<T[n]> {
         using type = T;
      };

      /// The value type associated with `Cont`.
      template <typename Cont>
      using value_type = typename _store_value_type<Cont>::type;

      template <class Cont>
      struct _store_size_type {
         using type = typename Cont::size_type;
      };

      template <class T, std::size_t n>
      struct _store_size_type<T[n]> {
         using type = std::size_t;
      };

      /// The size type associated with `Cont`.
      template <class Cont>
      using size_type = typename _store_size_type<Cont>::type;

      template <class Cont>
      struct _store_difference_type {
         using type = typename Cont::difference_type;
      };

      template <class T, std::size_t n>
      struct _store_difference_type<T[n]> {
         using type = std::ptrdiff_t;
      };

      /// The difference type associated with `Cont`.
      template <class Cont>
      using difference_type = typename _store_difference_type<Cont>::type;

      template <class Cont>
      struct _store_iterator_type {
         using type = typename Cont::iterator;
      };

      template <class T, std::size_t n>
      struct _store_iterator_type<T[n]> {
         using type = T *;
      };

      /// The iterator type associated with `Cont`.
      template <class Cont>
      using iterator_type = typename _store_iterator_type<Cont>::type;

      template <class Cont>
      struct _store_const_iterator_type {
         using type = typename Cont::const_iterator;
      };

      template <class T, std::size_t n>
      struct _store_const_iterator_type<T[n]> {
         using type = const T *;
      };

      /// The const iterator type associated with `Cont`.
      template <class Cont>
      using const_iterator_type = typename _store_const_iterator_type<Cont>::type;

      /// The number of elements that `c` contains.
      template <class Cont>
      size_type<Cont> size(const Cont & c) {
         return c.size();
      }

      /// The number of elements that `arr` contains.
      template <class T, std::size_t n>
      constexpr std::size_t size(T (& arr)[n]) {
         return n;
      }

      /// Check whether `c` is empty.
      template <class Cont>
      bool is_empty(const Cont & c) {
         return c.empty();
      }

      /// Check whether `arr` is empty.
      template <class T, std::size_t n>
      constexpr bool is_empty(T (& arr)[n]) {
         return n == 0;
      }
   }
}

#endif

#ifdef RIKETI_GENERIC
   #ifndef RIKETI_CONTAINER_GENERIC
   #define RIKETI_CONTAINER_GENERIC

   namespace rkt {
      template <class Cont>
      struct _store_value_type<Cont, enable_if<store_is_container<Cont>::value>> {
         using type = container::value_type<Cont>;
      };

      template <class Cont>
      struct _store_size_type<Cont, enable_if<store_is_container<Cont>::value>> {
         using type = container::size_type<Cont>;
      };

      template <class Cont>
      struct _store_difference_type<Cont, enable_if<store_is_container<Cont>::value>> {
         using type = container::difference_type<Cont>;
      };
   }

   #endif
#endif
