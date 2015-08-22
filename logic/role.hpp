#ifndef MAFIA_ROLE_H
#define MAFIA_ROLE_H

namespace mafia {
   class Rulebook;

   struct Role {
      friend class Rulebook;

      enum class ID {
         peasant,
         racketeer,
         coward
      };

      enum class Alignment {
         village,
         mafia,
         freelance
      };

      enum class Win_condition {
         survive
      };

      enum class Peace_condition {
         always_peaceful,
         village_eliminated,
         mafia_eliminated,
         last_survivor
      };

      // Create a role with the given ID, and all other traits default.
      Role(ID id): _id{id} { }

      ID id() const { return _id; }
      Alignment alignment() const { return _alignment; }
      Peace_condition peace_condition() const { return _peace_condition; }
      Win_condition win_condition() const { return _win_condition; }
      bool is_suspicious() const { return _is_suspicious; }

   private:
      ID _id;
      Alignment _alignment{Alignment::freelance};
      Peace_condition _peace_condition{Peace_condition::always_peaceful};
      Win_condition _win_condition{Win_condition::survive};
      bool _is_suspicious{false};
   };
}

#endif
