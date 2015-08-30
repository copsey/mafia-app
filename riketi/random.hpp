#ifndef RIKETI_RANDOM
#define RIKETI_RANDOM

#include <algorithm>
#include <iterator>
#include <random>

#include "container.hpp"

namespace rkt {
   // A random number engine used by various algorithms.
   // Automatically seeded when the program starts.
   extern std::default_random_engine random_engine;

   // Pick a random position in [b,e) using g.
   // Returns e if b == e.
   template <class Iter, class RNG>
   Iter pick_with(Iter b, Iter e, RNG &&g) {
      if (b == e) return e;

      auto n = std::distance(b, e);
      std::uniform_int_distribution<decltype(n)> uid{0, n - 1};
      std::advance(b, uid(g));
      return b;
   }

   // Pick a random position in c using g.
   // Returns std::end(c) if c has size 0.
   template <class Cont, class RNG>
   iterator_type<Cont> pick_with(Cont &c, RNG &&g) {
      return pick_with(std::begin(c), std::end(c), g);
   }
   template <class Cont, class RNG>
   const_iterator_type<Cont> pick_with(const Cont &c, RNG &&g) {
      return pick_with(std::begin(c), std::end(c), g);
   }

   // Pick a random position in [b,e) using random_engine.
   // Returns e if b == e.
   template <class Iter>
   Iter pick(Iter b, Iter e) {
      return rkt::pick_with(b, e, random_engine);
   }

   // Pick a random position in c using random_engine.
   // Returns std::end(c) if c has size 0.
   template <class Cont>
   iterator_type<Cont> pick(Cont &c) {
      return rkt::pick_with(c, random_engine);
   }
   template <class Cont>
   const_iterator_type<Cont> pick(const Cont &c) {
      return rkt::pick_with(c, random_engine);
   }

   // Shuffle [b,e) using g.
   template <class Iter, class RNG>
   void shuffle_with(Iter b, Iter e, RNG &&g) {
      std::shuffle(b, e, g);
   }

   // Shuffle c using g.
   template <class Cont, class RNG>
   void shuffle_with(Cont &c, RNG &&g) {
      std::shuffle(std::begin(c), std::end(c), g);
   }

   // Shuffle [b,e) using random_engine.
   template <class Iter>
   void shuffle(Iter b, Iter e) {
      std::shuffle(b, e, random_engine);
   }

   // Shuffle c using random_engine.
   template <class Cont>
   void shuffle(Cont &c) {
      std::shuffle(std::begin(c), std::end(c), random_engine);
   }
}

#endif
