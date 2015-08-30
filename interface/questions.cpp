#include "console.hpp"
#include "questions.hpp"

bool mafia::Confirm_end_game::do_commands(const std::vector<std::string> &commands, Console &console) {
   if (commands_match(commands, {"yes"})) {
      console.end_game();
      return true;
   }
   else if (commands_match(commands, {"no"})) {
      return true;
   }
   else {
      throw Bad_commands();
   }
}

void mafia::Confirm_end_game::write(std::ostream &os) const {
   os << "^HEnd Game?^hYou are about to end the current game.\nAre you sure that you want to do this? (^cyes^h or ^cno^h)";
}

//bool mafia::Confirm_mafia_kill_skip::do_commands(const std::vector<std::string> &commands, Console &console) {
//   if (commands_match(commands, {"yes"})) {
//      console.g
//      return true;
//   }
//   else if (commands_match(commands, {"no"})) {
//      return true;
//   }
//   else {
//      throw Bad_commands();
//   }
//}
//
//void mafia::Confirm_mafia_kill_skip::write(std::ostream &os) const {
//   os << "^HSkip?^hYou are about to skip the mafia's nightly kill.\nAre you sure that you want to do this? (^cyes^h or ^cno^h)";
//}