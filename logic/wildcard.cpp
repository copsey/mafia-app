#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "../riketi/map.hpp"
#include "../riketi/random.hpp"
#include "../riketi/ref.hpp"

#include "wildcard.hpp"
#include "rulebook.hpp"

maf::Wildcard::Wildcard(ID id, const std::map<Role::ID, double> & weights)
 : _id{id}, _role_ids{rkt::key_begin(weights), rkt::key_end(weights)}, _dist{rkt::item_begin(weights), rkt::item_end(weights)} {
   auto is_negative = [](double w) { return w < 0; };
   auto is_zero = [](double w) { return w == 0; };

   if (std::any_of(rkt::item_begin(weights), rkt::item_end(weights), is_negative)) {
      std::ostringstream err{};
      err << "A wildcard with alias " << alias() << " was created with a negative role weight.";

      throw std::invalid_argument{err.str()};
   }

   if (std::all_of(rkt::item_begin(weights), rkt::item_end(weights), is_zero)) {
      std::ostringstream err{};
      err << "A wildcard with alias " << alias() << " was created with every role weight set to zero.";

      throw std::invalid_argument{err.str()};
   }
}

const char * maf::Wildcard::alias() const {
   return maf::alias(_id);
}

bool maf::Wildcard::matches_alignment(Alignment alignment, const Rulebook & rulebook) const {
   if (uses_evaluator()) {
      auto wrong_alignment = [&](const Role & role) {
         return ((role.alignment() == alignment) && (_evaluator(role) > 0.0));
      };

      return std::none_of(rulebook.roles_begin(), rulebook.roles_end(), wrong_alignment);
   } else {
      std::vector<double> probabilities = _dist.probabilities();

      for (std::size_t i{0}; i < _role_ids.size(); ++i) {
         const Role & r = rulebook.get_role(_role_ids[i]);
         if (r.alignment() != alignment) {
            double p = probabilities[i];
            if (p > 0.0) return false;
         }
      }

      return true;
   }
}

const maf::Role & maf::Wildcard::pick_role(const Rulebook & rulebook) {
   if (uses_evaluator()) {
      std::vector<rkt::ref<const Role>> role_refs{};
      std::vector<double> weights{};

      auto evaluate_role = [&](const Role & role) {
         double w = _evaluator(role);

         if (w < 0.0) {
            std::ostringstream err{};
            err << "A wildcard with alias "
                << alias()
                << " returned the negative role weight of "
                << w
                << " for the role with alias "
                << role.alias()
                << ".";

            throw std::logic_error{err.str()};
         } else if (w > 0.0) {
            role_refs.emplace_back(role);
            weights.push_back(w);
         }
      };

      rulebook.for_each_role(evaluate_role);

      if (role_refs.size() == 0) {
         std::ostringstream err{};
         err << "A wildcard with alias "
             << alias()
             << " chose zero as the weight of every role in the rulebook.";

         throw std::logic_error{err.str()};
      }

      std::discrete_distribution<std::size_t> dist{weights.begin(), weights.end()};
      return *role_refs[dist(rkt::random_engine)];
   } else {
      return rulebook.get_role(_role_ids[_dist(rkt::random_engine)]);
   }
}

const char * maf::alias(Wildcard::ID id) {
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
