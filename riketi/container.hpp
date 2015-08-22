#ifndef RIKETI_CONTAINER
#define RIKETI_CONTAINER

namespace rkt {
   // Get the value type corresponding to Cont.
   template <class Cont>
   struct get_value_type {
      using type = typename Cont::value_type;
   };

   template <class T, std::size_t n>
   struct get_value_type<T[n]> {
      using type = T;
   };

   template <class Cont>
   using value_type = typename get_value_type<Cont>::type;



   // Get the size type corresponding to Cont.
   template <class Cont>
   struct get_size_type {
      using type = typename Cont::size_type;
   };

   template <class T, std::size_t n>
   struct get_size_type<T[n]> {
      using type = std::size_t;
   };

   template <class Cont>
   using size_type = typename get_size_type<Cont>::type;



   // Get the difference type corresponding to Cont.
   template <class Cont>
   struct get_difference_type {
      using type = typename Cont::difference_type;
   };

   template <class T, std::size_t n>
   struct get_difference_type<T[n]> {
      using type = std::ptrdiff_t;
   };

   template <class Cont>
   using difference_type = typename get_difference_type<Cont>::type;



   // Get the iterator type corresponding to Cont.
   template <class Cont>
   struct get_iterator {
      using type = typename Cont::iterator;
   };

   template <class T, std::size_t n>
   struct get_iterator<T[n]> {
      using type = T *;
   };

   template <class Cont>
   using iterator_type = typename get_iterator<Cont>::type;



   // Get the const iterator type corresponding to Cont.
   template <class Cont>
   struct get_const_iterator {
      using type = typename Cont::const_iterator;
   };

   template <class T, std::size_t n>
   struct get_const_iterator<T[n]> {
      using type = const T *;
   };

   template <class Cont>
   using const_iterator_type = typename get_const_iterator<Cont>::type;
}

#endif
