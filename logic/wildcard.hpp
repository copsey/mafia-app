#ifndef MAFIA_LOGIC_WILDCARD
#define MAFIA_LOGIC_WILDCARD

#include <functional>
#include <map>
#include <random>
#include <vector>

#include "role.hpp"

namespace maf {
   struct Rulebook;

   struct Wildcard {
      enum class ID {
         any,
         village,
         village_basic,
         mafia,
         freelance
      };

      // A function which assigns a weight to a given role.
      using Role_evaluator = std::function<double(const Role &)>;

      // Creates a new wildcard with the given role evaluator.
      Wildcard(ID id, Role_evaluator evaluator);
      // Creates a new wildcard with the given role weights.
      Wildcard(ID id, const std::map<Role::ID, double> &weights);

      // The ID of the wildcard.
      ID id() const;
      // The alias of the wildcard, which is fully determined by its ID.
      std::string alias() const;

      // Checks if the wildcard will only return roles of the given alignment
      // from the given rulebook.
      bool matches_alignment(Role::Alignment alignment, const Rulebook &rulebook) const;

      // Randomly chooses a role from the given rulebook.
      const Role & pick_role(const Rulebook &rulebook);

   private:
      ID _id;
      Role_evaluator _evaluator{};
      std::vector<Role::ID> _role_ids{};
      std::discrete_distribution<std::vector<Role::ID>::size_type> _dist{};

      // Whether the wildcard uses an evaluator in picking a role.
      // If false, then predefined weights will be used instead.
      bool uses_evaluator() const;
   };

   // Get the alias corresponding to the given wildcard ID.
   std::string alias(Wildcard::ID id);
}

#endif