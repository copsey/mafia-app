#include "game_screens.hpp"

#include "console.hpp"
#include "help_screens.hpp"
#include "questions.hpp"

maf::Game_log & maf::Game_Screen::game_log() const {
   return *console()._game_log;
}

bool maf::Game_Screen::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   auto& con = this->console();
   auto& glog = this->game_log();

   if (commands_match(commands, {"info", ""})) {
      auto& pl = glog.find_player(commands[1]);

      con.store_help_screen(new Player_Info_Screen{con, pl});
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

bool maf::screen::Town_Meeting::handle_commands(const std::vector<std::string> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   //auto& con = this->console();
   auto& glog = this->game_log();

   if (_lynch_can_occur) {
      if (commands_match(commands, {"kick", ""})) {
         auto& pl = glog.find_player(commands[1]);

         glog.kick_player(pl.id());
         glog.advance();
      } else if (commands_match(commands, {"vote", "", ""})) {
         auto& voter = glog.find_player(commands[1]);
         auto& target = glog.find_player(commands[2]);

         glog.cast_lynch_vote(voter.id(), target.id());
         glog.advance();
      } else if (commands_match(commands, {"abstain", ""})) {
         auto& voter = glog.find_player(commands[1]);

         glog.clear_lynch_vote(voter.id());
         glog.advance();
      } else if (commands_match(commands, {"lynch"})) {
         glog.process_lynch_votes();
         glog.advance();
      } else if (commands_match(commands, {"duel", "", ""})) {
         auto& caster = glog.find_player(commands[1]);
         auto& target = glog.find_player(commands[2]);

         glog.stage_duel(caster.id(), target.id());
         glog.advance();
      } else {
         return false;
      }
   } else {
      if (commands_match(commands, {"night"})) {
         glog.begin_night();
         glog.advance();
      } else if (commands_match(commands, {"kick", ""})) {
         auto& pl = glog.find_player(commands[1]);

         glog.kick_player(pl.id());
         glog.advance();
      } else if (commands_match(commands, {"duel", "", ""})) {
         auto& caster = glog.find_player(commands[1]);
         auto& target = glog.find_player(commands[2]);

         glog.stage_duel(caster.id(), target.id());
         glog.advance();
      } else {
         return false;
      }
   }

   return true;
}

void maf::screen::Town_Meeting::write(std::ostream & os) const {
   // FIXME: add in proper content.

   os << "^TDay " << _date << "^/";

   if (_lynch_can_occur) {
      os << "Gathered outside the town hall are:\n";

      for (auto it = _players.begin(); it != _players.end(); ) {
         auto& p_ref = *it;
         os << "   ";
         os << game_log().get_name(*p_ref);
         if (p_ref->lynch_vote() != nullptr) {
            os << ", voting to lynch ";
            os << game_log().get_name(*(p_ref->lynch_vote()));
         }
         os << ((++it == _players.end()) ? "." : ",\n");
      }

      os << "\n\nAs it stands, ";
      os << (_next_lynch_victim ? game_log().get_name(*_next_lynch_victim) : "nobody");
      os << " will be lynched.^h\n\nEnter ^clynch^/ to submit the current lynch votes. Daytime abilities may also be used at this point.";
   } else {
      os << "^iWith little time left in the day, the townsfolk prepare themselves for another night of uncertainty...^/\n\n";
      os << "Gathered outside the town hall are:\n";

      for (auto it = _players.begin(); it != _players.end(); ) {
         auto& p_ref = *it;
         os << "   ";
         os << game_log().get_name(*p_ref);
         os << ((++it == _players.end()) ? "." : ",\n");
      }

      os << "^h\n\nAnybody who wishes to use a daytime ability may do so now. Otherwise, enter ^cnight^/ to continue.";
   }
}

maf::Help_Screen * maf::screen::Town_Meeting::get_help_screen() const {
   // FIXME: Add help screen for Town_Meeting.

   return nullptr;
}
