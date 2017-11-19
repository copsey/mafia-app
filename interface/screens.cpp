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
      auto& alias = commands[2];
      auto& role = con.active_rulebook().get_role(alias);

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

   return true;
}
