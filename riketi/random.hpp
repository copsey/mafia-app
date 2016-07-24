#ifndef RIKETI_RANDOM
#define RIKETI_RANDOM

#include <algorithm>
#include <iterator>
#include <random>

#include "container.hpp"

namespace rkt {
   /// A mt19937 used by various algorithms, which is automatically seeded when
   /// the program first starts.
   extern std::mt19937 random_engine;

   /// Pick a random position in [b,e) using g.
   ///
   /// If b == e, e is returned instead.
   template <class Iter, class RNG>
   Iter pick_with (Iter b, Iter e, RNG && g) {
      if (b == e) return e;

      auto n = std::distance(b, e);
      std::uniform_int_distribution<decltype(n)> dist{0, n - 1};
      std::advance(b, dist(g));
      return b;
   }

   /// Pick a random position in c using g.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont, class RNG>
   container::iterator_type<Cont> pick_with (Cont & c, RNG && g) {
      return pick_with(std::begin(c), std::end(c), g);
   }

   /// Pick a random position in c using g.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont, class RNG>
   container::const_iterator_type<Cont> pick_with (const Cont & c, RNG && g) {
      return pick_with(std::begin(c), std::end(c), g);
   }

   /// Pick a random position in [b,e) using rkt::random_engine.
   ///
   /// If b == e, e is returned instead.
   template <class Iter>
   Iter pick (Iter b, Iter e) {
      return pick_with(b, e, random_engine);
   }

   /// Pick a random position in c using rkt::random_engine.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont>
   container::iterator_type<Cont> pick (Cont & c) {
      return pick(std::begin(c), std::end(c));
   }

   /// Pick a random position in c using rkt::random_engine.
   ///
   /// If c is empty, std::end(c) is returned instead.
   template <class Cont>
   container::const_iterator_type<Cont> pick (const Cont & c) {
      return pick(std::begin(c), std::end(c));
   }

   /// Shuffle c using g.
   template <class Cont, class RNG>
   void shuffle_with (Cont & c, RNG && g) {
      std::shuffle(std::begin(c), std::end(c), g);
   }

   /// Shuffle [b,e) using rkt::random_engine.
   template <class Iter>
   void shuffle (Iter b, Iter e) {
      std::shuffle(b, e, random_engine);
   }

   /// Shuffle c using rkt::random_engine.
   template <class Cont>
   void shuffle (Cont & c) {
      std::shuffle(std::begin(c), std::end(c), random_engine);
   }
}

#endif
