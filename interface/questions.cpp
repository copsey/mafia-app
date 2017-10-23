#include "console.hpp"
#include "errors.hpp"
#include "questions.hpp"

maf::Help_Screen * maf::Question::get_help_screen() const {
   return nullptr;
}

bool maf::Confirm_end_game::handle_commands(const std::vector<std::string> & commands) {
   if (Question::handle_commands(commands)) return true;

   auto& con = this->console();

   if (commands_match(commands, {"yes"})) {
      con.end_game();
      con.clear_question();
   } else if (commands_match(commands, {"no"})) {
      con.clear_question();
   } else {
      return false;
   }

   return true;
}

void maf::Confirm_end_game::write(std::ostream &os) const {
   os << "^TEnd Game?^/You are about to end the current game.\nAre you sure that you want to do this? (^cyes^/ or ^cno^/)";
}

//bool maf::Confirm_mafia_kill_skip::do_commands(const std::vector<std::string> &commands, Console &console) {
//   if (commands_match(commands, {"yes"})) {
//      console.g
//      return true;
//   } else if (commands_match(commands, {"no"})) {
//      return true;
//   } else {
//      return false;
//   }
//}
//
//void maf::Confirm_mafia_kill_skip::write(std::ostream &os) const {
//   os << "^TSkip?^/You are about to skip the mafia's nightly kill.\nAre you sure that you want to do this? (^cyes^/ or ^cno^/)";
//}
