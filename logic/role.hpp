#ifndef MAFIA_LOGIC_ROLE
#define MAFIA_LOGIC_ROLE

#include <string>

#include "../riketi/box.hpp"

namespace maf {
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
      be_lynched,
      win_duel
   };

   enum class Peace_condition {
      always_peaceful,
      village_eliminated,
      mafia_eliminated,
      last_survivor
   };

   struct Role {
      enum class ID {
         peasant,
         doctor,
         detective,
         racketeer,
         godfather,
         dealer,
         coward,
         actor,
         serial_killer,
         village_idiot,
         musketeer
      };

      /// Create a role with the given ID.
      ///
      /// All other traits are set to their default values.
      Role(ID id)
       : _id{id} { }

      /// The ID of the role.
      ID id() const {
         return _id;
      }
      
      /// The alias of the role.
      ///
      /// Note that this is fully determined by its ID.
      std::string alias() const;

      // The various traits of the role.
      Alignment alignment{Alignment::freelance};
      rkt::box<Ability> ability{};
      Win_condition win_condition{Win_condition::survive};
      Peace_condition peace_condition{Peace_condition::always_peaceful};
      bool is_suspicious{false};
      bool is_role_faker{false};
      bool is_troll{false};
      double duel_strength{1};

   private:
      ID _id;
   };

   /// The alias corresponding to the given role ID.
   std::string alias(Role::ID id);
}

#endif
