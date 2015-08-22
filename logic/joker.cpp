#include <algorithm>

#include "../riketi/map.hpp"
#include "../riketi/random.hpp"

#include "joker.hpp"
#include "rulebook.hpp"

mafia::Joker::Joker(ID id, std::map<Role::ID, double> weights)
: _id{id} {
   using Key_iterator = rkt::map::key_iterator<Role::ID, double>;
   using Item_iterator = rkt::map::item_const_iterator<Role::ID, double>;

   if (std::any_of(Item_iterator{weights.begin()}, Item_iterator{weights.end()},
                   [](double w) { return w < 0.0; })) {
      // fix-me: throw exception, weights must be non-negative
   }

   if (std::all_of(Item_iterator{weights.begin()}, Item_iterator{weights.end()},
                   [](double w) { return w == 0.0; } )) {
      // fix-me: throw exception, at least one weight must be positive
   }

   _role_ids = {Key_iterator{weights.begin()}, Key_iterator{weights.end()}};
   _dist = {Item_iterator{weights.begin()}, Item_iterator{weights.end()}};
}

mafia::Role & mafia::Joker::choose_role(Rulebook &rulebook) {
   if (_evaluator) {
      std::vector<Role *> role_ptrs{};
      std::vector<double> weights{};

      rulebook.for_roles([this, &role_ptrs, &weights](Role &role) {
         double w = _evaluator(role);
         if (w < 0.0) {
            // fix-me: throw exception, weights must be non-negative
         } else if (w > 0.0) {
            role_ptrs.push_back(&role);
            weights.push_back(w);
         }
      });

      if (role_ptrs.size() == 0) {
         // fix-me: throw exception, at least one weight must be positive
      }

      std::discrete_distribution<std::vector<Role *>::size_type> dist{
         weights.begin(), weights.end()
      };
      return *role_ptrs[dist(rkt::random_engine)];
   } else {
      return rulebook.get_role(_role_ids[_dist(rkt::random_engine)]);
   }
}
