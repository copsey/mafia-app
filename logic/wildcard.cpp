#include <sstream>
#include <stdexcept>

#include "../riketi/map.hpp"
#include "../riketi/random.hpp"

#include "wildcard.hpp"
#include "rulebook.hpp"

maf::Wildcard::Wildcard(ID id, const std::map<Role::ID, double> &weights)
: _id{id} {
   using Key_iterator = rkt::map::key_iterator<Role::ID, double>;
   using Item_iterator = rkt::map::item_const_iterator<Role::ID, double>;

   if (std::any_of(Item_iterator{weights.begin()}, Item_iterator{weights.end()},
                   [](double w) { return w < 0.0; })) {
      std::ostringstream err{};
      err << "A joker with alias "
      << alias()
      << " was created with a negative role weight.";

      throw std::invalid_argument{err.str()};
   }

   if (std::all_of(Item_iterator{weights.begin()}, Item_iterator{weights.end()},
                   [](double w) { return w == 0.0; } )) {
      std::ostringstream err{};
      err << "A joker with alias "
      << alias()
      << " was created with every role weight set to zero.";

      throw std::invalid_argument{err.str()};
   }

   _role_ids = {Key_iterator{weights.begin()}, Key_iterator{weights.end()}};
   _dist = {Item_iterator{weights.begin()}, Item_iterator{weights.end()}};
}

std::string maf::Wildcard::alias() const {
   return maf::alias(_id);
}

bool maf::Wildcard::matches_alignment(Alignment alignment, const Rulebook &rulebook) const {
   if (uses_evaluator()) {
      for (const Role &r: rulebook.roles()) {
         if (r.alignment != alignment) {
            double w = _evaluator(r);
            if (w > 0.0) return false;
         }
      }

      return true;
   } else {
      std::vector<double> probabilities = _dist.probabilities();

      for (std::size_t i{0}; i < _role_ids.size(); ++i) {
         const Role &r = rulebook.get_role(_role_ids[i]);
         if (r.alignment != alignment) {
            double p = probabilities[i];
            if (p > 0.0) return false;
         }
      }

      return true;
   }
}

const maf::Role & maf::Wildcard::pick_role(const Rulebook &rulebook) {
   if (uses_evaluator()) {
      std::vector<const Role *> role_ptrs{};
      std::vector<double> weights{};

      for (const Role &role: rulebook.roles()) {
         double w = _evaluator(role);
         if (w < 0.0) {
            std::ostringstream err{};
            err << "A joker with alias "
            << alias()
            << " returned the negative role weight of "
            << w
            << " for the role with alias "
            << role.alias()
            << ".";
            
            throw std::logic_error{err.str()};
         } else if (w > 0.0) {
            role_ptrs.push_back(&role);
            weights.push_back(w);
         }
      }

      if (role_ptrs.size() == 0) {
         std::ostringstream err{};
         err << "A joker with alias "
         << alias()
         << " chose zero as the weight of every role in the rulebook.";

         throw std::logic_error{err.str()};
      }

      std::discrete_distribution<std::size_t> dist{weights.begin(), weights.end()};
      return *role_ptrs[dist(rkt::random_engine)];
   } else {
      return rulebook.get_role(_role_ids[_dist(rkt::random_engine)]);
   }
}

std::string maf::alias(Wildcard::ID id) {
   switch (id) {
      case Wildcard::ID::any:
         return "random";
      case Wildcard::ID::village:
         return "any_village";
      case Wildcard::ID::village_basic:
         return "basic_village";
      case Wildcard::ID::mafia:
         return "any_mafia";
      case Wildcard::ID::freelance:
         return "any_freelance";
   }
}
