#include "../riketi/algorithm.hpp"

#include "help_screens.hpp"
#include "names.hpp"

void maf::Event_Help_Screen::write(std::ostream& os) const {
   event->write_help(os);
}

void maf::Role_Info_Screen::write(std::ostream& os) const {
   /* FIXME */

   os << "^HMissing Role Info^hNo extra help could be found for the ";
   os << full_name(*role);
   os << ".\n(this counts as a bug!)";

//   os << "^HHelp: " << full_name(_r->id()) << "^h";
//
//   os << "To leave this screen, enter ^cok^h.";
}

void maf::List_Roles_Screen::write(std::ostream &os) const {
   std::vector<rkt::ref<const Role>> filtered_roles{};

   switch (_filter_alignment) {
      case Filter_Alignment::all: {
         for (const Role & role: _rulebook->roles()) {
            filtered_roles.emplace_back(role);
         }
         break;
      }

      case Filter_Alignment::village:
         filtered_roles = _rulebook->village_roles();
         break;

      case Filter_Alignment::mafia:
         filtered_roles = _rulebook->mafia_roles();
         break;

      case Filter_Alignment::freelance:
         filtered_roles = _rulebook->freelance_roles();
         break;
   }

   auto order_by_full_name = [](const rkt::ref<const Role> & r1, const rkt::ref<const Role> & r2) {
      return full_name(*r1) < full_name(*r2);
   };

   rkt::sort(filtered_roles, order_by_full_name);

   switch (_filter_alignment) {
      case Filter_Alignment::all:
         os << "^H" << "Roles";
         os << "^h" << "The following is an alphabetical listing of every role in the rulebook:\n\n";
         break;

      case Filter_Alignment::village:
         os << "^H" << "Village Roles";
         os << "^h" << "The following is an alphabetical listing of all of the roles in the rulebook aligned to the village:\n\n";
         break;

      case Filter_Alignment::mafia:
         os << "^H" << "Mafia Roles";
         os << "^h" << "The following is an alphabetical listing of all of the roles in the rulebook aligned to the mafia:\n\n";
         break;

      case Filter_Alignment::freelance:
         os << "^H" << "Freelance Roles";
         os << "^h" << "The following is an alphabetical listing of all of the freelance roles in the rulebook:\n\n";
         break;
   }

   for (std::size_t i{0}; i < filtered_roles.size(); ++i) {
      const Role & r = *filtered_roles[i];

      os << "   the " << full_name(r) << ", alias " << "^c" << r.alias() << "^h";

      if (_filter_alignment == Filter_Alignment::all) {
         os << ", ";

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
      }

      os << ((i == filtered_roles.size() - 1) ? "." : ";\n");
   }

   os << "\n\nTo see more information about the role with alias ^cthat^h, enter ^chelp r that^h.\n\n";

   switch (_filter_alignment) {
      case Filter_Alignment::all:
         os << "To list only the village roles, enter ^clist r v^h. Similarly, ^clist r m^h will list the mafia roles, and ^clist r f^h will list the freelance roles.";
         break;

      case Filter_Alignment::village:
         os << "To list the mafia roles, enter ^clist r m^h, and to list the freelance roles, enter ^clist r f^h.";
         break;

      case Filter_Alignment::mafia:
         os << "To list the village roles, enter ^clist r v^h, and to list the freelance roles, enter ^clist r f^h.";
         break;

      case Filter_Alignment::freelance:
         os << "To list the village roles, enter ^clist r v^h, and to list the mafia roles, enter ^clist r m^h.";
         break;
   }

   os << "\n\nTo leave this screen, enter ^cok^h.";
}

void maf::Setup_Help_Screen::write(std::ostream &os) const {
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
