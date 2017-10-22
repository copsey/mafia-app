#include "console.hpp"
#include "questions.hpp"

bool maf::Confirm_end_game::do_commands(const std::vector<std::string> &commands) {
   if (commands_match(commands, {"yes"})) {
      _console_ref->end_game();
      return true;
   }
   else if (commands_match(commands, {"no"})) {
      return true;
   }
   else {
      throw Bad_commands();
   }
}

void maf::Confirm_end_game::write(std::ostream &os) const {
   os << "^HEnd Game?^hYou are about to end the current game.\nAre you sure that you want to do this? (^cyes^h or ^cno^h)";
}

//bool maf::Confirm_mafia_kill_skip::do_commands(const std::vector<std::string> &commands, Console &console) {
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
//void maf::Confirm_mafia_kill_skip::write(std::ostream &os) const {
//   os << "^HSkip?^hYou are about to skip the mafia's nightly kill.\nAre you sure that you want to do this? (^cyes^h or ^cno^h)";
//}
