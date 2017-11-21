#include "command.hpp"
#include "console.hpp"
#include "errors.hpp"
#include "events.hpp"
#include "help_screens.hpp"
#include "screens.hpp"

bool maf::Base_Screen::handle_commands(const std::vector<std::string_view> & commands) {
   auto& con = console();

   if (commands.empty()) {
      throw error::missing_commands();
   }

   if (commands_match(commands, {"help"})) {
      auto hs = this->get_help_screen();
      if (hs) {
         con.store_help_screen(hs);
      }
   } else if (commands_match(commands, {"help", "r", ""})) {
      const Role & role = con.active_rulebook().get_role(commands[2]);
      con.store_help_screen(new Role_Info_Screen(con, role));
   } else if (commands_match(commands, {"list", "r"})) {
      con.store_help_screen(new List_Roles_Screen(con));
   } else if (commands_match(commands, {"list", "r", "v"})) {
      con.store_help_screen(new List_Roles_Screen(con, List_Roles_Screen::Filter_Alignment::village));
   } else if (commands_match(commands, {"list", "r", "m"})) {
      con.store_help_screen(new List_Roles_Screen(con, List_Roles_Screen::Filter_Alignment::mafia));
   } else if (commands_match(commands, {"list", "r", "f"})) {
      con.store_help_screen(new List_Roles_Screen(con, List_Roles_Screen::Filter_Alignment::freelance));
   } else {
      return false;
   }

   // FIXME: add  "list w", "list w v", "list w m", "list w f".

   // FIXME: add 'list p random', a utility command to generate a list of the players in a game, in a random order.
   // (for example, when asking players to finalise their lynch votes, without the option to later amend.)
   // list p should be context-aware, i.e. it should show pending players if no game is in progress, and actual players if a game is in progress.

   return true;
}
