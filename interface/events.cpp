#include "console.hpp"
#include "events.hpp"
#include "names.hpp"

void mafia::Event::write_summary(std::ostream &os) const {
   // By default, write nothing.
}

void mafia::Event::write_help(std::ostream &os) const {
   os << "^HMissing Help Screen^hNo help has been written for the current game event.\n(this counts as a bug!)\n\nEnter ^cok^h to leave this screen.";
}

void mafia::Player_given_initial_role::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
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

void mafia::Player_given_initial_role::write_full(std::ostream &os) const {
   if (_is_private) {
      os << "^G" << _p->name() << "'s Role^g"
      << _p->name() << ", your role is the "
      << full_name(*_r) << ".";

      if (_w != nullptr) {
         /* fix-me */
         os << "\nYou were randomly given this role from a wildcard in the "
         << "[Insert Category Here]" << " category.";
      }

      os << "^h\n\nTo see a full description of your role, enter ^chelp r "
      << _r->alias()
      << "^h.";
   } else {
      os << "^G" << _p->name() << "'s Role^g"
      << _p->name() << ", you are about to be shown your role.";
   }
}

void mafia::Player_given_initial_role::write_summary(std::ostream &os) const {
   os << _p->name() << " played as the " << full_name(*_r) << ".";
}

void mafia::Time_changed::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void mafia::Time_changed::write_full(std::ostream &os) const {
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

void mafia::Time_changed::write_summary(std::ostream &os) const {
   switch (time) {
      case Time::day:
         os << "Day " << date << " began.";
         break;
      case Time::night:
         os << "Night " << date << " began.";
         break;
   }
}

void mafia::Obituary::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
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

void mafia::Obituary::write_full(std::ostream &os) const {
   if (_deaths_index < 0) {
      if (_deaths.size() == 0) {
         os << "^GObituary^gNobody died during the night.";
      } else {
         /* fix-me: reword to remove use of "us". */
         os << "^GObituary^gIt appears that " << _deaths.size() << " of us did not survive the night...";
      }
   } else {
      os << "^GObituary^g" << _deaths[_deaths_index].get().name() << " died during the night!";
   }
}

void mafia::Obituary::write_summary(std::ostream &os) const {
   bool write_nl = false;

   for (const Player &p: _deaths) {
      if (write_nl) os << '\n';
      os << p.name() << " died during the night.";
   }
}

void mafia::Town_meeting::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_lynch_can_occur) {
      if (commands_match(commands, {"vote", "", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);
         game_log.cast_lynch_vote(caster, target);
         game_log.advance();
      }
      else if (commands_match(commands, {"abstain", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         game_log.clear_lynch_vote(caster);
         game_log.advance();
      }
      else if (commands_match(commands, {"lynch"})) {
         game_log.process_lynch_votes();
         game_log.advance();
      }
      else if (commands_match(commands, {"duel", "", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);
         game_log.stage_duel(caster, target);
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
      else if (commands_match(commands, {"duel", "", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);
         game_log.stage_duel(caster, target);
      }
      else {
         throw Bad_commands{};
      }
   }
}

void mafia::Town_meeting::write_full(std::ostream &os) const {
   /* fix-me: add in proper content. */

   if (_lynch_can_occur) {
      os << "^GDay "
      << _date
      << "^i...\n\n^gGathered outside the town hall are:\n";

      for (auto it = _players.begin(); it != _players.end(); ) {
         const Player &p = *it;
         os << "   "
         << p.name();
         if (p.lynch_vote() != nullptr) {
            os << ", voting to lynch "
            << p.lynch_vote()->name();
         }
         os << ((++it == _players.end()) ? "." : ",\n");
      }

      os << "\n\nAs it stands, blah will be lynched.^h\n\nEnter ^clynch^h to submit the current lynch votes. Daytime abilities may also be used at this point.";
   } else {
      os << "^GDay "
      << _date
      << "^iWith little time left in the day, the townsfolk prepare themselves for another night of uncertainty...\n\n^gGathered outside the town hall are:\n";

      for (auto it = _players.begin(); it != _players.end(); ) {
         const Player &p = *it;
         os << "   "
         << p.name()
         << ((++it == _players.end()) ? "." : ",\n");
      }

      os << "^h\n\nAnybody who wishes to use a daytime ability may do so now. Otherwise, enter ^cnight^h to continue.";
   }
}

void mafia::Town_meeting::write_summary(std::ostream &os) const {
   if (_recent_vote_caster) {
      if (_recent_vote_target) {
         os << _recent_vote_caster->name() << " voted to lynch " << _recent_vote_target->name() << ".";
      } else {
         os << _recent_vote_caster->name() << " chose not to vote.";
      }
   }
}

void mafia::Lynch_result::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void mafia::Lynch_result::write_full(std::ostream &os) const {
   os << "^GLynch Result^g";

   if (victim) {
      os << victim->name() << " was lynched!\n";

      if (victim_role) {
         os << "They were a " << full_name(victim_role->id()) << ".";
      } else {
         os << "Their role could not be determined.";
      }
   } else {
      os << "Nobody was lynched today.";
   }
}

void mafia::Lynch_result::write_summary(std::ostream &os) const {
   os << (victim ? victim->name() : "Nobody") << " was lynched.";
}

void mafia::Duel_result::do_commands(const std::vector<std::string> &commands, mafia::Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void mafia::Duel_result::write_full(std::ostream &os) const {
   /* fix-me */
}

void mafia::Duel_result::write_summary(std::ostream &os) const {
   if (caster.get().is_alive()) {
      os << caster.get().name() << " won a duel against " << target.get().name() << ".";
   } else {
      os << caster.get().name() << " lost a duel against " << target.get().name() << ".";
   }
}

void mafia::Mafia_meeting::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (_initial) {
      if (commands_match(commands, {"ok"})) {
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   } else {
      if (commands_match(commands, {"kill", "", ""})) {
         const Player &caster = game_log.find_player(commands[1]);
         const Player &target = game_log.find_player(commands[2]);
         game_log.cast_mafia_kill(caster, target);
         game_log.advance();
      } else if (commands_match(commands, {"skip"})) {
         /* fix-me: show "confirm skip?" screen. */
         game_log.skip_mafia_kill();
         game_log.advance();
      } else {
         throw Bad_commands{};
      }
   }
}

void mafia::Mafia_meeting::write_full(std::ostream &os) const {
   /* fix-me */
   if (_initial) {
      os << "^GNight ";
      os << _date;
      os << "^gThe mafia consists of:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         const Player &p = *it;
         os << "   " << p.name() << ", the " << full_name(p.role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nStarting tomorrow, you can choose a target to kill each night.";
   } else {
      os << "^GNight ";
      os << _date;
      os << "^iIt is close to midnight. The members of th^gSeated around a large walnut table are:\n";

      for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
         const Player &p = *it;
         os << "   " << p.name() << ", the " << full_name(p.role());
         os << ((++it == _mafiosi.end()) ? "." : ",\n");
      }

      os << "\n\nThe mafia are ready to choose their next victim.^h\n\nEntering ^ckill abc xyz^h will make player abc attempt to kill player xyz.";
   }
}

void mafia::Boring_night::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   if (commands_match(commands, {"ok"})) {
      game_log.advance();
   }
   else {
      throw Bad_commands{};
   }
}

void mafia::Boring_night::write_full(std::ostream &os) const {
   os << "^GNight "
      << date
      << "^iIt is warm outside. The moon shines brightly. The gentle chirping of crickets is carried by a pleasant breeze...^g\n\nNothing of interest happened this night, although you should still wait a few moments before continuing, to maintain the illusion that something happened.";
}

void mafia::Game_ended::do_commands(const std::vector<std::string> &commands, Game_log &game_log) {
   throw Bad_commands{};
}

void mafia::Game_ended::write_full(std::ostream &os) const {
   /* fix-me */
   os << "^GGame Over^gThe game has ended!\nThe following players won:...\n\nThe following players lost:...^h\n\nTo return to the setup screen, enter ^cend^h.";
}