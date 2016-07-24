#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

#include <functional>
#include <ostream>

#include "../riketi/box.hpp"

#include "../logic/logic.hpp"

#include "events.hpp"

namespace maf {
   struct Help_screen {
      // Write a tagged string detailing the help screen to os.
      virtual void write(std::ostream &os) const = 0;
   };


   struct Event_help_screen: Help_screen {
      Event_help_screen(const Event &e): event{e} { }

      rkt::ref<const Event> event;

      void write(std::ostream &os) const override;
   };


   struct Role_info_screen: Help_screen {
      Role_info_screen(const Role &role): role{role} { }

      rkt::ref<const Role> role;

      void write(std::ostream &os) const override;
   };


   struct List_roles_screen: Help_screen {
      List_roles_screen(const Rulebook &rulebook, rkt::box<Alignment> alignment = {})
      : rulebook{rulebook}, alignment{alignment} { }

      void write(std::ostream &os) const override;

      rkt::ref<const Rulebook> rulebook;
      rkt::box<Alignment> alignment;
   };


   struct Setup_help_screen: Help_screen {
      void write(std::ostream &os) const override;
   };
}

#endif