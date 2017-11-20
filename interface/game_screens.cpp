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

bool maf::Game_Screen::handle_commands(const std::vector<std::string_view> & commands) {
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

bool maf::screen::Player_Given_Initial_Role::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

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
   std::string_view pl_name = glog.get_name(*_p_ref);

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

bool maf::screen::Time_Changed::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

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

bool maf::screen::Obituary::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

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

bool maf::screen::Town_Meeting::handle_commands(const std::vector<std::string_view> & commands) {
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

bool maf::screen::Player_Kicked::handle_commands(const std::vector<std::string_view> & commands) {
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
   std::string_view pl_name = glog.get_name(*_pl_ref);
   std::string r_name = full_name(_pl_ref->role());

   os << "^T" << pl_name << " kicked^/";
   os << pl_name << " was kicked from the game!\n";
   os << "They were the " << r_name << ".";
}

maf::Help_Screen * maf::screen::Player_Kicked::get_help_screen() const {
   // FIXME

   return nullptr;
}



/*
 * maf::screen::Lynch_Result
 */

bool maf::screen::Lynch_Result::handle_commands(const std::vector<std::string_view> & commands) {
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

void maf::screen::Lynch_Result::write(std::ostream & os) const {
   os << "^TLynch Result^/";

   if (_victim_ptr) {
      auto& victim = *_victim_ptr;
      std::string_view victim_name = game_log().get_name(*_victim_ptr);

      os << victim_name << " was lynched!\n";

      if (_victim_role_ptr) {
         auto& victim_role = *_victim_role_ptr;
         auto victim_role_name = full_name(victim_role);

         os << "They were a " << victim_role_name << ".";
         if (victim.is_troll()) {
            os << "^i\n\nA chill blows through the air. The townsfolk who voted to lynch ";
            os << victim_name;
            os << " look nervous...";
         }
      } else {
         os << "Their role could not be determined.";
      }
   } else {
      os << "Nobody was lynched today.";
   }
}

maf::Help_Screen * maf::screen::Lynch_Result::get_help_screen() const {
   // FIXME

   return nullptr;
}



/*
 * maf::screen::Duel_Result
 */

bool maf::screen::Duel_Result::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   auto& glog = this->game_log();

   if (commands_match(commands, {"ok"})) {
      glog.advance();
   } else {
      return false;
   }

   return true;
}

void maf::screen::Duel_Result::write(std::ostream & os) const {
   auto& caster = *_caster_ref;
   auto& target = *_target_ref;

   auto& winner = (caster.is_alive() ? caster : target);
   auto& loser = (caster.is_alive() ? target : caster);

   auto& glog = game_log();
   std::string_view caster_name = glog.get_name(caster);
   std::string_view target_name = glog.get_name(target);
   std::string_view winner_name = glog.get_name(winner);
   std::string_view loser_name = glog.get_name(loser);

   os << "^TDuel^/";
   os << caster_name;
   os << " has challenged ";
   os << target_name;
   os << " to a duel!^i\n\nThe pistols are loaded, and the participants take ten paces in opposite directions...\n\n3... 2... 1... BANG!!^/\n\nThe lifeless body of ";
   os << loser_name;
   os << " falls to the ground. ";
   os << winner_name;
   os << " lets out a sigh of relief.";

   if (!winner.is_present()) {
      os << "\n\nWith that, ";
      os << winner_name;
      os << " throws their gun to the ground and flees from the village.";
   }

   os << "^h\n\nWhen you have finished with this screen, enter ^cok^/.";
}

maf::Help_Screen * maf::screen::Duel_Result::get_help_screen() const {
   // FIXME
   return nullptr;
}



/*
 * maf::screen::Choose_Fake_Role
 */

bool maf::screen::Choose_Fake_Role::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   //auto& con = this->console();
   auto& glog = this->game_log();

   if (commands_match(commands, {"choose", ""})) {
      if (_page == 0) {
         auto& alias = commands[1];
         auto& role = glog.game().rulebook().get_role(alias);

         glog.choose_fake_role(_player_ref->id(), role.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"ok"})) {
      if (_page == 1) {
         _page ++;
      } else if (_page == 2) {
         glog.advance();
      } else {
         return false;
      }
   } else {
      return false;
   }

   return true;
}

void maf::screen::Choose_Fake_Role::write(std::ostream & os) const {
   auto& glog = this->game_log();
   const Player & pl = *_player_ref;
   std::string_view pl_name = glog.get_name(pl);

   os << "^TChoose Fake Role^/";

   if (_page == 2) {
      os << pl_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else if (_page == 1) {
      const Role & fake_role = *pl.fake_role();
      auto fr_name = full_name(fake_role);
      auto fr_alias = fake_role.alias();

      os << pl_name << ", you have been given the " << fr_name << " as your fake role.\n\nYou must pretend that this is your real role for the remainder of the game. Breaking this rule will result in you being kicked from the game!\n\nNow would be a good time to study your fake role.^h\n\nEnter ^chelp r " << fr_alias << "^/ to see more information about your fake role.\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << pl_name << " needs to be given a fake role, which they must pretend is their true role for the rest of the game.^h\n\nIf they break the rules by contradicting their fake role, then they should be kicked from the game by entering ^ckick " << pl_name << "^/ during the day.\n\nTo choose the role with alias ^cA^/, enter ^cchoose A^/.";
   }
}

maf::Help_Screen * maf::screen::Choose_Fake_Role::get_help_screen() const {
   // FIXME
   return nullptr;
}



/*
 * maf::screen::Mafia_Meeting
 */

bool maf::screen::Mafia_Meeting::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   auto& glog = game_log();

   if (commands_match(commands, {"ok"})) {
      if (_initial) {
         if (_page == 1) {
            glog.advance();
         } else {
            _page ++;
         }
      } else {
         if (_page == 1) glog.advance();
         else return false;
      }
   } else if (commands_match(commands, {"kill", "", ""})) {
      if (!_initial) {
         const Player & caster = glog.find_player(commands[1]);
         const Player & target = glog.find_player(commands[2]);

         glog.cast_mafia_kill(caster.id(), target.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"skip"})) {
      if (!_initial) {
         // FIXME: show "confirm skip?" screen.
         glog.skip_mafia_kill();
         _page ++;
      } else {
         return false;
      }
   } else {
      return false;
   }

   return true;
}

void maf::screen::Mafia_Meeting::write(std::ostream & os) const {
   // FIXME

   os << "^TMafia Meeting^/";

   if (_page == 1) {
      os << "The mafia have nothing more to discuss for now, and should go back to sleep.^h\n\nEnter ^cok^/ when you are ready to continue.";
   } else if (_initial) {
      os << "The mafia consists of:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         const Player & pl = **it;

         os << "   " << game_log().get_name(pl) << ", the " << full_name(pl.role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nThere is not enough time left to organise a murder.";
   } else {
      os << "Seated around a polished walnut table are:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         const Player & pl = **it;

         os << "   " << game_log().get_name(pl) << ", the " << full_name(pl.role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nThe mafia are ready to choose their next victim.^h\n\nEntering ^ckill A B^/ will make player ^cA^/ attempt to kill player ^cB^/. Player ^cA^/ must be a member of the mafia.\n\nIf the mafia have chosen not to kill anybody this night, enter ^cskip^/.";
   }
}

maf::Help_Screen * maf::screen::Mafia_Meeting::get_help_screen() const {
   // FIXME
   return nullptr;
}



/*
 * maf::screen::Kill_Use
 */

bool maf::screen::Kill_Use::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   auto& glog = game_log();

   if (commands_match(commands, {"kill", ""})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;
         const Player & target = glog.find_player(commands[1]);

         glog.cast_kill(caster.id(), target.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"skip"})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;

         glog.skip_kill(caster.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"ok"})) {
      if (_page == 1) glog.advance();
      else return false;
   } else {
      return false;
   }

   return true;
}

void maf::screen::Kill_Use::write(std::ostream & os) const {
   auto& glog = game_log();

   const Player & caster = *_caster_ref;
   std::string_view caster_name = glog.get_name(caster);

   os << "^TKill Use^/";

   if (_page == 1) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to kill somebody this night.^h\n\nEnter ^ckill A^/ to kill player ^cA^/, or enter ^cskip^/ if you don't wish to kill anybody.";
   }
}

maf::Help_Screen * maf::screen::Kill_Use::get_help_screen() const {
   // FIXME: decide whether or not to include a help screen
   return nullptr;
}



/*
 * maf::screen::Heal_Use
 */

bool maf::screen::Heal_Use::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   auto& glog = game_log();

   if (commands_match(commands, {"heal", ""})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;
         const Player & target = glog.find_player(commands[1]);

         glog.cast_heal(caster.id(), target.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"skip"})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;

         glog.skip_heal(caster.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"ok"})) {
      if (_page == 1) glog.advance();
      else return false;
   } else {
      return false;
   }

   return true;
}

void maf::screen::Heal_Use::write(std::ostream & os) const {
   auto& glog = game_log();

   const Player & caster = *_caster_ref;
   std::string_view caster_name = glog.get_name(caster);

   os << "^THeal Use^/";

   if (_page == 1) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to heal somebody this night.^h\n\nEnter ^cheal A^/ to heal player ^cA^/, or enter ^cskip^/ if you don't wish to heal anybody.";
   }
}

maf::Help_Screen * maf::screen::Heal_Use::get_help_screen() const {
   // FIXME: decide whether or not to include a help screen
   return nullptr;
}



/*
 * maf::screen::Investigate_Use
 */

bool maf::screen::Investigate_Use::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   auto& glog = game_log();

   if (commands_match(commands, {"check", ""})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;
         const Player & target = glog.find_player(commands[1]);

         glog.cast_investigate(caster.id(), target.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"skip"})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;

         glog.skip_investigate(caster.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"ok"})) {
      if (_page == 1) glog.advance();
      else return false;
   } else {
      return false;
   }

   return true;
}

void maf::screen::Investigate_Use::write(std::ostream & os) const {
   auto& glog = game_log();

   const Player & caster = *_caster_ref;
   std::string_view caster_name = glog.get_name(caster);

   os << "^TInvestigation^/";

   if (_page == 1) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to investigate somebody this night.^h\n\nEnter ^ccheck A^/ to investigate player ^cA^/, or enter ^cskip^/ if you don't wish to investigate anybody.";
   }
}

maf::Help_Screen * maf::screen::Investigate_Use::get_help_screen() const {
   // FIXME: decide whether or not to include a help screen
   return nullptr;
}



/*
 * maf::screen::Peddle_Use
 */

bool maf::screen::Peddle_Use::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   auto& glog = game_log();

   if (commands_match(commands, {"target", ""})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;
         const Player & target = glog.find_player(commands[1]);

         glog.cast_peddle(caster.id(), target.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"skip"})) {
      if (_page == 0) {
         const Player & caster = *_caster_ref;

         glog.skip_peddle(caster.id());
         _page ++;
      } else {
         return false;
      }
   } else if (commands_match(commands, {"ok"})) {
      if (_page == 1) glog.advance();
      else return false;
   } else {
      return false;
   }

   return true;
}

void maf::screen::Peddle_Use::write(std::ostream & os) const {
   auto& glog = game_log();

   const Player & caster = *_caster_ref;
   std::string_view caster_name = glog.get_name(caster);

   os << "^TPeddle^/";

   if (_page == 1) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to peddle drugs to somebody this night.^h\n\nEnter ^ctarget A^/ to peddle drugs to player ^cA^/, or enter ^cskip^/ if you don't wish to peddle drugs to anybody.";
   }
}

maf::Help_Screen * maf::screen::Peddle_Use::get_help_screen() const {
   // FIXME: decide whether or not to include a help screen
   return nullptr;
}



/*
 * maf::screen::Boring_Night
 */

bool maf::screen::Boring_Night::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

   auto& glog = this->game_log();

   if (commands_match(commands, {"ok"})) {
      glog.advance();
   } else {
      return false;
   }

   return true;
}

void maf::screen::Boring_Night::write(std::ostream & os) const {
   // FIXME: show current date in title. (e.g. "Night 1")

   os << "^TCalm Night^/^iIt is warm outside. The moon shines brightly. The gentle chirping of crickets is carried by a pleasant breeze...^/\n\nNothing of interest happened this night, although you should still wait a few moments before continuing, to maintain the illusion that something happened.^h\n\nEnter ^cok^/ to continue.";
}

maf::Help_Screen * maf::screen::Boring_Night::get_help_screen() const {
   // FIXME
   return nullptr;
}



/*
 * maf::screen::Investigation_Result
 */

bool maf::screen::Investigation_Result::handle_commands(const std::vector<std::string_view> & commands) {
   if (Game_Screen::handle_commands(commands)) return true;

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
   auto& glog = this->game_log();
   std::string_view caster_name = glog.get_name(_inv.caster());
   std::string_view target_name = glog.get_name(_inv.target());

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

bool maf::screen::Game_Ended::handle_commands(const std::vector<std::string_view> & commands) {
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
