#include "../riketi/algorithm.hpp"

#include "console.hpp"
#include "errors.hpp"
#include "help_screens.hpp"
#include "names.hpp"

bool maf::Help_Screen::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   auto& con = this->console();

   if (commands_match(commands, {"ok"})) {
      con.clear_help_screen();
   } else {
      throw error::bad_commands();
//      err << "^h^TInvalid input!^/Please leave the help screen that is currently being displayed before trying to do anything else.\n(this is done by entering ^cok^/)";
   }

   return true;
}

maf::Help_Screen * maf::Help_Screen::get_help_screen() const {
   return nullptr;
}

void maf::Event_Help_Screen::write(std::ostream & os) const {
   _ev_ref->write_help(os);
}

void maf::Role_Info_Screen::write(std::ostream & os) const {
   /* FIXME */

   os << "^h^TMissing Role Info^/";
   os << "No extra help could be found for the ";
   os << full_name(*_role_ref);
   os << ".\n(this counts as a bug!)";

//   os << "^h^THelp: " << full_name(_r->id()) << "^/";
//
//   os << "To leave this screen, enter ^cok^/.";
}

void maf::List_Roles_Screen::write(std::ostream &os) const {
   auto& con = this->console();
   auto& rb = con.active_rulebook();

   std::vector<rkt::ref<const Role>> filtered_roles{};

   switch (_filter_alignment) {
      case Filter_Alignment::all:
         filtered_roles = rb.all_roles();
         break;

      case Filter_Alignment::village:
         filtered_roles = rb.village_roles();
         break;

      case Filter_Alignment::mafia:
         filtered_roles = rb.mafia_roles();
         break;

      case Filter_Alignment::freelance:
         filtered_roles = rb.freelance_roles();
         break;
   }

   auto order_by_full_name = [](const rkt::ref<const Role> & r1, const rkt::ref<const Role> & r2) {
      return full_name(*r1) < full_name(*r2);
   };

   rkt::sort(filtered_roles, order_by_full_name);

   switch (_filter_alignment) {
      case Filter_Alignment::all:
         os << "^h^TRoles^/";
         os << "The following is an alphabetical listing of every role in the rulebook:\n\n";
         break;

      case Filter_Alignment::village:
         os << "^h^TVillage Roles^/";
         os << "The following is an alphabetical listing of all of the roles in the rulebook aligned to the village:\n\n";
         break;

      case Filter_Alignment::mafia:
         os << "^h^TMafia Roles^/";
         os << "The following is an alphabetical listing of all of the roles in the rulebook aligned to the mafia:\n\n";
         break;

      case Filter_Alignment::freelance:
         os << "^h^TFreelance Roles^/";
         os << "The following is an alphabetical listing of all of the freelance roles in the rulebook:\n\n";
         break;
   }

   for (std::size_t i{0}; i < filtered_roles.size(); ++i) {
      const Role & r = *filtered_roles[i];

      os << "   the " << full_name(r) << ", alias " << "^c" << r.alias() << "^/";

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

   os << "\n\nTo see more information about the role with alias ^cthat^/, enter ^chelp r that^/.\n\n";

   switch (_filter_alignment) {
      case Filter_Alignment::all:
         os << "To list only the village roles, enter ^clist r v^/. Similarly, ^clist r m^/ will list the mafia roles, and ^clist r f^/ will list the freelance roles.";
         break;

      case Filter_Alignment::village:
         os << "To list the mafia roles, enter ^clist r m^/, and to list the freelance roles, enter ^clist r f^/.";
         break;

      case Filter_Alignment::mafia:
         os << "To list the village roles, enter ^clist r v^/, and to list the freelance roles, enter ^clist r f^/.";
         break;

      case Filter_Alignment::freelance:
         os << "To list the village roles, enter ^clist r v^/, and to list the mafia roles, enter ^clist r m^/.";
         break;
   }

   os << "\n\nTo leave this screen, enter ^cok^/.";
}

void maf::Setup_Help_Screen::write(std::ostream & os) const {
   os << "^h^THelp: Setup^/The setup screen is where you can choose the players and cards that will feature in the next game of Mafia.\n\nTo add a player called ^cname^/ to the next game, enter ^cadd p name^/. The player can be removed again by entering ^ctake p name^/. To remove all of the players that have been selected, enter ^cclear p^/.\n\nA single copy of the rolecard with alias ^cthat^/ can be added by entering ^cadd r that^/, and a single copy removed by entering ^ctake r that^/. You can remove all copies of the rolecard by entering ^cclear r that^/, and you can remove every rolecard that has been selected by entering ^cclear r^/.\n\nSimilar effects can be achieved for the wildcard with alias ^cthat^/ by using the commands ^cadd w that^/, ^ctake w that^/, ^cclear w that^/, and ^cclear w^/ respectively. In addition, every card that has been selected (both rolecards and wildcards) can be removed through the use of the command ^cclear c^/.\n\nTo clear absolutely everything (both players and cards), enter ^cclear^/.\n\nOnce you have finished choosing players and cards, you can enter ^cbegin^/ to start a new game. Alternatively, you can enter ^cpreset i^/ to start a particular preconfigured game, or just ^cpreset^/ to start a random preset. (note: at the moment, presets exist primarily for debugging, and you are unlikely to ever use them.)\n\nYou can get extra information on the role with alias ^cthat^/ by entering ^chelp r that^/, and you can see a list of every role in the rulebook by entering ^clist r^/. To see a list of only the village roles, you can enter ^clist r v^/. Similarly, the command ^clist r m^/ will list the mafia roles, and the command ^clist r f^/ will list the freelance roles.\n\nThe commands ^chelp w that^/, ^clist w^/, ^clist w v^/, ^clist w m^/, and ^clist w f^/ have similar effects for wildcards.\n\nTo leave this screen, enter ^cok^/.";
}

void maf::Player_Info_Screen::write(std::ostream & os) const {
   auto& player = *_player_ref;
   auto& con = this->console();
   auto& glog = con.game_log();
   auto& game = glog.game();

   os << "^h^TInfo: " << glog.get_name(player) << "^/";

   os << "Your role is the " << full_name(player.role()) << ".";
   if (player.has_fake_role()) {
      // FIXME
      os << " You were randomly given this role from the ^c"
      << player.wildcard()->alias()
      << "^/ wildcard.";
   }

   if (game.time() == Time::day) {
      if (player.lynch_vote()) {
         os << "\n\nYou are voting to lynch "
         << glog.get_name(*player.lynch_vote())
         << ".";
      } else {
         os << "\n\nYou are not voting to lynch anyone.";
      }
   }

   for (auto& inv: game.investigations()) {
      if (inv.caster() == player) {
         os << "\n\nYou checked ";
         os << glog.get_name(inv.target());
         os << " on night ";
         os << inv.date();
         os << ", who appeared to be ";
         os << ((inv.result()) ? "suspicious" : "innocent");
         os << ".";
      }
   }
}
