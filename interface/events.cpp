#include "console.hpp"
#include "events.hpp"
#include "names.hpp"

void maf::Event::write_summary(std::ostream &os) const {
   // By default, write nothing.
}

void maf::Event::write_help(std::ostream &os) const {
   os << "^HMissing Help Screen^hNo help has been written for the current game event.\n(this counts as a bug!)\n\nEnter ^cok^h to leave this screen.";
}

void maf::Player_given_initial_role::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      if (_is_private) {
         game_log.advance();
      } else {
         _is_private = true;
      }
   }
   else {
      throw Bad_commands{};
   }
}

void maf::Player_given_initial_role::write_full(std::ostream &os) const {
   auto & glog = game_log();

   if (_is_private) {
      os << "^G" << glog.get_name(*_p) << "'s Role^g"
      << glog.get_name(*_p) << ", your role is the "
      << full_name(*_r) << ".";

      if (_w != nullptr) {
         /* fix-me */
         os << "\nYou were randomly given this role from the ^c"
         << _w->alias()
         << "^g wildcard.";
      }

      os << "^h\n\nTo see a full description of your role, enter ^chelp r "
      << _r->alias()
      << "^h.";
   } else {
      os << "^G" << glog.get_name(*_p) << "'s Role^g"
      << glog.get_name(*_p) << ", you are about to be shown your role.";
   }
}

void maf::Player_given_initial_role::write_summary(std::ostream &os) const {
   os << game_log().get_name(*_p) << " played as the " << full_name(*_r) << ".";
}

void maf::Time_changed::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void maf::Time_changed::write_full(std::ostream &os) const {
   switch (time) {
      case Time::day: {
         os << "^GDay "
         << date
         << "^iDawn breaks, and dim sunlight beams onto the weary townsfolk...\n\n^gIt is now day "
         << date
         << ". Anybody still asleep can wake up.";
      }
         break;

      case Time::night: {
         os << "^GNight "
         << date
         << "^iAs darkness sets in, the townsfolk return to the comforts of their shelters...\n\n^gIt is now night "
         << date
         << ". Everybody still in the game should go to sleep.";
      }
         break;
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

void maf::Obituary::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      if (_deaths_index + 1 < _deaths.size()) {
         ++_deaths_index;
      } else {
         game_log.advance();
      }
   } else {
      throw Bad_commands{};
   }
}

void maf::Obituary::write_full(std::ostream &os) const {
   if (_deaths_index < 0) {
      if (_deaths.size() == 0) {
         os << "^GObituary^gNobody died during the night.";
      } else {
         /* fix-me: reword to remove use of "us". */
         os << "^GObituary^gIt appears that " << _deaths.size() << " of us did not survive the night...";
      }
   } else {
      const Player& death = *_deaths[_deaths_index];

      os << "^GObituary^g" << game_log().get_name(death) << " died during the night!";
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

void maf::Town_meeting::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_lynch_can_occur) {
      if (commands_match(commands, {"kick", ""})) {
         const Player &player = game_log.find_player(commands[1]);

         game_log.kick_player(player.id());
         game_log.advance();
      }
      else if (commands_match(commands, {"vote", "", ""})) {
         const Player &voter = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);

         game_log.cast_lynch_vote(voter.id(), target.id());
         game_log.advance();
      }
      else if (commands_match(commands, {"abstain", ""})) {
         const Player &voter = game_log.find_player(commands[1]);

         game_log.clear_lynch_vote(voter.id());
         game_log.advance();
      }
      else if (commands_match(commands, {"lynch"})) {
         game_log.process_lynch_votes();
         game_log.advance();
      }
      else if (commands_match(commands, {"duel", "", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);

         game_log.stage_duel(caster.id(), target.id());
         game_log.advance();
      }
      else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"night"})) {
         game_log.begin_night();
         game_log.advance();
      }
      else if (commands_match(commands, {"kick", ""})) {
         const Player &player = game_log.find_player(commands[1]);

         game_log.kick_player(player.id());
         game_log.advance();
      }
      else if (commands_match(commands, {"duel", "", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);

         game_log.stage_duel(caster.id(), target.id());
         game_log.advance();
      }
      else {
         throw Bad_commands{};
      }
   }
}

void maf::Town_meeting::write_full(std::ostream &os) const {
   /* fix-me: add in proper content. */

   if (_lynch_can_occur) {
      os << "^GDay "
      << _date
      << "^gGathered outside the town hall are:\n";

      for (auto it = _players.begin(); it != _players.end(); ) {
         auto& p_ref = *it;
         os << "   "
         << game_log().get_name(*p_ref);
         if (p_ref->lynch_vote() != nullptr) {
            os << ", voting to lynch "
            << game_log().get_name(*(p_ref->lynch_vote()));
         }
         os << ((++it == _players.end()) ? "." : ",\n");
      }

      os << "\n\nAs it stands, "
      << (_next_lynch_victim ? game_log().get_name(*_next_lynch_victim) : "nobody")
      << " will be lynched.^h\n\nEnter ^clynch^h to submit the current lynch votes. Daytime abilities may also be used at this point.";
   } else {
      os << "^GDay "
      << _date
      << "^iWith little time left in the day, the townsfolk prepare themselves for another night of uncertainty...\n\n^gGathered outside the town hall are:\n";

      for (auto it = _players.begin(); it != _players.end(); ) {
         auto& p_ref = *it;
         os << "   ";
         os << game_log().get_name(*p_ref);
         os << ((++it == _players.end()) ? "." : ",\n");
      }

      os << "^h\n\nAnybody who wishes to use a daytime ability may do so now. Otherwise, enter ^cnight^h to continue.";
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

void maf::Player_kicked::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void maf::Player_kicked::write_full(std::ostream &os) const {
   os << "^G"
   << game_log().get_name(*player)
   << " kicked^g"
   << game_log().get_name(*player)
   << " was kicked from the game!\nThey were the "
   << full_name(player->role())
   << ".";
}

void maf::Player_kicked::write_summary(std::ostream &os) const {
   os << game_log().get_name(*player) << " was kicked.";
}

void maf::Lynch_result::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void maf::Lynch_result::write_full(std::ostream &os) const {
   os << "^GLynch Result^g";

   if (victim) {
      auto & victim_name = game_log().get_name(*victim);

      os << victim_name << " was lynched!\n";

      if (victim_role) {
         os << "They were a " << full_name(victim_role->id()) << ".";
         if (victim_role->is_troll()) {
            os << "^i\n\nA chill blows through the air. The townsfolk who voted to lynch "
            << victim_name
            << " look nervous...";
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

void maf::Duel_result::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
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

   os << "^GDuel^g"
   << caster_name
   << " has challenged "
   << target_name
   << " to a duel!^i\n\nThe pistols are loaded, and the participants take ten paces in opposite directions...\n\n3... 2... 1... BANG!!^g\n\nThe lifeless body of "
   << loser_name
   << " falls to the ground. "
   << winner_name
   << " lets out a sigh of relief.";

   if (!winner->is_present()) {
      os << "\n\nWith that, "
      << winner_name
      << " throws their gun to the ground and flees from the village.";
   }

   os << "^h\n\nWhen you have finished with this screen, enter ^cok^h.";
}

void maf::Duel_result::write_summary(std::ostream &os) const {
   auto & glog = game_log();
   auto & caster_name = glog.get_name(*caster);
   auto & target_name = glog.get_name(*target);

   if (caster->is_alive()) { // fix-me: unstable code, as the caster may not be alive later in the game if the rules change, yet still won the duel
      os << caster_name << " won a duel against " << target_name << ".";
   } else {
      os << caster_name << " lost a duel against " << target_name << ".";
   }
}

void maf::Choose_fake_role::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   } else if (_fake_role) {
      if (commands_match(commands, {"ok"})) {
         _go_to_sleep = true;
      } else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"choose", ""})) {
         const Role &fake_role = game_log.game().rulebook().get_role(commands[1]);

         _fake_role = &fake_role;
         game_log.choose_fake_role(_player->id(), fake_role.id());
      } else {
         throw Bad_commands{};
      }
   }
}

void maf::Choose_fake_role::write_full(std::ostream &os) const {
   if (_go_to_sleep) {
      os << "^GChoose Fake Role^g"
      << game_log().get_name(*_player)
      << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^h to continue.";
   } else if (_fake_role) {
      os << "^GChoose Fake Role^g"
      << game_log().get_name(*_player)
      << ", you have been given the "
      << full_name(*_fake_role)
      << " as your fake role.\n\nYou must pretend that this is your real role for the remainder of the game. Breaking this rule will result in you being kicked from the game!\n\nNow would be a good time to study your fake role.^h\n\nEnter ^chelp r "
      << _fake_role->alias()
      << "^h to see more information about your fake role.\nWhen you are ready, enter ^cok^h to continue.";
   } else {
      os << "^GChoose Fake Role^g"
      << game_log().get_name(*_player)
      << " needs to be given a fake role, which they must pretend is their true role for the rest of the game.^h\n\nIf they break the rules by contradicting their fake role, then they should be kicked from the game by entering ^ckick "
      << game_log().get_name(*_player)
      << "^h during the day.\n\nTo choose the role with alias ^cA^h, enter ^cchoose A^h.";
   }
}

void maf::Choose_fake_role::write_summary(std::ostream &os) const {
   if (_fake_role) {
      os << game_log().get_name(*_player) << " was given the " << full_name(*_fake_role) << " as a fake role.";
   }
}

void maf::Mafia_meeting::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   } else if (_initial) {
      if (commands_match(commands, {"ok"})) {
         _go_to_sleep = true;
      } else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"kill", "", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);

         game_log.cast_mafia_kill(caster.id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         /* fix-me: show "confirm skip?" screen. */
         game_log.skip_mafia_kill();
         _go_to_sleep = true;
      } else {
         throw Bad_commands{};
      }
   }
}

void maf::Mafia_meeting::write_full(std::ostream &os) const {
   /* fix-me */
   if (_go_to_sleep) {
      os << "^GMafia Meeting^gThe mafia have nothing more to discuss for now, and should go back to sleep.^h\n\nEnter ^cok^h when you are ready to continue.";
   } else if (_initial) {
      os << "^GMafia Meeting^gThe mafia consists of:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         auto& p_ref = *it;
         os << "   " << game_log().get_name(*p_ref) << ", the " << full_name(p_ref->role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nThere is not enough time left to organise a murder.";
   } else {
      os << "^GMafia Meeting^gSeated around a polished walnut table are:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         auto& p_ref = *it;
         os << "   " << game_log().get_name(*p_ref) << ", the " << full_name(p_ref->role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nThe mafia are ready to choose their next victim.^h\n\nEntering ^ckill A B^h will make player A attempt to kill player B. Player A must be a member of the mafia.\n\nIf the mafia have chosen not to kill anybody this night, enter ^cskip^h.";
   }
}

void maf::Kill_use::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"kill", ""})) {
         const Player &target = game_log.find_player(commands[1]);

         game_log.cast_kill(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         game_log.skip_kill(_caster->id());
         _go_to_sleep = true;
      } else {
         throw Bad_commands{};
      }
   }
}

void maf::Kill_use::write_full(std::ostream &os) const {
   if (_go_to_sleep) {
      os <<"^GKill Use^g"
      << game_log().get_name(*_caster)
      << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^h to continue.";
   } else {
      os << "^GKill Use^g"
      << game_log().get_name(*_caster)
      << ", you can choose to kill somebody this night.^h\n\nEnter ^ckill A^h to kill player A, or enter ^cskip^h if you don't wish to kill anybody.";
   }
}

void maf::Heal_use::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"heal", ""})) {
         const Player &target = game_log.find_player(commands[1]);

         game_log.cast_heal(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         game_log.skip_heal(_caster->id());
         _go_to_sleep = true;
      } else {
         throw Bad_commands{};
      }
   }
}

void maf::Heal_use::write_full(std::ostream &os) const {
   if (_go_to_sleep) {
      os <<"^GHeal Use^g"
      << game_log().get_name(*_caster)
      << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^h to continue.";
   } else {
      os << "^GHeal Use^g"
      << game_log().get_name(*_caster)
      << ", you can choose to heal somebody this night.^h\n\nEnter ^cheal A^h to heal player A, or enter ^cskip^h if you don't wish to heal anybody.";
   }
}

void maf::Investigate_use::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"check", ""})) {
         const Player &target = game_log.find_player(commands[1]);

         game_log.cast_investigate(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         game_log.skip_investigate(_caster->id());
         _go_to_sleep = true;
      } else {
         throw Bad_commands{};
      }
   }
}

void maf::Investigate_use::write_full(std::ostream &os) const {
   if (_go_to_sleep) {
      os <<"^GInvestigation^g"
      << game_log().get_name(*_caster)
      << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^h to continue.";
   } else {
      os << "^GInvestigation^g"
      << game_log().get_name(*_caster)
      << ", you can choose to investigate somebody this night.^h\n\nEnter ^ccheck A^h to investigate player A, or enter ^cskip^h if you don't wish to investigate anybody.";
   }
}

void maf::Peddle_use::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"target", ""})) {
         const Player &target = game_log.find_player(commands[1]);

         game_log.cast_peddle(_caster->id(), target.id());
         _go_to_sleep = true;
      } else if (commands_match(commands, {"skip"})) {
         game_log.skip_peddle(_caster->id());
         _go_to_sleep = true;
      } else {
         throw Bad_commands{};
      }
   }
}

void maf::Peddle_use::write_full(std::ostream &os) const {
   if (_go_to_sleep) {
      os <<"^GPeddle^g"
      << game_log().get_name(*_caster)
      << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^h to continue.";
   } else {
      os << "^GPeddle^g"
      << game_log().get_name(*_caster)
      << ", you can choose to peddle drugs to somebody this night.^h\n\nEnter ^ctarget A^h to peddle drugs to player A, or enter ^cskip^h if you don't wish to peddle drugs to anybody.";
   }
}

void maf::Boring_night::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void maf::Boring_night::write_full(std::ostream &os) const {
   /* fix-me: show current date in title. (e.g. "Night 1") */

   os << "^GCalm Night^iIt is warm outside. The moon shines brightly. The gentle chirping of crickets is carried by a pleasant breeze...^g\n\nNothing of interest happened this night, although you should still wait a few moments before continuing, to maintain the illusion that something happened.^h\n\nEnter ^cok^h to continue.";
}

void maf::Investigation_result::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_go_to_sleep) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      }
      else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"ok"})) {
         _go_to_sleep = true;
      }
      else {
         throw Bad_commands{};
      }
   }
}

void maf::Investigation_result::write_full(std::ostream &os) const {
   if (_go_to_sleep) {
      os << "^GInvestigation Result^g"
      << game_log().get_name(investigation.caster())
      << " should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^h to continue.";
   } else {
      os << "^GInvestigation Result^g"
      << game_log().get_name(investigation.caster())
      << ", you have completed your investigation of "
      << game_log().get_name(investigation.target())
      << ".\n\n";

      if (investigation.result()) {
         os << game_log().get_name(investigation.target())
         << " was behaving very suspiciously this night!";
      } else {
         os << "The investigation was fruitless. "
         << game_log().get_name(investigation.target())
         << " appears to be innocent.";
      }

      os << "^h\n\nWhen you are ready, enter ^cok^h to continue.";
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

void maf::Game_ended::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   throw Bad_commands{};
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

   os << "^GGame Over^gThe game has ended!";

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

   os << "^h\n\nTo return to the setup screen, enter ^cend^h.";
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
