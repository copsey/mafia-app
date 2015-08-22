#ifndef MAFIA_EXCEPTION_H
#define MAFIA_EXCEPTION_H

#include "player.hpp"

namespace mafia {
   // Signifies that a game was created with unequal numbers of players and
   // role cards.
   struct players_to_cards_mismatch {
      players_to_cards_mismatch(std::size_t num_players, std::size_t num_cards)
      : _np{num_players}, _nc{num_cards} { }

      std::size_t num_players() const { return _np; }
      std::size_t num_cards() const { return _nc; }

   private:
      std::size_t _np, _nc;
   };


   // Signifies that the game cannot continue right now, providing a reason why
   // this is so.
   struct Game_cannot_continue {
      enum class Reason {
         lynch_can_occur
      };

      Game_cannot_continue(Reason reason): _r(reason) { }

      Reason reason() const { return _r; }

   private:
      Reason _r;
   };


   // Signifies that the caster cannot cast a lynch vote against the target,
   // providing a reason why this is so.
   struct lynch_vote_failed {
      enum class Reason {
         bad_timing,
         caster_is_not_present,
         target_is_not_present,
         caster_is_target
      };

      lynch_vote_failed(const Player &caster,
                        const Player *target,
                        Reason reason)
      : _c{&caster}, _t{target}, _r{reason} { }

      const Player & caster() const { return *_c; }
      const Player * target() const { return _t; }
      Reason reason() const { return _r; }

   private:
      const Player *_c, *_t;
      Reason _r;
   };


   // Signifies that a lynch cannot occur at the moment.
   struct badly_timed_lynch { };
}

#endif
