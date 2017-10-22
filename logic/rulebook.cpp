#include "errors.hpp"
#include "rulebook.hpp"

maf::Rulebook::Rulebook(Edition edition)
 : _edition{edition} {
   if (edition != 1) throw error::invalid_edition();

   Role & peasant = new_village_role(Role::ID::peasant);
   peasant._duel_strength = 0.333333333;

   Role & doctor = new_village_role(Role::ID::doctor);
   doctor._ability_box.put({Ability::ID::heal});
   doctor._duel_strength = 0.1;

   Role & detective = new_village_role(Role::ID::detective);
   detective._ability_box.put({Ability::ID::investigate});
   detective._duel_strength = 4;

   Role & racketeer = new_mafia_role(Role::ID::racketeer);
   racketeer._duel_strength = 9;

   Role & godfather = new_mafia_role(Role::ID::godfather);
   godfather._suspicious = false;
   godfather._duel_strength = 0.4;

   Role & dealer = new_mafia_role(Role::ID::dealer);
   dealer._ability_box.put({Ability::ID::peddle});

   Role & coward = new_freelance_role(Role::ID::coward);
   coward._suspicious = true;
   coward._duel_strength = 0.000000001;

   Role & actor = new_freelance_role(Role::ID::actor);
   actor._role_faker = true;
   actor._duel_strength = 0.333333333;

   Role & serial_killer = new_freelance_role(Role::ID::serial_killer);
   serial_killer._ability_box.put({Ability::ID::kill});
   serial_killer._peace_condition = Peace_condition::last_survivor;
   serial_killer._suspicious = true;
   serial_killer._duel_strength = 999999999;

   Role & village_idiot = new_freelance_role(Role::ID::village_idiot);
   village_idiot._win_condition = Win_condition::be_lynched;
   village_idiot._troll = true;
   village_idiot._duel_strength = 0.001;

   Role & musketeer = new_freelance_role(Role::ID::musketeer);
   musketeer._ability_box.put({Ability::ID::duel});
   musketeer._win_condition = Win_condition::win_duel;

   new_wildcard(Wildcard::ID::any, [](const Role &) {
      return 1;
   });

   new_wildcard(Wildcard::ID::village, [](const Role & r) {
      return (r.alignment() == Alignment::village) ? 1 : 0;
   });

   new_wildcard(Wildcard::ID::village_basic, {
      {Role::ID::peasant, 5},
      {Role::ID::doctor, 2},
      {Role::ID::detective, 2}
   });

   new_wildcard(Wildcard::ID::mafia, [](const Role & r) {
      return (r.alignment() == Alignment::mafia) ? 1 : 0;
   });

   new_wildcard(Wildcard::ID::freelance, [](const Role & r) {
      return (r.alignment() == Alignment::freelance) ? 1 : 0;
   });
}

std::vector<rkt::ref<const maf::Role>> maf::Rulebook::all_roles() const {
   std::vector<rkt::ref<const Role>> v{};
   for (auto & r: _roles) v.emplace_back(r);
   return v;
}

std::vector<rkt::ref<const maf::Role>> maf::Rulebook::village_roles() const {
   std::vector<rkt::ref<const Role>> v{};
   for (auto & r: _roles) {
      if (r.alignment() == Alignment::village) v.emplace_back(r);
   }
   return v;
}

std::vector<rkt::ref<const maf::Role>> maf::Rulebook::mafia_roles() const {
   std::vector<rkt::ref<const Role>> v{};
   for (auto & r: _roles) {
      if (r.alignment() == Alignment::mafia) v.emplace_back(r);
   }
   return v;
}

std::vector<rkt::ref<const maf::Role>> maf::Rulebook::freelance_roles() const {
   std::vector<rkt::ref<const Role>> v{};
   for (auto & r: _roles) {
      if (r.alignment() == Alignment::freelance) v.emplace_back(r);
   }
   return v;
}

std::vector<rkt::ref<const maf::Wildcard>> maf::Rulebook::village_wildcards() const {
   std::vector<rkt::ref<const Wildcard>> v{};
   for (auto & w: _wildcards) {
      if (w.matches_alignment(Alignment::village, *this)) v.emplace_back(w);
   }
   return v;
}

std::vector<rkt::ref<const maf::Wildcard>> maf::Rulebook::mafia_wildcards() const {
   std::vector<rkt::ref<const Wildcard>> v{};
   for (auto & w: _wildcards) {
      if (w.matches_alignment(Alignment::mafia, *this)) v.emplace_back(w);
   }
   return v;
}

std::vector<rkt::ref<const maf::Wildcard>> maf::Rulebook::freelance_wildcards() const {
   std::vector<rkt::ref<const Wildcard>> v{};
   for (auto & w: _wildcards) {
      if (w.matches_alignment(Alignment::freelance, *this)) v.emplace_back(w);
   }
   return v;
}

bool maf::Rulebook::contains_role(Role::ID id) const {
   for (auto & r: _roles) {
      if (r.id() == id) return true;
   }

   return false;
}

bool maf::Rulebook::contains_role(const std::string & alias) const {
   for (auto & r: _roles) {
      if (r.alias() == alias) return true;
   }

   return false;
}

bool maf::Rulebook::contains_wildcard(Wildcard::ID id) const {
   for (auto & w: _wildcards) {
      if (w.id() == id) return true;
   }

   return false;
}

bool maf::Rulebook::contains_wildcard(const std::string & alias) const {
   for (auto & w: _wildcards) {
      if (w.alias() == alias) return true;
   }

   return false;
}

maf::Role & maf::Rulebook::get_role(Role::ID id) {
   for (auto & r: _roles) {
      if (r.id() == id) return r;
   }

   throw error::missing_role();
}

const maf::Role & maf::Rulebook::get_role(Role::ID id) const {
   for (auto & r: _roles) {
      if (r.id() == id) return r;
   }

   throw error::missing_role();
}

maf::Role & maf::Rulebook::get_role(const std::string & alias) {
   for (auto & r: _roles) {
      if (r.alias() == alias) return r;
   }

   throw error::missing_role();
}

const maf::Role & maf::Rulebook::get_role(const std::string & alias) const {
   for (auto & r: _roles) {
      if (r.alias() == alias) return r;
   }

   throw error::missing_role();
}

maf::Wildcard & maf::Rulebook::get_wildcard(Wildcard::ID id) {
   for (auto & w: _wildcards) {
      if (w.id() == id) return w;
   }

   throw error::missing_wildcard();
}

const maf::Wildcard & maf::Rulebook::get_wildcard(Wildcard::ID id) const {
   for (auto & w: _wildcards) {
      if (w.id() == id) return w;
   }

   throw error::missing_wildcard();
}

maf::Wildcard & maf::Rulebook::get_wildcard(const std::string & alias) {
   for (auto & w: _wildcards) {
      if (w.alias() == alias) return w;
   }

   throw error::missing_wildcard();
}

const maf::Wildcard & maf::Rulebook::get_wildcard(const std::string & alias) const {
   for (auto & w: _wildcards) {
      if (w.alias() == alias) return w;
   }

   throw error::missing_wildcard();
}

maf::Role & maf::Rulebook::new_role(Role::ID id) {
   if (contains_role(id)) {
      throw error::duplicate_role();
   }

   _roles.emplace_back(id);
   return _roles.back();
}

maf::Role & maf::Rulebook::new_village_role(Role::ID id) {
   Role & role = new_role(id);
   role._alignment = Alignment::village;
   role._peace_condition = Peace_condition::mafia_eliminated;
   return role;
}

maf::Role & maf::Rulebook::new_mafia_role(Role::ID id) {
   Role & role = new_role(id);
   role._alignment = Alignment::mafia;
   role._peace_condition = Peace_condition::village_eliminated;
   role._suspicious = true;
   role._duel_strength = 4;
   return role;
}

maf::Role & maf::Rulebook::new_freelance_role(Role::ID id) {
   return new_role(id);
}

maf::Wildcard & maf::Rulebook::new_wildcard(Wildcard::ID id, Wildcard::Role_evaluator evaluator) {
   if (contains_wildcard(id)) {
      throw error::duplicate_wildcard();
   }

   _wildcards.emplace_back(id, evaluator);
   return _wildcards.back();
}

maf::Wildcard & maf::Rulebook::new_wildcard(Wildcard::ID id, const std::map<Role::ID, double> & weights) {
   if (contains_wildcard(id)) {
      throw error::duplicate_wildcard();
   }

   _wildcards.emplace_back(id, weights);
   return _wildcards.back();
}
