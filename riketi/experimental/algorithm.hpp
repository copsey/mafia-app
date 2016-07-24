#ifndef RIKETI_EXPERIMENTAL_ALGORITHM
#define RIKETI_EXPERIMENTAL_ALGORITHM

#include <iterator>
#include <vector>

#include "../container.hpp"

namespace rkt {
   /// Split the container `c` into a vector of subcontainers.
   ///
   /// Each time an element `t` of `c` is such that `p(t)` is true,
   /// a new subcontainer is added to the vector, with `t` excluded.
   ///
   /// The vector may contain empty subcontainers.
   ///
   /// Example: `split_if("d78agf3qf3",isdigit)` gives `{"d","","agf","qf",""}`.
   template <class Cont, class Pred>
   std::vector<Cont> split_if(const Cont & c, Pred p) {
      std::vector<Cont> v;

      auto i = std::begin(c), j = i, end = std::end(c);

      while (j != end) {
         if (p(*j)) {
            v.emplace_back(i, j);
            i = ++j;
         } else {
            ++j;
         }
      }

      v.emplace_back(i, j);

      return v;
   }

   /// Split the container `c` into a vector of subcontainers.
   ///
   /// Each time the delimiter `t` appears, a new subcontainer is
   /// added to the vector, with `t` excluded.
   ///
   /// The vector may contain empty subcontainers.
   ///
   /// Example: `split("aabbcdabda",'b')` gives `{"aa","","cda","da"}`.
   template <class Cont, class T>
   std::vector<Cont> split(const Cont & c, const T & t) {
      using V = container::value_type<Cont>;

      auto eq_t = [&t](const V & v) { return v == t; };
      return split_if(c, eq_t);
   }

   /// Split the container `c` into a vector of subcontainers.
   ///
   /// Each time an element `t` of `c` is such that `p(t)` is true,
   /// a new subcontainer is added to the vector, with `t` excluded.
   ///
   /// Empty subcontainers are automatically removed.
   ///
   /// Example: `split_if_and_prune("8dfug98a",isdigit)` gives `{"dfug","a"}`.
   template <class Cont, class Pred>
   std::vector<Cont> split_if_and_prune(const Cont & c, Pred p) {
      std::vector<Cont> v;

      auto i = std::begin(c), j = i, end = std::end(c);

      while (j != end) {
         if (p(*j)) {
            if (i != j) v.emplace_back(i, j);
            i = ++j;
         } else {
            ++j;
         }
      }

      if (i != j) v.emplace_back(i, j);

      return v;
   }

   /// Split the container `c` into a vector of subcontainers.
   ///
   /// Each time the delimiter `t` appears, a new subcontainer is
   /// added to the vector, with `t` excluded.
   ///
   /// Empty subcontainers are automatically removed.
   ///
   /// Example: `split_and_prune("bbacdabdca",'b')` gives `{"acda","dca"}`.
   template <class Cont, class T>
   std::vector<Cont> split_and_prune(const Cont & c, const T & t) {
      using V = container::value_type<Cont>;

      auto eq_t = [&t](const V & v) { return v == t; };
      return split_if_and_prune(c, eq_t);
   }
}

#endif
