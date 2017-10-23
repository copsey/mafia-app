#include "game_screens.hpp"

#include "console.hpp"
#include "help_screens.hpp"
#include "questions.hpp"

bool maf::Game_Screen::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   auto& con = this->console();
   auto& glog = con.game_log();

   if (commands_match(commands, {"info", ""})) {
      auto& approx_name = commands[1];
      auto& player = glog.find_player(approx_name);

      con.store_help_screen(new Player_Info_Screen{con, player});
   } else if (commands_match(commands, {"end"})) {
      if (dynamic_cast<const Game_ended *>(&con.game_log().current_event())) {
         con.end_game();
      } else {
         con.store_question(new Confirm_end_game{con});
      }
   } else {
      return false;
   }

   return true;
}
