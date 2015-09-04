#include "player.hpp"

mafia::Player::Player(std::string name, ID id)
 : _name{name}, _id{id} {

}

const std::string & mafia::Player::name() const {
   return _name;
}

mafia::Player::ID mafia::Player::id() const {
   return _id;
}

const mafia::Role & mafia::Player::role() const {
   return *_role;
}

void mafia::Player::assign_role(const Role &role) {
   _role = &role;
}

const mafia::Wildcard * mafia::Player::wildcard() const {
   return _wildcard;
}

void mafia::Player::set_wildcard(const Wildcard &wildcard) {
   _wildcard = &wildcard;
}

bool mafia::Player::is_alive() const {
   return _is_alive;
}

bool mafia::Player::is_dead() const {
   return !_is_alive;
}

mafia::Date mafia::Player::date_of_death() const {
   return _date_of_death;
}

mafia::Time mafia::Player::time_of_death() const {
   return _time_of_death;
}

void mafia::Player::kill(Date date, Time time) {
   _is_alive = false;
   _is_present = false;
   _date_of_death = date;
   _time_of_death = time;
}

bool mafia::Player::is_present() const {
   return _is_present;
}

void mafia::Player::leave() {
   _is_present = false;
}

void mafia::Player::refresh() {
   _compulsory_abilities.clear();

   _lynch_vote = nullptr;
   
   _is_healed = false;
   _is_high = false;
}

const std::vector<mafia::Role::Ability> & mafia::Player::compulsory_abilities() const {
   return _compulsory_abilities;
}

void mafia::Player::add_compulsory_ability(Role::Ability ability) {
   _compulsory_abilities.push_back(ability);
}

void mafia::Player::remove_compulsory_ability(Role::Ability ability) {
   for (auto it = _compulsory_abilities.begin(); it != _compulsory_abilities.end(); ++it) {
      if ((*it).id == ability.id) {
         _compulsory_abilities.erase(it);
         return;
      }
   }
   /* fix-me: throw exception if compulsory ability is not currently stored. */
}

const mafia::Player * mafia::Player::lynch_vote() const {
   return _lynch_vote;
}

void mafia::Player::cast_lynch_vote(const mafia::Player &target) {
   _lynch_vote = &target;
}

void mafia::Player::clear_lynch_vote() {
   _lynch_vote = nullptr;
}

bool mafia::Player::has_won_duel() const {
   return _has_won_duel;
}

void mafia::Player::win_duel() {
   _has_won_duel = true;
}

bool mafia::Player::is_healed() const {
   return _is_healed;
}

void mafia::Player::heal() {
   _is_healed = true;
}

bool mafia::Player::is_suspicious() const {
   return role().is_suspicious || _is_high;
}

void mafia::Player::give_drugs() {
   _is_high = true;
}

bool mafia::Player::has_won() const {
   return _has_won;
}

void mafia::Player::win() {
   _has_won = true;
}

void mafia::Player::lose() {
   _has_won = false;
}
