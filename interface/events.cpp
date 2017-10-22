#include <sstream>

#include "console.hpp"
#include "error.hpp"
#include "events.hpp"
#include "names.hpp"

void maf::Event::write_summary(std::ostream &os) const {
   // By default, write nothing.
}

void maf::Event::write_help(std::ostream &os) const {
   os << "^HMissing Help Screen^/^hNo help has been written for the current game event.\n(this counts as a bug!)\n\nEnter ^cok^/ to leave this screen.";
}

void maf::Event::kick_player(const std::string& pl_name, Game_log& glog) {
   try {
      const Player& pl = glog.find_player(pl_name);

      try {
         glog.kick_player(pl.id());
         glog.advance();
      } catch (error::game_has_ended) {
         std::stringstream err;
         err << "^HKick failed!^h";
         err << glog.get_name(pl);
         err << " could not be kicked from the game, because the game has already ended.";
         throw error::unresolved_input(err);
      } catch (error::bad_timing) {
         std::stringstream err;
         err << "^HKick failed!^h";
         err << "Players can only be kicked from the game during the day.";
         throw error::unresolved_input(err);
      } catch (error::repeat_action) {
         std::stringstream err;
         err << "^HKick failed!^h";
         err << glog.get_name(pl);
         err << " has already been kicked from the game";
         throw error::unresolved_input(err);
      }
   } catch (error::missing_player) {
      auto pl_name_esc = copy_with_escaped_style_codes(pl_name);

      std::stringstream err;
      err << "^HKick failed!^h";
      err << "A player named ^c";
      err << pl_name_esc;
      err << "^h could not be found.";
      throw error::unresolved_input(err);
   }
}

void maf::Player_given_initial_role::do_commands(const std::vector<std::string> & commands) {
   if (commands_match(commands, {"ok"})) {
      if (_is_private) {
         game_log().advance();
      } else {
         _is_private = true;
      }
   } else {
      throw error::bad_commands();
   }
}

void maf::Player_given_initial_role::write_full(std::ostream &os) const {
   auto & glog = game_log();
   auto & pl_name = glog.get_name(*_p);
   auto rl_name = full_name(*_r);
   auto rl_alias = _r->alias();

   os << "^G" << pl_name << "'s Role^/";

   if (_is_private) {
      os << pl_name << ", your role is the "  << rl_name << ".";

      if (_w != nullptr) {
         /* FIXME */
         os << "\nYou were randomly given this role from the ^c";
         os << _w->alias();
         os << "^/ wildcard.";
      }

      os << "^h\n\nTo see a full description of your role, enter ^chelp r " << rl_alias << "^/.^/";
   } else {
      os << pl_name << ", you are about to be shown your role.";
   }
}

void maf::Player_given_initial_role::write_summary(std::ostream &os) const {
   os << game_log().get_name(*_p) << " played as the " << full_name(*_r) << ".";
}

void maf::Time_changed::do_commands(const std::vector<std::string> & commands) {
   if (commands_match(commands, {"ok"})) {
      game_log().advance();
   }
   else {
      throw error::bad_commands();
   }
}

void maf::Time_changed::write_full(std::ostream & os) const {
   switch (time) {
      case Time::day: {
         os << "^GDay " << date << "^/";
         os << "^iDawn breaks, and dim sunlight beams onto the weary townsfolk...\n\n^/It is now day ";
         os << date;
         os << ". Anybody still asleep can wake up.";
         break;
      }

      case Time::night: {
         os << "^GNight " << date << "^/";
         os << "^iAs darkness sets in, the townsfolk return to the comforts of their shelters...\n\n^/It is now night ";
         os << date;
         os << ". Everybody still in the game should go to sleep.";
         break;
      }
   }
}

void maf::Time_changed::write_summary(std::ostream &os) const {
   switch (time) {
      case Time::day:
         os << "Day " << date << " began.";
         break;
      case Time::night:
         os << "Night " << date << " began.";
         break;
   }
}

void maf::Obituary::do_commands(const std::vector<std::string> & commands) {
   if (commands_match(commands, {"ok"})) {
      if (_deaths_index + 1 < _deaths.size()) {
         ++_deaths_index;
      } else {
         game_log().advance();
      }
   } else {
      throw error::bad_commands();
   }
}

void maf::Obituary::write_full(std::ostream &os) const {
   os << "^GObituary^/";

   if (_deaths_index < 0) {
      if (_deaths.size() == 0) {
         os << "Nobody died during the night.";
      } else {
         /* FIXME: reword to remove use of "us". */
         os << "It appears that " << _deaths.size() << " of us did not survive the night...";
      }
   } else {
      const Player& death = *_deaths[_deaths_index];

      os << game_log().get_name(death) << " died during the night!";
      if (death.is_haunted()) {
         os << "\n\nA slip of paper was found by their bed. On it has been written the name \""
         << game_log().get_name(*death.haunter())
         << "\" over and over...";
      }
   }
}

void maf::Obituary::write_summary(std::ostream &os) const {
   bool write_nl = false;

   for (auto& p_ref: _deaths) {
      if (write_nl) os << '\n';
      os << game_log().get_name(*p_ref) << " died during the night.";
      write_nl = true;
   }
}

void maf::Town_meeting::do_commands(const std::vector<std::string> & commands) {
   auto& glog = game_log();

   if (commands_match(commands, {"kick", ""})) {
      auto& pl_name = commands[1];
      kick_player(pl_name, glog);
   }
   else if (commands_match(commands, {"duel", "", ""})) {
      const Player & caster = glog.find_player(commands[1]);
      const Player & target = glog.find_player(commands[2]);

      glog.stage_duel(caster.id(), target.id());
      glog.advance();
   }
   else if (_lynch_can_occur) {
      if (commands_match(commands, {"vote", "", ""})) {
         const Player & voter = glog.find_player(commands[1]);
         const Player & target = glog.find_player(commands[2]);

         glog.cast_lynch_vote(voter.id(), target.id());
         glog.advance();
      }
      else if (commands_match(commands, {"abstain", ""})) {
         const Player &voter = glog.find_player(commands[1]);

         glog.clear_lynch_vote(voter.id());
         glog.advance();
      }
      else if (commands_match(commands, {"lynch"})) {
         glog.process_lynch_votes();
         glog.advance();
      }
      else {
         throw error::bad_commands();
      }
   }
   else {
      if (commands_match(commands, {"night"})) {
         glog.begin_night();
         glog.advance();
      }
      else {
         throw error::bad_commands();
      }
   }
}

void maf::Town_meeting::write_full(std::ostream &os) const {
   /* FIXME: add in proper content. */

   os << "^GDay " << _date << "^/";

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

      os << "\n\nAs it stands, "
      << (_next_lynch_victim ? game_log().get_name(*_next_lynch_victim) : "nobody")
      << " will be lynched.^h\n\nEnter ^clynch^/ to submit the current lynch votes. Daytime abilities may also be used at this point.";
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

void maf::Town_meeting::write_summary(std::ostream &os) const {
   if (_recent_vote_caster) {
      if (_recent_vote_target) {
         os << game_log().get_name(*_recent_vote_caster) << " voted to lynch " << game_log().get_name(*_recent_vote_target) << ".";
      } else {
         os << game_log().get_name(*_recent_vote_caster) << " chose not to vote.";
      }
   }
}

void maf::Player_kicked::do_commands(const std::vector<std::string> & commands) {
   if (commands_match(commands, {"ok"})) {
      game_log().advance();
   }
   else {
      throw error::bad_commands();
   }
}

void maf::Player_kicked::write_full(std::ostream &os) const {
   os << "^G" << game_log().get_name(*player) << " kicked^/";
   os << game_log().get_name(*player) << " was kicked from the game!\n";
   os << "They were the " << full_name(player->role()) << ".";
}

void maf::Player_kicked::write_summary(std::ostream &os) const {
   os << game_log().get_name(*player) << " was kicked.";
}

void maf::Lynch_result::do_commands(const std::vector<std::string> & commands) {
   if (commands_match(commands, {"ok"})) {
      game_log().advance();
   }
   else {
      throw error::bad_commands();
   }
}

void maf::Lynch_result::write_full(std::ostream &os) const {
   os << "^GLynch Result^/";

   if (victim) {
      auto & victim_name = game_log().get_name(*victim);

      os << victim_name << " was lynched!\n";

      if (victim_role) {
         os << "They were a " << full_name(victim_role->id()) << ".";
         if (victim_role->is_troll()) {
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

void maf::Lynch_result::write_summary(std::ostream &os) const {
   os << (victim ? game_log().get_name(*victim) : "Nobody") << " was lynched.";
}

void maf::Duel_result::do_commands(const std::vector<std::string> & commands) {
   if (commands_match(commands, {"ok"})) {
      game_log().advance();
   }
   else {
      throw error::bad_commands();
   }
}

void maf::Duel_result::write_full(std::ostream &os) const {
   auto& winner = (caster->is_alive() ? caster : target);
   auto& loser = (caster->is_alive() ? target : caster);

   auto& glog = game_log();
   auto& caster_name = glog.get_name(*caster);
   auto& target_name = glog.get_name(*target);
   auto& winner_name = glog.get_name(*winner);
   auto& loser_name = glog.get_name(*loser);

   os << "^GDuel^/";
   os << caster_name;
   os << " has challenged ";
   os << target_name;
   os << " to a duel!^i\n\nThe pistols are loaded, and the participants take ten paces in opposite directions...\n\n3... 2... 1... BANG!!^/\n\nThe lifeless body of ";
   os << loser_name;
   os << " falls to the ground. ";
   os << winner_name;
   os << " lets out a sigh of relief.";

   if (!winner->is_present()) {
      os << "\n\nWith that, "
      << winner_name
      << " throws their gun to the ground and flees from the village.";
   }

   os << "^h\n\nWhen you have finished with this screen, enter ^cok^/.";
}

void maf::Duel_result::write_summary(std::ostream &os) const {
   auto & glog = game_log();
   auto & caster_name = glog.get_name(*caster);
   auto & target_name = glog.get_name(*target);

   if (caster->is_alive()) { // FIXME: unstable code, as the caster may not be alive later in the game if the rules change, yet still won the duel
      os << caster_name << " won a duel against " << target_name << ".";
   } else {
      os << caster_name << " lost a duel against " << target_name << ".";
   }
}

void maf::Choose_fake_role::do_commands(const std::vector<std::string> & commands) {
   auto & glog = game_log();

   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         glog.advance();
      } else {
         throw error::bad_commands();
      }
   } else if (_fake_role) {
      if (commands_match(commands, {"ok"})) {
         _go_to_sleep = true;
      } else {
         throw error::bad_commands();
      }
   } else {
      if (commands_match(commands, {"choose", ""})) {
         auto & fake_role_alias = commands[1];
         auto & fake_role = glog.game().rulebook().get_role(fake_role_alias);

         _fake_role = &fake_role;
         glog.choose_fake_role(_player->id(), fake_role.id());
      } else {
         throw error::bad_commands();
      }
   }
}

void maf::Choose_fake_role::write_full(std::ostream &os) const {
   auto & pl_name = game_log().get_name(*_player);

   os << "^GChoose Fake Role^/";

   if (_go_to_sleep) {
      os << pl_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else if (_fake_role) {
      auto fake_role_name = full_name(*_fake_role);
      auto fake_role_alias = _fake_role->alias();

      os << pl_name << ", you have been given the " << fake_role_name << " as your fake role.\n\nYou must pretend that this is your real role for the remainder of the game. Breaking this rule will result in you being kicked from the game!\n\nNow would be a good time to study your fake role.^h\n\nEnter ^chelp r " << fake_role_alias << "^/ to see more information about your fake role.\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << pl_name << " needs to be given a fake role, which they must pretend is their true role for the rest of the game.^h\n\nIf they break the rules by contradicting their fake role, then they should be kicked from the game by entering ^ckick " << pl_name << "^/ during the day.\n\nTo choose the role with alias ^cA^/, enter ^cchoose A^/.";
   }
}

void maf::Choose_fake_role::write_summary(std::ostream &os) const {
   if (_fake_role) {
      os << game_log().get_name(*_player) << " was given the " << full_name(*_fake_role) << " as a fake role.";
   }
}

void maf::Mafia_meeting::do_commands(const std::vector<std::string> & commands) {
   auto & glog = game_log();

   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         glog.advance();
      } else {
         throw error::bad_commands();
      }
   } else if (_initial) {
      if (commands_match(commands, {"ok"})) {
         _go_to_sleep = true;
      } else {
         throw error::bad_commands();
      }
   } else {
      if (commands_match(commands, {"kill", "", ""})) {
         const Player &caster = glog.find_player(commands[1]);
         const Player &target = glog.find_player(commands[2]);

         glog.cast_mafia_kill(caster.id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         /* FIXME: show "confirm skip?" screen. */
         glog.skip_mafia_kill();
         _go_to_sleep = true;
      } else {
         throw error::bad_commands();
      }
   }
}

void maf::Mafia_meeting::write_full(std::ostream &os) const {
   /* FIXME */
   os << "^GMafia Meeting^/";

   if (_go_to_sleep) {
      os << "The mafia have nothing more to discuss for now, and should go back to sleep.^h\n\nEnter ^cok^/ when you are ready to continue.";
   } else if (_initial) {
      os << "The mafia consists of:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         auto& p_ref = *it;
         os << "   " << game_log().get_name(*p_ref) << ", the " << full_name(p_ref->role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nThere is not enough time left to organise a murder.";
   } else {
      os << "Seated around a polished walnut table are:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         auto& p_ref = *it;
         os << "   " << game_log().get_name(*p_ref) << ", the " << full_name(p_ref->role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nThe mafia are ready to choose their next victim.^h\n\nEntering ^ckill A B^/ will make player ^cA^/ attempt to kill player ^cB^/. Player ^cA^/ must be a member of the mafia.\n\nIf the mafia have chosen not to kill anybody this night, enter ^cskip^/.";
   }
}

void maf::Kill_use::do_commands(const std::vector<std::string> &commands) {
   auto & glog = game_log();

   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         glog.advance();
      } else {
         throw error::bad_commands();
      }
   } else {
      if (commands_match(commands, {"kill", ""})) {
         auto & target_name = commands[1];
         auto & target = glog.find_player(target_name);

         glog.cast_kill(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         glog.skip_kill(_caster->id());
         _go_to_sleep = true;
      } else {
         throw error::bad_commands();
      }
   }
}

void maf::Kill_use::write_full(std::ostream &os) const {
   auto & glog = game_log();
   auto & caster_name = glog.get_name(*_caster);

   os << "^GKill Use^/";

   if (_go_to_sleep) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to kill somebody this night.^h\n\nEnter ^ckill A^/ to kill player ^cA^/, or enter ^cskip^/ if you don't wish to kill anybody.";
   }
}

void maf::Heal_use::do_commands(const std::vector<std::string> & commands) {
   auto & glog = game_log();

   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         glog.advance();
      } else {
         throw error::bad_commands();
      }
   } else {
      if (commands_match(commands, {"heal", ""})) {
         auto & target_name = commands[1];
         auto & target = glog.find_player(target_name);

         glog.cast_heal(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         glog.skip_heal(_caster->id());
         _go_to_sleep = true;
      } else {
         throw error::bad_commands();
      }
   }
}

void maf::Heal_use::write_full(std::ostream &os) const {
   auto & glog = game_log();
   auto & caster_name = glog.get_name(*_caster);

   os << "^GHeal Use^/";

   if (_go_to_sleep) {
      os << caster_name << " should now go back to sleep.";
      os << "^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to heal somebody this night.";
      os << "^h\n\nEnter ^cheal A^/ to heal player ^cA^/, or enter ^cskip^/ if you don't wish to heal anybody.";
   }
}

void maf::Investigate_use::do_commands(const std::vector<std::string> & commands) {
   auto & glog = game_log();

   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         glog.advance();
      } else {
         throw error::bad_commands();
      }
   } else {
      if (commands_match(commands, {"check", ""})) {
         auto & target_name = commands[1];
         auto & target = glog.find_player(target_name);

         glog.cast_investigate(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         glog.skip_investigate(_caster->id());
         _go_to_sleep = true;
      } else {
         throw error::bad_commands();
      }
   }
}

void maf::Investigate_use::write_full(std::ostream &os) const {
   auto & glog = game_log();
   auto & caster_name = glog.get_name(*_caster);

   os << "^GInvestigation^/";

   if (_go_to_sleep) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to investigate somebody this night.^h\n\nEnter ^ccheck A^/ to investigate player ^cA^/, or enter ^cskip^/ if you don't wish to investigate anybody.";
   }
}

void maf::Peddle_use::do_commands(const std::vector<std::string> & commands) {
   auto & glog = game_log();

   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         glog.advance();
      } else {
         throw error::bad_commands();
      }
   } else {
      if (commands_match(commands, {"target", ""})) {
         auto & target_name = commands[1];
         auto & target = glog.find_player(target_name);

         glog.cast_peddle(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         glog.skip_peddle(_caster->id());
         _go_to_sleep = true;
      } else {
         throw error::bad_commands();
      }
   }
}

void maf::Peddle_use::write_full(std::ostream &os) const {
   auto & glog = game_log();
   auto & caster_name = glog.get_name(*_caster);

   os << "^GPeddle^/";

   if (_go_to_sleep) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you can choose to peddle drugs to somebody this night.^h\n\nEnter ^ctarget A^/ to peddle drugs to player ^cA^/, or enter ^cskip^/ if you don't wish to peddle drugs to anybody.";
   }
}

void maf::Boring_night::do_commands(const std::vector<std::string> & commands) {
   if (commands_match(commands, {"ok"})) {
      game_log().advance();
   }
   else {
      throw error::bad_commands();
   }
}

void maf::Boring_night::write_full(std::ostream &os) const {
   /* FIXME: show current date in title. (e.g. "Night 1") */

   os << "^GCalm Night^/^iIt is warm outside. The moon shines brightly. The gentle chirping of crickets is carried by a pleasant breeze...^/\n\nNothing of interest happened this night, although you should still wait a few moments before continuing, to maintain the illusion that something happened.^h\n\nEnter ^cok^/ to continue.";
}

void maf::Investigation_result::do_commands(const std::vector<std::string> & commands) {
   auto & glog = game_log();

   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         glog.advance();
      }
      else {
         throw error::bad_commands();
      }
   } else {
      if (commands_match(commands, {"ok"})) {
         _go_to_sleep = true;
      }
      else {
         throw error::bad_commands();
      }
   }
}

void maf::Investigation_result::write_full(std::ostream &os) const {
   auto & glog = game_log();
   auto & caster_name = glog.get_name(investigation.caster());
   auto & target_name = glog.get_name(investigation.target());

   os << "^GInvestigation Result^/";

   if (_go_to_sleep) {
      os << caster_name << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   } else {
      os << caster_name << ", you have completed your investigation of " << target_name << ".\n\n";

      if (investigation.result()) {
         os << target_name << " was behaving very suspiciously this night!";
      } else {
         os << "The investigation was fruitless. " << target_name << " appears to be innocent.";
      }

      os << "^h\n\nWhen you are ready, enter ^cok^/ to continue.";
   }
}

void maf::Investigation_result::write_summary(std::ostream &os) const {
   os << game_log().get_name(investigation.caster())
   << " decided that "
   << game_log().get_name(investigation.target())
   << " was "
   << (investigation.result() ? "suspicious" : "innocent")
   << ".";
}

void maf::Game_ended::do_commands(const std::vector<std::string> & commands) {
   throw error::bad_commands();
}

void maf::Game_ended::write_full(std::ostream &os) const {
   std::vector<const Player *> winners{};
   std::vector<const Player *> losers{};
   for (const Player &player: game_log().game().players()) {
      if (player.has_won()) {
         winners.push_back(&player);
      } else {
         losers.push_back(&player);
      }
   }

   os << "^GGame Over^/The game has ended!";

   if (winners.size() > 0) {
      os << "\n\nThe following players won:\n";

      for (auto it = winners.begin(); it != winners.end(); ) {
         const Player &player = **it;
         os << "   " << game_log().get_name(player) << ", the " << full_name(player.role());
         os << ((++it == winners.end()) ? "." : ",\n");
      }
   } else {
      os << "\n\nNobody won.";
   }

   if (losers.size() > 0) {
      os << "\n\nCommiserations go out to:\n";

      for (auto it = losers.begin(); it != losers.end(); ) {
         const Player &player = **it;
         os << "   " << game_log().get_name(player) << ", the " << full_name(player.role());
         os << ((++it == losers.end()) ? "." : ",\n");
      }
   } else {
      os << "\n\nNobody lost.";
   }

   os << "^h\n\nTo return to the setup screen, enter ^cend^/.";
}

void maf::Game_ended::write_summary(std::ostream &os) const {
   for (auto it = game_log().game().players().begin(); it != game_log().game().players().end(); ) {
      const Player &player = *it;
      os << game_log().get_name(player) << (player.has_won() ? " won." : " lost.");
      if (++it != game_log().game().players().end()) {
         os << "\n";
      }
   }
}
