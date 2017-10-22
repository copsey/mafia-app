#include "../riketi/algorithm.hpp"

#include "help_screens.hpp"
#include "names.hpp"

void maf::Event_help_screen::write(std::ostream& os) const {
   event->write_help(os);
}

void maf::Role_info_screen::write(std::ostream& os) const {
   /* FIXME */

   os << "^HMissing Role Info^hNo extra help could be found for the ";
   os << full_name(*role);
   os << ".\n(this counts as a bug!)";

//   os << "^HHelp: " << full_name(_r->id()) << "^h";
//
//   os << "To leave this screen, enter ^cok^h.";
}

void maf::List_roles_screen::write(std::ostream &os) const {
   if (alignment.is_empty()) {
      os << "^HRoles^hThe following is an alphabetical listing of every role in the rulebook:\n\n";

      std::vector<const Role *> sorted_roles{};
      for (const Role& r: rulebook->roles()) {
         sorted_roles.push_back(&r);
      }
      rkt::sort(sorted_roles, [](const Role *r1, const Role *r2) {
         return full_name(*r1) < full_name(*r2);
      });

      for (std::size_t i{0}; i < sorted_roles.size(); ++i) {
         const Role &r = *sorted_roles[i];

         os << "   the " << full_name(r) << ", alias ^c" << r.alias() << "^h, ";

         switch (r.alignment()) {
            case Alignment::village:
               os << "aligned to the village";
               break;
            case Alignment::mafia:
               os << "aligned to the mafia";
               break;
            case Alignment::freelance:
               os << "a freelance role";
               break;
         }

         os << ((i == sorted_roles.size() - 1) ? "." : ";\n");
      }

      os << "\n\nTo see more information about the role with alias ^cthat^h, enter ^chelp r that^h.\n\nTo list only the village roles, enter ^clist r v^h. Similarly, ^clist r m^h will list the mafia roles, and ^clist r f^h will list the freelance roles.";
   } else {
      std::vector<rkt::ref<const Role>> v;
      switch (alignment.get()) {
         case Alignment::village:
            v = rulebook->village_roles();
            os << "^HVillage Roles^hThe following is an alphabetical listing of all of the roles in the rulebook aligned to the village:\n\n";
            break;
         case Alignment::mafia:
            v = rulebook->mafia_roles();
            os << "^HMafia Roles^hThe following is an alphabetical listing of all of the roles in the rulebook aligned to the mafia:\n\n";
            break;
         case Alignment::freelance:
            v = rulebook->freelance_roles();
            os << "^HFreelance Roles^hThe following is an alphabetical listing of all of the freelance roles in the rulebook:\n\n";
            break;
      }

      /* FIXME: make rkt::refs work directly. */
      /* FIXME: still not sorted alphabetically! */

      std::vector<const Role *> w;
      for (rkt::ref<const Role> r: v) {
         w.push_back(&(*r));
      }

      rkt::sort(w, [](const Role *r1, const Role *r2) {
         return full_name(r1->id()) < full_name(r2->id());
      });

      for (std::size_t i{0}; i < v.size(); ++i) {
         const Role& r = *(v[i]);

         os << "   the "
         << full_name(r)
         << ", alias ^c"
         << r.alias()
         << "^h"
         << ((i == v.size() - 1) ? "." : ";\n");
      }

      os << "\n\nTo see more information about the role with alias ^cthat^h, enter ^chelp r that^h.\n\n";

      switch (alignment.get()) {
         case Alignment::village:
            os << "To list the mafia roles, enter ^clist r m^h, and to list the freelance roles, enter ^clist r f^h.";
            break;
         case Alignment::mafia:
            os << "To list the village roles, enter ^clist r v^h, and to list the freelance roles, enter ^clist r f^h.";
            break;
         case Alignment::freelance:
            os << "To list the village roles, enter ^clist r v^h, and to list the mafia roles, enter ^clist r m^h.";
            break;
      }
   }

   os << "\n\nTo leave this screen, enter ^cok^h.";
}

void maf::Setup_help_screen::write(std::ostream &os) const {
   os << "^HHelp: Setup^hThe setup screen is where you can choose the players and cards that will feature in the next game of Mafia.\n\nTo add a player called ^cname^h to the next game, enter ^cadd p name^h. The player can be removed again by entering ^ctake p name^h. To remove all of the players that have been selected, enter ^cclear p^h.\n\nA single copy of the rolecard with alias ^cthat^h can be added by entering ^cadd r that^h, and a single copy removed by entering ^ctake r that^h. You can remove all copies of the rolecard by entering ^cclear r that^h, and you can remove every rolecard that has been selected by entering ^cclear r^h.\n\nSimilar effects can be achieved for the wildcard with alias ^cthat^h by using the commands ^cadd w that^h, ^ctake w that^h, ^cclear w that^h, and ^cclear w^h respectively. In addition, every card that has been selected (both rolecards and wildcards) can be removed through the use of the command ^cclear c^h.\n\nTo clear absolutely everything (both players and cards), enter ^cclear^h.\n\nOnce you have finished choosing players and cards, you can enter ^cbegin^h to start a new game. Alternatively, you can enter ^cpreset i^h to start a particular preconfigured game, or just ^cpreset^h to start a random preset. (note: at the moment, presets exist primarily for debugging, and you are unlikely to ever use them.)\n\nYou can get extra information on the role with alias ^cthat^h by entering ^chelp r that^h, and you can see a list of every role in the rulebook by entering ^clist r^h. To see a list of only the village roles, you can enter ^clist r v^h. Similarly, the command ^clist r m^h will list the mafia roles, and the command ^clist r f^h will list the freelance roles.\n\nThe commands ^chelp w that^h, ^clist w^h, ^clist w v^h, ^clist w m^h, and ^clist w f^h have similar effects for wildcards.\n\nTo leave this screen, enter ^cok^h.";
}

void maf::Player_Info_Screen::write(std::ostream & os) const {
   const Player & player = *_player_ref;
   const Game & game = _game_log_ref->game();
   const Game_log & game_log = *_game_log_ref;

   os << "^HInfo: " << game_log.get_name(player);

   os << "^hYour role is the " << full_name(player.role()) << ".";
   if (player.has_fake_role()) {
      // FIXME
      os << " You were randomly given this role from the ^c"
      << player.wildcard()->alias()
      << "^g wildcard.";
   }

   if (game.time() == Time::day) {
      if (player.lynch_vote()) {
         os << "\n\nYou are voting to lynch "
         << game_log.get_name(*player.lynch_vote())
         << ".";
      } else {
         os << "\n\nYou are not voting to lynch anyone.";
      }
   }

   for (const Investigation& inv: game.investigations()) {
      if (inv.caster() == player) {
         os << "\n\nYou checked ";
         os << game_log.get_name(inv.target());
         os << " on night ";
         os << inv.date();
         os << ", who appeared to be ";
         os << ((inv.result()) ? "suspicious" : "innocent");
         os << ".";
      }
   }
}
