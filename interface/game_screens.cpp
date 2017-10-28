#include "game_screens.hpp"

#include "console.hpp"
#include "help_screens.hpp"
#include "questions.hpp"

/*
 * maf::Game_Screen
 */

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
      con.store_question(new Confirm_end_game{con});
   } else {
      return false;
   }

   return true;
}



/*
 * maf::screen::Player_Given_Initial_Role
 */

bool maf::screen::Player_Given_Initial_Role::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   auto& glog = this->game_log();

   if (commands_match(commands, {"ok"})) {
      if (_page == 1) glog.advance();
      else _page++;
   } else {
      return false;
   }

   return true;
}

void maf::screen::Player_Given_Initial_Role::write(std::ostream & os) const {
   auto& glog = this->game_log();
   auto& pl_name = glog.get_name(*_p_ref);

   os << "^T" << pl_name << "'s Role^/";

   if (_page == 0) {
      auto rl_name = full_name(*_r_ref);
      auto rl_alias = _r_ref->alias();

      os << pl_name << ", your role is the "  << rl_name << ".";

      if (_w_ptr != nullptr) {
         /* FIXME */
         os << "\nYou were randomly given this role from the ^c";
         os << _w_ptr->alias();
         os << "^/ wildcard.";
      }

      os << "^h\n\nTo see a full description of your role, enter ^chelp r " << rl_alias << "^/.";
   } else {
      os << pl_name << ", you are about to be shown your role.";
   }
}

maf::Help_Screen * maf::screen::Player_Given_Initial_Role::get_help_screen() const {
   // FIXME: Add help screen for Player_Given_Initial_Role.

   return nullptr;
}



/*
 * maf::screen::Time_Changed
 */

bool maf::screen::Time_Changed::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   auto& glog = this->game_log();

   if (commands_match(commands, {"ok"})) {
      glog.advance();
   } else {
      return false;
   }

   return true;
}

void maf::screen::Time_Changed::write(std::ostream & os) const {
   switch (time) {
      case Time::day: {
         os << "^TDay " << date << "^/";
         os << "^iDawn breaks, and dim sunlight beams onto the weary townsfolk...\n\n^/It is now day ";
         os << date;
         os << ". Anybody still asleep can wake up.";
         break;
      }

      case Time::night: {
         os << "^TNight " << date << "^/";
         os << "^iAs darkness sets in, the townsfolk return to the comforts of their shelters...\n\n^/It is now night ";
         os << date;
         os << ". Everybody still in the game should go to sleep.";
         break;
      }
   }
}

maf::Help_Screen * maf::screen::Time_Changed::get_help_screen() const {
   // FIXME: Add help screen for Time_Changed, or decide if none is needed.
   // (only help would be indicating that command to continue to next
   //  screen is "ok".)

   return nullptr;
}



/*
 * maf::screen::Obituary
 */

bool maf::screen::Obituary::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   auto& glog = this->game_log();

   if (commands_match(commands, {"ok"})) {
      if (_deaths_index + 1 < _deaths.size()) {
         ++_deaths_index;
      } else {
         glog.advance();
      }
   } else {
      return false;
   }

   return true;
}

void maf::screen::Obituary::write(std::ostream & os) const {
   os << "^TObituary^/";

   if (_deaths_index < 0) {
      if (_deaths.size() == 0) {
         os << "Nobody died during the night.";
      } else {
         /* FIXME: reword to remove use of "us". */
         os << "It appears that " << _deaths.size() << " of us did not survive the night...";
      }
   } else {
      auto& glog = this->game_log();
      auto& death = *_deaths[_deaths_index];

      os << glog.get_name(death) << " died during the night!";

      if (death.is_haunted()) {
         os << "\n\nA slip of paper was found by their bed. On it has been written the name ^i";
         os << game_log().get_name(*death.haunter());
         os << "^/ over and over...";
      }
   }
}

maf::Help_Screen * maf::screen::Obituary::get_help_screen() const {
   // FIXME

   return nullptr;
}



/*
 * maf::screen::Town_Meeting
 */

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



/*
 * maf::screen::Player_Kicked
 */

bool maf::screen::Player_Kicked::handle_commands(const std::vector<std::string> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   //auto& con = this->console();
   auto& glog = this->game_log();

   if (commands_match(commands, {"ok"})) {
      glog.advance();
   } else {
      return false;
   }

   return true;
}

void maf::screen::Player_Kicked::write(std::ostream & os) const {
   auto& glog = this->game_log();
   auto& pl_name = glog.get_name(*_pl_ref);
   auto r_name = full_name(_pl_ref->role());

   os << "^T" << pl_name << " kicked^/";
   os << pl_name << " was kicked from the game!\n";
   os << "They were the " << r_name << ".";
}

maf::Help_Screen * maf::screen::Player_Kicked::get_help_screen() const {
   // FIXME

   return nullptr;
}



/*
 * maf::screen::Investigation_Result
 */

bool maf::screen::Investigation_Result::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   auto& glog = this->game_log();

   if (commands_match(commands, {"ok"})) {
      if (_page == 1) glog.advance();
      else _page++;
   } else {
      return false;
   }

   return true;
}

void maf::screen::Investigation_Result::write(std::ostream & os) const {
   auto & glog = this->game_log();
   auto & caster_name = glog.get_name(_inv.caster());
   auto & target_name = glog.get_name(_inv.target());

   os << "^TInvestigation Result^/";

   if (_page == 0) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you have completed your investigation of " << target_name << ".\n\n";

      if (_inv.result()) {
         os << target_name << " was behaving very suspiciously this night!";
      } else {
         os << "The investigation was fruitless. " << target_name << " appears to be innocent.";
      }

      os << "^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   }
}

maf::Help_Screen * maf::screen::Investigation_Result::get_help_screen() const {
   return nullptr;
}



/*
 * maf::screen::Game_Ended
 */

bool maf::screen::Game_Ended::handle_commands(const std::vector<std::string> & commands) {
   auto& con = this->console();
   //auto& glog = this->game_log();

   if (commands_match(commands, {"end"})) {
      con.end_game();
      return true;
   }

   return Game_Screen::handle_commands(commands);
}

void maf::screen::Game_Ended::write(std::ostream & os) const {
   auto& glog = this->game_log();
   auto& game = glog.game();

   std::vector<rkt::ref<const Player>> winners{};
   std::vector<rkt::ref<const Player>> losers{};

   for (auto& pl: game.players()) {
      if (pl.has_won()) {
         winners.emplace_back(pl);
      } else {
         losers.emplace_back(pl);
      }
   }

   os << "^TGame Over^/The game has ended!";

   if (winners.size() > 0) {
      os << "\n\nThe following players won:\n";

      for (auto it = winners.begin(); it != winners.end(); ) {
         auto& pl = **it;
         os << "   " << game_log().get_name(pl) << ", the " << full_name(pl.role());
         os << ((++it == winners.end()) ? "." : ",\n");
      }
   } else {
      os << "\n\nNobody won.";
   }

   if (losers.size() > 0) {
      os << "\n\nCommiserations go out to:\n";

      for (auto it = losers.begin(); it != losers.end(); ) {
         auto& pl = **it;
         os << "   " << game_log().get_name(pl) << ", the " << full_name(pl.role());
         os << ((++it == losers.end()) ? "." : ",\n");
      }
   } else {
      os << "\n\nNobody lost.";
   }

   os << "^h\n\nTo return to the setup screen, enter ^cend^/.";
}

maf::Help_Screen * maf::screen::Game_Ended::get_help_screen() const {
   return nullptr;
}
