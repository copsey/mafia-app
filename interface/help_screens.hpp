#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

#include <functional>
#include <ostream>

#include "../riketi/box.hpp"

#include "../logic/logic.hpp"

#include "events.hpp"
#include "game_log.hpp"

namespace maf {
   struct Help_Screen {
      // Write a tagged string detailing the help screen to os.
      virtual void write(std::ostream &os) const = 0;
   };


   struct Event_Help_Screen: Help_Screen {
      Event_Help_Screen(const Event &e)
         : event{e}
      { }

      rkt::ref<const Event> event;

      void write(std::ostream &os) const override;
   };


   struct Role_Info_Screen: Help_Screen {
      Role_Info_Screen(const Role &role)
         : role{role}
      { }

      rkt::ref<const Role> role;

      void write(std::ostream &os) const override;
   };


   struct List_Roles_Screen: Help_Screen {
      List_Roles_Screen(const Rulebook &rulebook, rkt::box<Alignment> alignment = {})
         : rulebook{rulebook}, alignment{alignment}
      { }

      void write(std::ostream &os) const override;

      rkt::ref<const Rulebook> rulebook;
      rkt::box<Alignment> alignment;
   };


   struct Setup_Help_Screen: Help_Screen {
      void write(std::ostream &os) const override;
   };


   /// A screen presenting information on a given player.
   struct Player_Info_Screen: Help_Screen {
      /// Create an info screen for `player`, who should be a participant in the
      /// game managed by `game_log`.
      Player_Info_Screen(const Player & player, const Game_log & game_log)
         : _player_ref{player}, _game_log_ref{game_log}
      { }

      void write(std::ostream & os) const override;

   private:
      rkt::ref<const Player> _player_ref;
      rkt::ref<const Game_log> _game_log_ref;
   };
}

#endif
