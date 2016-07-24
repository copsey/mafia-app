#ifndef RIKETI_ALGORITHM
#define RIKETI_ALGORITHM

#include <algorithm>

#include "iterator.hpp"
#include "container.hpp"

namespace rkt {
   /// Check whether p(t) is true for all t in c.
   template <class Cont, class Pred>
   bool all_of (const Cont & c, Pred p) {
      return std::all_of(std::begin(c), std::end(c), p);
   }

   /// Check whether p(t) is true for at least one t in c.
   template <class Cont, class Pred>
   bool any_of (const Cont & c, Pred p) {
      return std::any_of(std::begin(c), std::end(c), p);
   }

   /// Check whether p(t) is false for all t in c.
   template <class Cont, class Pred>
   bool none_of (const Cont & c, Pred p) {
      return std::none_of(std::begin(c), std::end(c), p);
   }

   /// Check whether t is equal to any values in the range [i,j).
   template <class Iter, class T>
   bool contains (Iter i, Iter j, const T & t) {
      for ( ; i != j; ++i) {
         if (*i == t) return true;
      }

      return false;
   }

   /// Check whether t is equal to any elements in c.
   template <class Cont, class T>
   bool contains (const Cont & c, const T & t) {
      return contains(std::begin(c), std::end(c), t);
   }

   /// Check whether the two sequences [i1,j1) and [i2,j2) match, according to
   /// eq.
   ///
   /// This is defined to be true precisely when both sequences have the same
   /// length, and every corresponding pair of values t1 and t2 are such that
   /// eq(t1,t2) is true.
   template <class Iter1, class Iter2, class Eq>
   bool matches (Iter1 i1, Iter1 j1, Iter2 i2, Iter2 j2, Eq eq) {
      for ( ; i1 != j1; ++i1, ++i2) {
         if (i2 == j2) return false;
         if (!eq(*i1, *i2)) return false;
      }

      return i2 == j2;
   }

   /// Check whether c1 and c2 match, according to eq.
   ///
   /// This is defined to be true precisely when both containers are of the same
   /// size, and for every corresponding pair of values t1 in c1 and t2 in c2,
   /// eq(t1, t2) is true.
   template <class Cont1, class Cont2, class Eq>
   bool matches (const Cont1 & c1, const Cont2 & c2, Eq eq) {
      return matches(std::begin(c1), std::end(c1), std::begin(c2), std::end(c2), eq);
   }

   /// Check whether the two sequences [i1,j1) and [i2,j2) are equivalent.
   ///
   /// This is defined to be true precisely when both sequences have the same
   /// length, and every corresponding pair of values are equal.
   template <class Iter1, class Iter2>
   bool equivalent (Iter1 i1, Iter1 j1, Iter2 i2, Iter2 j2) {
      using T1 = iterator::value_type<Iter1>;
      using T2 = iterator::value_type<Iter2>;

      auto eq = [](const T1 & t1, const T2 & t2) { return t1 == t2; };
      return matches(i1, j1, i2, j2, eq);
   }

   /// Check whether c1 and c2 are equivalent.
   ///
   /// This is defined to be true precisely when both containers are of the same
   /// size, and have equal contents with the same ordering.
   template <class Cont1, class Cont2>
   bool equivalent (const Cont1 & c1, const Cont2 & c2) {
      return equivalent(std::begin(c1), std::end(c1), std::begin(c2), std::end(c2));
   }

   /// Find the first position i in c such that *i == t.
   ///
   /// If no such position exists, std::end(c) is returned instead.
   template <class Cont, class T>
   container::iterator_type<Cont> find (Cont & c, const T & t) {
      return std::find(std::begin(c), std::end(c), t);
   }

   /// Find the first position i in c such that *i == t.
   ///
   /// If no such position exists, std::end(c) is returned instead.
   template <class Cont, class T>
   container::const_iterator_type<Cont> find (const Cont & c, const T & t) {
      return std::find(std::begin(c), std::end(c), t);
   }

   /// Find the first position i in c such that p(*i) is true.
   ///
   /// If no such position exists, std::end(c) is returned instead.
   template <class Cont, class Pred>
   container::iterator_type<Cont> find_if (Cont & c, Pred p) {
      return std::find_if(std::begin(c), std::end(c), p);
   }

   /// Find the first position i in c such that p(*i) is true.
   ///
   /// If no such position exists, std::end(c) is returned instead.
   template <class Cont, class Pred>
   container::const_iterator_type<Cont> find_if (const Cont & c, Pred p) {
      return std::find_if(std::begin(c), std::end(c), p);
   }

   /// Count the number of elements t in c such that p(t) is true.
   template <class Cont, class Pred>
   container::size_type<Cont> count_if (const Cont & c, Pred p) {
      container::size_type<Cont> n = 0;

      auto end = std::end(c);
      for (auto i = std::begin(c); i != end; ++i) {
         if (p(*i)) ++n;
      }

      return n;
   }

   /// Count the number of elements of c that are equal to t.
   template <class Cont, class T>
   container::size_type<Cont> count (const Cont & c, const T & t) {
      using V = container::value_type<Cont>;
      auto eq_t = [&t](const V & v) { return v == t; };
      return count_if(c, eq_t);
   }

   /// Find the position of the largest element of c with respect to <.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont>
   container::iterator_type<Cont> max_element (Cont & c) {
      return std::max_element(std::begin(c), std::end(c));
   }

   /// Find the position of the largest element of c with respect to <.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont>
   container::const_iterator_type<Cont> max_element (const Cont & c) {
      return std::max_element(std::begin(c), std::end(c));
   }

   /// Find the position of the largest element of c with respect to lt.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont, class Comp>
   container::iterator_type<Cont> max_element (Cont & c, Comp lt) {
      return std::max_element(std::begin(c), std::end(c), lt);
   }

   /// Find the position of the largest element of c with respect to lt.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont, class Comp>
   container::const_iterator_type<Cont> max_element (const Cont & c, Comp lt) {
      return std::max_element(std::begin(c), std::end(c), lt);
   }

   /// Replace every element of c with a copy of t.
   template <class Cont, class T>
   void fill (Cont & c, const T & t) {
      std::fill(std::begin(c), std::end(c), t);
   }

   /// Remove every element of c that is equal to t.
   template <class Cont, class T>
   void remove (Cont & c, const T & t) {
      c.erase(std::remove(std::begin(c), std::end(c), t), std::end(c));
   }

   /// Remove every element t of c such that p(t) == true.
   template <class Cont, class Pred>
   void remove_if (Cont & c, Pred p) {
      c.erase(std::remove_if(std::begin(c), std::end(c), p), std::end(c));
   }

   /// Sort the elements of c by <.
   template <class Cont>
   void sort (Cont & c) {
      std::sort(std::begin(c), std::end(c));
   }

   /// Sort the elements of c by lt.
   template <class Cont, class Comp>
   void sort (Cont & c, Comp lt) {
      std::sort(std::begin(c), std::end(c), lt);
   }

   /// Reverse the order of elements in c.
   template <class Cont>
   void reverse (Cont & c) {
      std::reverse(std::begin(c), std::end(c));
   }

   /// Generate the next permutation of c, using lexicographical ordering with
   /// respect to <.
   ///
   /// @returns true if the new permutation is lexicographically greater than
   /// the old permutation, and false if c has wrapped back to its initial
   /// permutation.
   template <class Cont>
   bool next_permutation (Cont & c) {
      return std::next_permutation(std::begin(c), std::end(c));
   }

   /// Generate the next permutation of c, using lexicographical ordering with
   /// respect to lt.
   ///
   /// @returns true if the new permutation is lexicographically greater than
   /// the old permutation, and false if c has wrapped back to its initial
   /// permutation.
   template <class Cont, class Comp>
   bool next_permutation (Cont & c, Comp lt) {
      return std::next_permutation(std::begin(c), std::end(c), lt);
   }
}

#endif
