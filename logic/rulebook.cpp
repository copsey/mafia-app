#include <sstream>

#include "../riketi/algorithm.hpp"
#include "../riketi/enum.hpp"

#include "rulebook.hpp"

mafia::Rulebook::Rulebook(): Rulebook{latest_edition} { }

mafia::Rulebook::Rulebook(int edition)
: _edition{edition} {
   if (edition != 1) {
      std::ostringstream err{};
      err << "No rulebook of edition " << edition << " is defined.";

      throw std::invalid_argument(err.str());
   }

   Role &peasant = new_village_role(Role::ID::peasant);
   peasant.duel_strength = 0.333333333;

   Role &racketeer = new_mafia_role(Role::ID::racketeer);
   racketeer.duel_strength = 9;

   Role &coward = new_freelance_role(Role::ID::coward);
   coward.is_suspicious = true;
   coward.duel_strength = 0.000000001;

   Role &serial_killer = new_freelance_role(Role::ID::serial_killer);
   serial_killer.peace_condition = Role::Peace_condition::last_survivor;
   serial_killer.is_suspicious = true;
   serial_killer.duel_strength = 999999999;

   Role &musketeer = new_freelance_role(Role::ID::musketeer);
   musketeer.ability.put({Role::Ability::ID::duel});
   musketeer.win_condition = Role::Win_condition::win_duel;

   new_wildcard(Wildcard::ID::any, [](const Role &) {
      return 1.0;
   });

   new_wildcard(Wildcard::ID::village, [](const Role &r) {
      if (r.alignment == Role::Alignment::village) {
         return 1.0;
      } else {
         return 0.0;
      }
   });

   new_wildcard(Wildcard::ID::village_basic, {
      {Role::ID::peasant, 1.0}
   });

   new_wildcard(Wildcard::ID::mafia, [](const Role &r) {
      if (r.alignment == Role::Alignment::mafia) {
         return 1.0;
      } else {
         return 0.0;
      }
   });

   new_wildcard(Wildcard::ID::freelance, [](const Role &r) {
      if (r.alignment == Role::Alignment::freelance) {
         return 1.0;
      } else {
         return 0.0;
      }
   });
}

const std::vector<mafia::Role> & mafia::Rulebook::roles() const {
   return _roles;
}

std::vector<rkt::ref<const mafia::Role>> mafia::Rulebook::village_roles() const {
   std::vector<rkt::ref<const Role>> v{};
   for (const Role &r: _roles) {
      if (r.alignment == Role::Alignment::village) v.emplace_back(r);
   }
   return v;
}

std::vector<rkt::ref<const mafia::Role>> mafia::Rulebook::mafia_roles() const {
   std::vector<rkt::ref<const Role>> v{};
   for (const Role &r: _roles) {
      if (r.alignment == Role::Alignment::mafia) v.emplace_back(r);
   }
   return v;
}

std::vector<rkt::ref<const mafia::Role>> mafia::Rulebook::freelance_roles() const {
   std::vector<rkt::ref<const Role>> v{};
   for (const Role &r: _roles) {
      if (r.alignment == Role::Alignment::freelance) v.emplace_back(r);
   }
   return v;
}

std::vector<rkt::ref<const mafia::Wildcard>> mafia::Rulebook::village_wildcards() const {
   std::vector<rkt::ref<const Wildcard>> v{};
   for (const Wildcard &w: _wildcards) {
      if (w.matches_alignment(Role::Alignment::village, *this)) v.emplace_back(w);
   }
   return v;
}

std::vector<rkt::ref<const mafia::Wildcard>> mafia::Rulebook::mafia_wildcards() const {
   std::vector<rkt::ref<const Wildcard>> v{};
   for (const Wildcard &w: _wildcards) {
      if (w.matches_alignment(Role::Alignment::mafia, *this)) v.emplace_back(w);
   }
   return v;
}

std::vector<rkt::ref<const mafia::Wildcard>> mafia::Rulebook::freelance_wildcards() const {
   std::vector<rkt::ref<const Wildcard>> v{};
   for (const Wildcard &w: _wildcards) {
      if (w.matches_alignment(Role::Alignment::freelance, *this)) v.emplace_back(w);
   }
   return v;
}

const std::vector<mafia::Wildcard> & mafia::Rulebook::wildcards() const {
   return _wildcards;
}

bool mafia::Rulebook::contains_role(Role::ID id) const {
   for (const Role &r: _roles) {
      if (r.id() == id) return true;
   }
   return false;
}

bool mafia::Rulebook::contains_role(const std::string &alias) const {
   for (const Role &r: _roles) {
      if (r.alias() == alias) return true;
   }
   return false;
}

bool mafia::Rulebook::contains_wildcard(Wildcard::ID id) const {
   for (const Wildcard &w: _wildcards) {
      if (w.id() == id) return true;
   }
   return false;
}

bool mafia::Rulebook::contains_wildcard(const std::string &alias) const {
   for (const Wildcard &w: _wildcards) {
      if (w.alias() == alias) return true;
   }
   return false;
}

mafia::Role & mafia::Rulebook::get_role(Role::ID id) {
   for (Role &r: _roles) {
      if (r.id() == id) return r;
   }
   throw Missing_role_ID{id};
}

const mafia::Role & mafia::Rulebook::get_role(Role::ID id) const {
   for (const Role &r: _roles) {
      if (r.id() == id) return r;
   }
   throw Missing_role_ID{id};
}

mafia::Role & mafia::Rulebook::get_role(const std::string &alias) {
   for (Role &r: _roles) {
      if (r.alias() == alias) return r;
   }
   throw Missing_role_alias{alias};
}

const mafia::Role & mafia::Rulebook::get_role(const std::string &alias) const {
   for (const Role &r: _roles) {
      if (r.alias() == alias) return r;
   }
   throw Missing_role_alias{alias};
}

mafia::Wildcard & mafia::Rulebook::get_wildcard(Wildcard::ID id) {
   for (Wildcard &w: _wildcards) {
      if (w.id() == id) return w;
   }
   throw Missing_wildcard_ID{id};
}

const mafia::Wildcard & mafia::Rulebook::get_wildcard(Wildcard::ID id) const {
   for (const Wildcard &w: _wildcards) {
      if (w.id() == id) return w;
   }
   throw Missing_wildcard_ID{id};
}

mafia::Wildcard & mafia::Rulebook::get_wildcard(const std::string &alias) {
   for (Wildcard &w: _wildcards) {
      if (w.alias() == alias) return w;
   }
   throw Missing_wildcard_alias{alias};
}

const mafia::Wildcard & mafia::Rulebook::get_wildcard(const std::string &alias) const {
   for (const Wildcard &w: _wildcards) {
      if (w.alias() == alias) return w;
   }
   throw Missing_wildcard_alias{alias};
}

mafia::Role & mafia::Rulebook::new_role(Role::ID id) {
   if (contains_role(id)) {
      std::ostringstream err{};
      err << "A role with ID "
          << rkt::value(id)
          << " (alias "
          << alias(id)
          << ") already exists in the rulebook.";

      throw std::invalid_argument{err.str()};
   }

   _roles.emplace_back(id);
   return _roles.back();
}

mafia::Role & mafia::Rulebook::new_village_role(Role::ID id) {
   Role &role = new_role(id);
   role.alignment = Role::Alignment::village;
   role.peace_condition = Role::Peace_condition::mafia_eliminated;
   return role;
}

mafia::Role & mafia::Rulebook::new_mafia_role(Role::ID id) {
   Role &role = new_role(id);
   role.alignment = Role::Alignment::mafia;
   role.is_suspicious = true;
   role.peace_condition = Role::Peace_condition::village_eliminated;
   return role;
}

mafia::Role & mafia::Rulebook::new_freelance_role(Role::ID id) {
   return new_role(id);
}

mafia::Wildcard & mafia::Rulebook::new_wildcard(Wildcard::ID id, Wildcard::Role_evaluator evaluator) {
   if (contains_wildcard(id)) {
      std::ostringstream err{};
      err << "A wildcard with ID "
      << rkt::value(id)
      << " (alias "
      << alias(id)
      << ") already exists in the rulebook.";

      throw std::invalid_argument{err.str()};
   }

   _wildcards.emplace_back(id, evaluator);
   return _wildcards.back();
}

mafia::Wildcard & mafia::Rulebook::new_wildcard(Wildcard::ID id, const std::map<Role::ID, double> &weights) {
   if (contains_wildcard(id)) {
      std::ostringstream err{};
      err << "A wildcard with ID "
      << rkt::value(id)
      << " (alias "
      << alias(id)
      << ") already exists in the rulebook.";

      throw std::invalid_argument{err.str()};
   }

   _wildcards.emplace_back(id, weights);
   return _wildcards.back();
}