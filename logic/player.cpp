#include "player.hpp"

void maf::Player::assign_role(const Role & role) {
   _role_ptr = &role;
   _fake_role_ptr = nullptr;
}

void maf::Player::set_wildcard(const Wildcard & wildcard) {
   _wildcard_ptr = &wildcard;
}

void maf::Player::kill(Date date, Time time) {
   _alive = false;
   _present = false;
   _date_of_death = date;
   _time_of_death = time;
}

void maf::Player::leave() {
   _present = false;
}

void maf::Player::kick() {
   leave();
   _kicked = true;
}

bool maf::Player::has_been_lynched() const {
   return _lynched;
}

void maf::Player::lynch(Date date) {
   kill(date, Time::day);
   _lynched = true;
}

void maf::Player::give_fake_role(const maf::Role & role) {
   _fake_role_ptr = &role;
}

void maf::Player::refresh() {
   _compulsory_abilities.clear();

   _lynch_vote = nullptr;
   
   _healed = false;
   _on_drugs = false;
}

const std::vector<maf::Ability> & maf::Player::compulsory_abilities() const {
   return _compulsory_abilities;
}

void maf::Player::add_compulsory_ability(Ability ability) {
   _compulsory_abilities.push_back(ability);
}

void maf::Player::remove_compulsory_ability(Ability ability) {
   for (auto it = _compulsory_abilities.begin(); it != _compulsory_abilities.end(); ++it) {
      if ((*it).id == ability.id) {
         _compulsory_abilities.erase(it);
         return;
      }
   }
   /* FIXME: throw exception if compulsory ability is not currently stored. */
}

void maf::Player::cast_lynch_vote(const Player & target) {
   _lynch_vote = &target;
}

bool maf::Player::is_suspicious() const {
   return role().is_suspicious() || _on_drugs;
}

void maf::Player::give_drugs() {
   _on_drugs = true;
}

void maf::Player::haunt(const Player &haunter) {
   _haunter = &haunter;
}

bool maf::operator==(const Player & p1, const Player & p2) {
   return p1.id() == p2.id();
}
