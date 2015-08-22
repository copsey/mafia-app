#include <sstream>

#include "alias.hpp"
#include "event.hpp"

std::string mafia::Player_given_initial_role::summary() const {
   /* fix-me: add more useful info */

   std::string s{};
   s += _player_ptr->name();
   s += " played as the ";
   s += full_name(*_role_ptr);
   s += ".";
   return s;
}

std::string mafia::Player_given_initial_role::description() const {
   /* fix-me */

   std::ostringstream ss{};

   if (_is_private) {
      ss << "^G" << _player_ptr->name() << "'s Role^g"
         << _player_ptr->name() << ", your role is the "
         << full_name(*_role_ptr) << ".";

      if (_joker_ptr != nullptr) {
         ss << " You were randomly given this role from "
            << "[Insert Joker Here]" << ".";
      }

      ss << "^h\n\nTo see a full description of your role, enter ^chelp r "
         << alias(_role_ptr->id()) << "^h.";
   } else {
      ss << "^G" << _player_ptr->name() << "'s Role^g"
         << _player_ptr->name() << ", you are about to be shown your role.";
   }

   return ss.str();
}

bool mafia::Player_given_initial_role::advance() {
   if (_is_private) {
      return false;
   } else {
      _is_private = true;
      return true;
   }
}

std::string mafia::Time_changed::summary() const {
   std::ostringstream ss{};

   switch (_time) {
      case Time::day:
         ss << "Day " << _date << " began.";
         break;

      case Time::night:
         ss << "Night " << _date << " began.";
         break;
   }

   return ss.str();
}

std::string mafia::Time_changed::description() const {
   std::ostringstream ss{};

   switch (_time) {
      case Time::day: {
         ss << "^GDay " << _date << "^iDawn breaks, and dim sunlight beams "
            "onto the weary townsfolk...\n\n"
            "^gIt is now day " << _date
            << ". Anybody still asleep can wake up.";
      }
         break;

      case Time::night: {
         ss << "^GNight " << _date << "^iAs darkness sets in, the townsfolk "
            "return to the comforts of their shelters...\n\n^gIt is now night "
            << _date << ". Everybody still in the game should go to sleep.";
      }
         break;
   }

   return ss.str();
}

std::string mafia::Town_meeting::description() const {
   /* fix-me */

   std::ostringstream ss{};

   ss << "^GDay " << _game->date() << "^gThis is a town meeting.";

   return ss.str();
}

std::string mafia::Lynch_vote_cast::summary() const {
   std::ostringstream ss{};

   if (_target == nullptr) {
      ss << _caster->name() << " chose not to vote.";
   } else {
      ss << _caster->name() << " voted to lynch " << _target->name() << ".";
   }

   return ss.str();
}

std::string mafia::Lynch_vote_cast::description() const {
   /* fix-me */

   std::ostringstream ss{};

   ss << "^GLynch Vote^gA lynch vote was cast.";

   return ss.str();
}

std::string mafia::Player_lynched::summary() const {
   std::ostringstream ss{};
   ss << (_victim ? _victim->name() : "Nobody") << " was lynched.";
   return ss.str();
}

std::string mafia::Player_lynched::description() const {
   std::ostringstream ss{};

   ss << "^GLynch Result^g";
   if (_victim == nullptr) {
      ss << "Nobody was lynched today.";
   } else {
      ss << _victim->name() << " was lynched!\n";
      if (_victim_role == nullptr) {
         ss << "Their role could not be determined.";
      } else {
         ss << "They were a " << full_name(_victim_role->id()) << ".";
      }
   }

   return ss.str();
}

std::string mafia::Mafia_meeting::description() const {
   /* fix-me */

   std::ostringstream ss{};

   if (_initial) {
      ss << "^GMafia Meeting^gThe Mafia consists of:\n";
      for (const Player *p_ptr : _mafiosi) {
         ss << "    - " << p_ptr->name() << ", the " << full_name(p_ptr->role())
            << ",\n";
      }
   } else {
      ss << "^GMafia Meeting^gBlah blah blah Mafia meeting blah blah blah.";
   }

   return ss.str();
}

std::string mafia::Boring_night::description() const {
   return "^GNothing to Do^gNothing of interest happened this night, although "
   "you should still wait a few moments before continuing, to maintain the "
   "illusion that something happened.";
}

std::string mafia::Game_ended::description() const {
   /* fix-me */
   return "^GGame Over^gThe game has ended!\nThe following players won:...\n\nThe following players lost:...^h\n\nEnter some commands to do some stuff.";
}