#ifndef MAFIA_INTERFACE_HELP_SCREENS
#define MAFIA_INTERFACE_HELP_SCREENS

#include <functional>
#include <ostream>

#include "../logic/logic.hpp"

#include "events.hpp"
#include "game_log.hpp"
#include "screens.hpp"

namespace maf {
   struct Help_Screen: Base_Screen {
      using Base_Screen::Base_Screen;

      virtual bool handle_commands(const std::vector<std::string_view> & commands) override;
      virtual Help_Screen * get_help_screen() const override;
   };


   struct Event_Help_Screen: Help_Screen {
      Event_Help_Screen(Console & con, const Event & ev)
         : Help_Screen{con}, _ev_ref{ev}
      { }

      void write(std::ostream & os) const override;

   private:
      rkt::ref<const Event> _ev_ref;
   };


   struct Role_Info_Screen: Help_Screen {
      Role_Info_Screen(Console & con, const Role & role)
         : Help_Screen{con}, _role_ref{role}
      { }

      void write(std::ostream & os) const override;

   private:
      rkt::ref<const Role> _role_ref;
   };


   struct List_Roles_Screen: Help_Screen {
      // An optional filter, specifying the alignment of roles that should be displayed.
      enum class Filter_Alignment { all, village, mafia, freelance };

      // Create a help screen listing all of the roles present in `rulebook`.
      //
      // It is also possible to specify an optional alignment filter, in which case
      // only roles of that alignment will be listed.
      List_Roles_Screen(Console & con, Filter_Alignment alignment = Filter_Alignment::all)
         : Help_Screen{con}, _filter_alignment{alignment}
      { }

      void write(std::ostream & os) const override;

   private:
      Filter_Alignment _filter_alignment;
   };


   struct Setup_Help_Screen: Help_Screen {
      using Help_Screen::Help_Screen;

      void write(std::ostream & os) const override;
   };


   /// A screen presenting information on a given player.
   struct Player_Info_Screen: Help_Screen {
      /// Create an info screen for `pl`, who should be a participant in the
      /// game being displayed by `con`.
      Player_Info_Screen(Console & con, const Player & pl)
         : Help_Screen{con}, _player_ref{pl}
      { }

      void write(std::ostream & os) const override;

   private:
      rkt::ref<const Player> _player_ref;
   };
}

#endif
