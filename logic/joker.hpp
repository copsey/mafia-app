#ifndef MAFIA_JOKER_H
#define MAFIA_JOKER_H

#include <functional>
#include <map>
#include <random>
#include <vector>

#include "role.hpp"

namespace mafia {
   struct Rulebook;

   struct Joker {
      enum class ID {
         any,
         village,
         village_basic,
         mafia,
         freelance
      };

      using Role_evaluator = std::function<double(const Role &)>;

      Joker(ID id, Role_evaluator role_evaluator)
         : _id{id}, _evaluator{role_evaluator} {
      }

      Joker(ID id, std::map<Role::ID, double> weights);

      ID id() const {
         return _id;
      }

      Role & choose_role(Rulebook &rulebook);

   private:
      ID _id;
      Role_evaluator _evaluator{};
      std::vector<Role::ID> _role_ids{};
      std::discrete_distribution<std::vector<Role::ID>::size_type> _dist{};
   };
}

#endif
