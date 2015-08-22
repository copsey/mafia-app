#ifndef RIKETI_ALGORITHM
#define RIKETI_ALGORITHM

#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>

#include "container.hpp"

namespace rkt {
   // Split the container c into a vector of subcontainers.
   // Each time the delimiter t appears, a new subcontainer is added to the
   // vector, with t excluded.
   // The vector may contain empty subcontainers.
   // Example: split("aabbcdabda",'b') == {"aa","","cda","da"}.
   template <class Cont, class T>
   std::vector<Cont> split(const Cont &c, const T &t) {
      std::vector<Cont> v{};

      auto it1 = std::begin(c), it2 = it1, end = std::end(c);
      while (it2 != end) {
         if (*it2 == t) {
            v.emplace_back(it1, it2);
            it1 = ++it2;
         } else {
            ++it2;
         }
      }
      v.emplace_back(it1, it2);

      return v;
   }

   // Split the container c into a vector of subcontainers.
   // Each time an element t of c satisfies p(t) == true, a new subcontainer is
   // added to the vector, with t excluded.
   // The vector may contain empty subcontainers.
   // Example: split_if("d78agf3qf3",isdigit) == {"d","","agf","qf",""}.
   template <class Cont, class Pred>
   std::vector<Cont> split_if(const Cont &c, Pred p) {
      std::vector<Cont> v{};

      auto it1 = std::begin(c), it2 = it1, end = std::end(c);
      while (it2 != end) {
         if (p(*it2)) {
            v.emplace_back(it1, it2);
            it1 = ++it2;
         } else {
            ++it2;
         }
      }
      v.emplace_back(it1, it2);

      return v;
   }

   // Split the container c into a vector of subcontainers.
   // Each time the delimiter t appears, a new subcontainer is added to the
   // vector, with t excluded.
   // Empty subcontainers are automatically removed.
   // Example: split_and_prune("bbacdabdca",'b') == {"acda","dca"}.
   template <class Cont, class T>
   std::vector<Cont> split_and_prune(const Cont &c, const T &t) {
      std::vector<Cont> v{};

      auto it1 = std::begin(c), it2 = it1, end = std::end(c);
      while (it2 != end) {
         if (*it2 == t) {
            if (it1 != it2) v.emplace_back(it1, it2);
            it1 = ++it2;
         } else {
            ++it2;
         }
      }
      if (it1 != it2) v.emplace_back(it1, it2);

      return v;
   }

   // Split the container c into a vector of subcontainers.
   // Each time an element t of c satisfies p(t) == true, a new subcontainer is
   // added to the vector, with t excluded.
   // Empty subcontainers are automatically removed.
   // Example: split_if_and_prune("8dfug98a",isdigit) == {"dfug","a"}.
   template <class Cont, class Pred>
   std::vector<Cont> split_if_and_prune(const Cont &c, Pred p) {
      std::vector<Cont> v{};

      auto it1 = std::begin(c), it2 = it1, end = std::end(c);
      while (it2 != end) {
         if (p(*it2)) {
            if (it1 != it2) v.emplace_back(it1, it2);
            it1 = ++it2;
         } else {
            ++it2;
         }
      }
      if (it1 != it2) v.emplace_back(it1, it2);

      return v;
   }

   template <class Cont, class Pred>
   bool all_of(const Cont &c, Pred p) {
      return std::all_of(std::begin(c), std::end(c), p);
   }

   template <class Cont, class Pred>
   bool any_of(const Cont &c, Pred p) {
      return std::any_of(std::begin(c), std::end(c), p);
   }

   template <class Cont, class Pred>
   bool none_of(const Cont &c, Pred p) {
      return std::none_of(std::begin(c), std::end(c), p);
   }

   template <class Cont1, class Cont2>
   bool equal(const Cont1 &c1, const Cont2 &c2) {
      return std::equal(
         std::begin(c1), std::end(c1),
         std::begin(c2), std::end(c2)
      );
   }

   template <class Cont1, class Cont2, class Pred>
   bool equal(const Cont1 &c1, const Cont2 &c2, Pred p) {
      return std::equal(
         std::begin(c1), std::end(c1),
         std::begin(c2), std::end(c2),
         p
      );
   }

   template <class Cont, class T>
   iterator_type<Cont> find(Cont &c, const T &t) {
      return std::find(std::begin(c), std::end(c), t);
   }

   template <class Cont, class T>
   const_iterator_type<Cont> find(const Cont &c, const T &t) {
      return std::find(std::begin(c), std::end(c), t);
   }

   template <class Cont, class Pred>
   iterator_type<Cont> find_if(Cont &c, Pred p) {
      return std::find_if(std::begin(c), std::end(c), p);
   }

   template <class Cont, class Pred>
   const_iterator_type<Cont> find_if(const Cont &c, Pred p) {
      return std::find_if(std::begin(c), std::end(c), p);
   }

   template <class Cont, class Pred>
   iterator_type<Cont> find_if_not(Cont &c, Pred p) {
      return std::find_if_not(std::begin(c), std::end(c), p);
   }

   template <class Cont, class Pred>
   const_iterator_type<Cont> find_if_not(const Cont &c, Pred p) {
      return std::find_if_not(std::begin(c), std::end(c), p);
   }

   template <class Cont, class T>
   difference_type<Cont> count(const Cont &c, const T &t) {
      return std::count(std::begin(c), std::end(c), t);
   }

   template <class Cont, class Pred>
   difference_type<Cont> count_if(const Cont &c, Pred p) {
      return std::count_if(std::begin(c), std::end(c), p);
   }

   template <class Cont>
   iterator_type<Cont> max_element(Cont &c) {
      return std::max_element(std::begin(c), std::end(c));
   }

   template <class Cont>
   const_iterator_type<Cont> max_element(const Cont &c) {
      return std::max_element(std::begin(c), std::end(c));
   }

   template <class Cont, class Comp>
   iterator_type<Cont> max_element(Cont &c, Comp lt) {
      return std::max_element(std::begin(c), std::end(c), lt);
   }

   template <class Cont, class Comp>
   const_iterator_type<Cont> max_element(const Cont &c, Comp lt) {
      return std::max_element(std::begin(c), std::end(c), lt);
   }

   template <class Cont>
   void reverse(Cont &c) {
      std::reverse(std::begin(c), std::end(c));
   }

   template <class Cont>
   void sort(Cont &c) {
      std::sort(std::begin(c), std::end(c));
   }

   template <class Cont, class Comp>
   void sort(Cont &c, Comp lt) {
      std::sort(std::begin(c), std::end(c), lt);
   }

   template <class Cont>
   bool next_permutation(Cont &c) {
      return std::next_permutation(std::begin(c), std::end(c));
   }

   template <class Cont, class Comp>
   bool next_permutation(Cont &c, Comp lt) {
      return std::next_permutation(std::begin(c), std::end(c), lt);
   }

   template <class Cont>
   value_type<Cont> accumulate(const Cont &c) {
      using T = value_type<Cont>;
      return std::accumulate(std::begin(c), std::end(c), T{0});
   }

   template <class Cont, class T>
   T accumulate(const Cont &c, T t0) {
      return std::accumulate(std::begin(c), std::end(c), t0);
   }

   template <class Cont, class T, class BinOp>
   T accumulate(const Cont &c, T t0, BinOp op) {
      return std::accumulate(std::begin(c), std::end(c), t0, op);
   }

   // Fill c with successive values, starting at 0.
   template <class Cont>
   void iota(Cont &c) {
      using T = value_type<Cont>;
      std::iota(std::begin(c), std::end(c), T{0});
   }

   // Fill c with successive values, starting at t0.
   template <class Cont, class T>
   void iota(Cont &c, T t0) {
      std::iota(std::begin(c), std::end(c), t0);
   }
}

#endif
