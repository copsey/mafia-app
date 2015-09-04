#ifndef MAFIA_LOGIC_ROLE
#define MAFIA_LOGIC_ROLE

#include <string>

#include "../riketi/box.hpp"

namespace mafia {
   struct Role {
      enum class ID {
         peasant,
         doctor,
         detective,
         racketeer,
         dealer,
         coward,
         serial_killer,
         musketeer
      };

      enum class Alignment {
         village,
         mafia,
         freelance
      };

      struct Ability {
         enum class ID {
            kill,
            heal,
            investigate,
            peddle,
            duel
         };

         ID id;
      };

      enum class Win_condition {
         survive,
         village_remains,
         mafia_remains,
         win_duel
      };

      enum class Peace_condition {
         always_peaceful,
         village_eliminated,
         mafia_eliminated,
         last_survivor
      };

      // Creates a role with the given ID.
      // All other traits are given their default values.
      Role(ID id);

      // The ID of the role.
      ID id() const;
      // The alias of the role, which is fully determined from its ID.
      std::string alias() const;

      // The various traits of the role.
      Alignment alignment{Alignment::freelance};
      rkt::box<Ability> ability{};
      Win_condition win_condition{Win_condition::survive};
      Peace_condition peace_condition{Peace_condition::always_peaceful};
      bool is_suspicious{false};
      double duel_strength{1};

   private:
      ID _id;
   };

   // Get the alias corresponding to the given role ID.
   std::string alias(Role::ID id);
}

#endif
