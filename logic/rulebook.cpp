#include <sstream>
#include <stdexcept>

#include "../riketi/algorithm.hpp"
#include "../riketi/enum.hpp"

#include "rulebook.hpp"

mafia::Rulebook::Rulebook(int edition)
   : _edition(edition) {
   if (edition != 1) {
      std::ostringstream err_msg{};
      err_msg << "No rulebook of edition " << edition << " is defined.";

      throw std::invalid_argument{err_msg.str()};
   }

   new_village_role(Role::ID::peasant);

   new_mafia_role(Role::ID::racketeer);

   Role &coward = new_freelance_role(Role::ID::coward);
   coward._is_suspicious = true;

   new_joker(Joker::ID::any,
             [](const Role &role) {
                return 1.0;
             });

   new_joker(Joker::ID::village,
             [](const Role &role) {
                if (role.alignment() == Role::Alignment::village) {
                   return 1.0;
                } else {
                   return 0.0;
                }
             });

   new_joker(Joker::ID::village_basic,
             {{Role::ID::peasant, 1.0}});

   new_joker(Joker::ID::mafia,
             [](const Role &role) {
                if (role.alignment() == Role::Alignment::mafia) {
                   return 1.0;
                } else {
                   return 0.0;
                }
             });

   new_joker(Joker::ID::freelance,
             [](const Role &role) {
                if (role.alignment() == Role::Alignment::freelance) {
                   return 1.0;
                } else {
                   return 0.0;
                }
             });
}

std::vector<mafia::Role>::iterator
mafia::Rulebook::find_role(Role::ID id) {
   return rkt::find_if(_roles,
                       [id](const Role &role) {
                          return role.id() == id;
                       });
}

std::vector<mafia::Role>::const_iterator
mafia::Rulebook::find_role(Role::ID id) const {
   return rkt::find_if(_roles,
                       [id](const Role &role) {
                          return role.id() == id;
                       });
}

mafia::Role & mafia::Rulebook::get_role(Role::ID id) {
   auto p = find_role(id);

   if (p == _roles.end()) {
      std::ostringstream err_msg{};
      err_msg << "No role with ID " << rkt::value(id)
              << " could be found in the rulebook.";

      throw std::out_of_range{err_msg.str()};
   }

   return *p;
}

mafia::Role & mafia::Rulebook::new_role(Role::ID id) {
   if (contains_role(id)) {
      std::ostringstream err_msg{};
      err_msg << "A role with ID " << rkt::value(id)
              << " already exists in the rulebook.";

      throw std::invalid_argument{err_msg.str()};
   }

   _roles.emplace_back(id);
   return _roles.back();
}

mafia::Role & mafia::Rulebook::new_village_role(Role::ID id) {
   Role &role = new_role(id);
   role._alignment = Role::Alignment::village;
   role._peace_condition = Role::Peace_condition::mafia_eliminated;
   return role;
}

mafia::Role & mafia::Rulebook::new_mafia_role(Role::ID id) {
   Role &role = new_role(id);
   role._alignment = Role::Alignment::mafia;
   role._is_suspicious = true;
   role._peace_condition = Role::Peace_condition::village_eliminated;
   return role;
}

mafia::Role & mafia::Rulebook::new_freelance_role(Role::ID id) {
   return new_role(id);
}

std::vector<mafia::Joker>::iterator
mafia::Rulebook::find_joker(Joker::ID id) {
   return rkt::find_if(_jokers,
                       [id](const Joker &joker) {
                          return joker.id() == id;
                       });
}

std::vector<mafia::Joker>::const_iterator
mafia::Rulebook::find_joker(Joker::ID id) const {
   return rkt::find_if(_jokers,
                       [id](const Joker &joker) {
                          return joker.id() == id;
                       });
}


bool mafia::Rulebook::contains_joker(Joker::ID id) const {
   return find_joker(id) != _jokers.end();
}

mafia::Joker & mafia::Rulebook::get_joker(Joker::ID id) {
   auto p = find_joker(id);
   if (p == _jokers.end()) {
      std::ostringstream err_msg{};
      err_msg << "No joker with ID " << rkt::value(id)
              << " could be found in the rulebook.";

      throw std::out_of_range{err_msg.str()};
   }
   return *p;
}

mafia::Joker & mafia::Rulebook::new_joker(Joker::ID id, Joker::Role_evaluator evaluator) {
   if (contains_joker(id)) {
      std::ostringstream err_msg{};
      err_msg << "A joker with ID " << rkt::value(id)
              << " already exists in the rulebook.";

      throw std::invalid_argument{err_msg.str()};
   }

   _jokers.emplace_back(id, evaluator);
   return _jokers.back();
}

mafia::Joker & mafia::Rulebook::new_joker(Joker::ID id, std::map<Role::ID, double> weights) {
   if (contains_joker(id)) {
      std::ostringstream err_msg{};
      err_msg << "A joker with ID " << rkt::value(id)
              << " already exists in the rulebook.";

      throw std::invalid_argument{err_msg.str()};
   }

   _jokers.emplace_back(id, weights);
   return _jokers.back();
}
